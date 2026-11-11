#include "../include/dandelion/composition.h"

#include "../include/dandelion/meta_function.h"
#include "../include/dandelion/runtime.h"

#define INVALID DANDELION_COMP_INVALID

// the builtins need no declaration and are lowered to inline code or calls to
// memcpy etc., which the system library (FREESTANDING) or libc provides
#define memcpy __builtin_memcpy
#define memset __builtin_memset
#define memcmp __builtin_memcmp

/*
    The builder only records the graph and renders it. Everything the
    composition parser checks anyway (names being identifiers and declared,
    join orders, cycles, ...) is left to the platform, which reports it when
    parsing the plan. The builder only checks what it needs to render and what
    the parser would silently accept.
*/

// ---------------------------------------------------------------------------
// growable arrays, elements may move when they grow

typedef struct Vec {
  void *data;
  size_t len;
  size_t cap;
} Vec;

// makes room for extra more elements, returns 0 on success
static int vec_reserve(Vec *vec, size_t elem_size, size_t extra) {
  if (extra <= vec->cap - vec->len) {
    return 0;
  }
  size_t new_cap = vec->cap == 0 ? 8 : vec->cap;
  while (extra > new_cap - vec->len) {
    new_cap *= 2;
  }
  void *new_data = dandelion_alloc(new_cap * elem_size, sizeof(size_t));
  if (new_data == NULL) {
    return -1;
  }
  if (vec->data != NULL) {
    memcpy(new_data, vec->data, vec->len * elem_size);
    dandelion_free(vec->data);
  }
  vec->data = new_data;
  vec->cap = new_cap;
  return 0;
}

static void vec_free(Vec *vec) {
  if (vec->data != NULL) {
    dandelion_free(vec->data);
  }
}

#define VEC_AT(vec, type, idx) (((type *)(vec).data)[idx])

static size_t c_str_len(const char *str) {
  size_t len = 0;
  while (str[len] != '\0') {
    len++;
  }
  return len;
}

// ---------------------------------------------------------------------------
// composition graph

// a string in the composition's string pool
typedef struct Name {
  size_t offset;
  size_t len;
} Name;

typedef struct Function {
  Name name;
  // the parameter names followed by the return set names in comp->names
  size_t first_name;
  size_t num_params;
  size_t num_rets;
} Function;

typedef struct Call {
  size_t function;
} Call;

// a set is either an input of the composition or a return set of a call
typedef struct Set {
  // INVALID for inputs
  size_t call;
  // input position for inputs, return set name otherwise
  size_t input;
  Name ret;
} Set;

typedef struct Arg {
  size_t call;
  Name param;
  DandelionSharding sharding;
  int optional;
  size_t set;
} Arg;

typedef struct JoinEntry {
  size_t call;
  Name param;
  // strategy joining the parameter to the previous ones, unused for the first
  DandelionJoin strategy;
} JoinEntry;

struct DandelionComposition {
  Name name;
  size_t num_inputs;
  Vec pool;      // char, all strings
  Vec names;     // Name, parameter and return set names of functions
  Vec functions; // Function
  Vec calls;     // Call
  Vec sets;      // Set, the first num_inputs are the inputs
  Vec args;      // Arg
  Vec joins;     // JoinEntry
  Vec outputs;   // size_t, set id per output, INVALID while not set
  Vec source;    // char, rendered source, emptied on modification
  const char *error;
};

static void fail(DandelionComposition *comp, const char *error) {
  if (comp->error == NULL) {
    comp->error = error;
  }
}

// whether the composition can be modified, invalidates rendered source
static int modifiable(DandelionComposition *comp) {
  if (comp == NULL || comp->error != NULL) {
    return 0;
  }
  comp->source.len = 0;
  return 1;
}

// copies the string into the pool, the name is empty if out of memory
static Name intern(DandelionComposition *comp, const char *str) {
  Name name = {comp->pool.len, 0};
  size_t len = str == NULL ? 0 : c_str_len(str);
  if (vec_reserve(&comp->pool, 1, len) != 0) {
    fail(comp, "out of memory");
    return name;
  }
  memcpy((char *)comp->pool.data + comp->pool.len, str, len);
  comp->pool.len += len;
  name.len = len;
  return name;
}

static int name_equals(DandelionComposition *comp, Name name, const char *str) {
  size_t len = str == NULL ? 0 : c_str_len(str);
  return name.len == len &&
         memcmp((char *)comp->pool.data + name.offset, str, len) == 0;
}

// appends an element and returns its index, INVALID if out of memory
static size_t push(DandelionComposition *comp, Vec *vec, size_t elem_size) {
  if (vec_reserve(vec, elem_size, 1) != 0) {
    fail(comp, "out of memory");
    return INVALID;
  }
  return vec->len++;
}

DandelionComposition *dandelion_comp_new(const char *name, size_t num_inputs,
                                         size_t num_outputs) {
  DandelionComposition *comp =
      dandelion_alloc(sizeof(DandelionComposition), sizeof(size_t));
  if (comp == NULL) {
    return NULL;
  }
  memset(comp, 0, sizeof(DandelionComposition));
  comp->num_inputs = num_inputs;
  comp->name = intern(comp, name);
  if (vec_reserve(&comp->sets, sizeof(Set), num_inputs) != 0 ||
      vec_reserve(&comp->outputs, sizeof(size_t), num_outputs) != 0 ||
      comp->error != NULL) {
    dandelion_comp_free(comp);
    return NULL;
  }
  for (size_t i = 0; i < num_inputs; i++) {
    VEC_AT(comp->sets, Set, i) = (Set){INVALID, i, {0, 0}};
  }
  comp->sets.len = num_inputs;
  for (size_t i = 0; i < num_outputs; i++) {
    VEC_AT(comp->outputs, size_t, i) = INVALID;
  }
  comp->outputs.len = num_outputs;
  return comp;
}

DandelionComposition *dandelion_comp_new_plan(const char *name) {
  return dandelion_comp_new(name, dandelion_meta_input_count(),
                            dandelion_meta_output_count());
}

void dandelion_comp_free(DandelionComposition *comp) {
  if (comp == NULL) {
    return;
  }
  vec_free(&comp->pool);
  vec_free(&comp->names);
  vec_free(&comp->functions);
  vec_free(&comp->calls);
  vec_free(&comp->sets);
  vec_free(&comp->args);
  vec_free(&comp->joins);
  vec_free(&comp->outputs);
  vec_free(&comp->source);
  dandelion_free(comp);
}

const char *dandelion_comp_error(const DandelionComposition *comp) {
  if (comp == NULL) {
    return "no composition";
  }
  return comp->error;
}

DandelionSet dandelion_comp_input(DandelionComposition *comp, size_t idx) {
  DandelionSet set = {INVALID};
  if (!modifiable(comp)) {
    return set;
  }
  if (idx >= comp->num_inputs) {
    fail(comp, "input index out of range");
    return set;
  }
  // inputs are the first sets
  set.id = idx;
  return set;
}

DandelionFunction dandelion_comp_function(DandelionComposition *comp,
                                          const char *name,
                                          const char *const *params,
                                          size_t num_params,
                                          const char *const *rets,
                                          size_t num_rets) {
  DandelionFunction handle = {INVALID};
  if (!modifiable(comp)) {
    return handle;
  }
  size_t first_name = comp->names.len;
  if (vec_reserve(&comp->names, sizeof(Name), num_params + num_rets) != 0) {
    fail(comp, "out of memory");
    return handle;
  }
  for (size_t i = 0; i < num_params + num_rets; i++) {
    VEC_AT(comp->names, Name, comp->names.len++) =
        intern(comp, i < num_params ? params[i] : rets[i - num_params]);
  }
  Name function_name = intern(comp, name);
  size_t idx = push(comp, &comp->functions, sizeof(Function));
  if (comp->error != NULL) {
    return handle;
  }
  VEC_AT(comp->functions, Function, idx) =
      (Function){function_name, first_name, num_params, num_rets};
  handle.id = idx;
  return handle;
}

DandelionCall dandelion_comp_call(DandelionComposition *comp,
                                  DandelionFunction function) {
  DandelionCall call = {INVALID};
  if (!modifiable(comp)) {
    return call;
  }
  if (function.id >= comp->functions.len) {
    fail(comp, "invalid function handle");
    return call;
  }
  size_t idx = push(comp, &comp->calls, sizeof(Call));
  if (idx == INVALID) {
    return call;
  }
  VEC_AT(comp->calls, Call, idx).function = function.id;
  call.id = idx;
  return call;
}

static void add_arg(DandelionComposition *comp, DandelionCall call,
                    const char *param, DandelionSharding sharding,
                    DandelionSet set, int optional) {
  if (!modifiable(comp)) {
    return;
  }
  if (call.id >= comp->calls.len || set.id >= comp->sets.len) {
    fail(comp, "invalid call or set handle");
    return;
  }
  // the parser lets a later binding silently replace an earlier one
  for (size_t i = 0; i < comp->args.len; i++) {
    Arg *arg = &VEC_AT(comp->args, Arg, i);
    if (arg->call == call.id && name_equals(comp, arg->param, param)) {
      fail(comp, "parameter is bound twice");
      return;
    }
  }
  Name param_name = intern(comp, param);
  size_t idx = push(comp, &comp->args, sizeof(Arg));
  if (comp->error != NULL) {
    return;
  }
  VEC_AT(comp->args, Arg, idx) =
      (Arg){call.id, param_name, sharding, optional, set.id};
}

void dandelion_comp_arg(DandelionComposition *comp, DandelionCall call,
                        const char *param, DandelionSharding sharding,
                        DandelionSet set) {
  add_arg(comp, call, param, sharding, set, 0);
}

void dandelion_comp_optional_arg(DandelionComposition *comp, DandelionCall call,
                                 const char *param, DandelionSharding sharding,
                                 DandelionSet set) {
  add_arg(comp, call, param, sharding, set, 1);
}

DandelionSet dandelion_comp_ret(DandelionComposition *comp, DandelionCall call,
                                const char *ret) {
  DandelionSet handle = {INVALID};
  if (!modifiable(comp)) {
    return handle;
  }
  if (call.id >= comp->calls.len) {
    fail(comp, "invalid call handle");
    return handle;
  }
  // a return set can only be bound to one set
  for (size_t i = comp->num_inputs; i < comp->sets.len; i++) {
    Set *set = &VEC_AT(comp->sets, Set, i);
    if (set->call == call.id && name_equals(comp, set->ret, ret)) {
      handle.id = i;
      return handle;
    }
  }
  Name ret_name = intern(comp, ret);
  size_t idx = push(comp, &comp->sets, sizeof(Set));
  if (comp->error != NULL) {
    return handle;
  }
  VEC_AT(comp->sets, Set, idx) = (Set){call.id, 0, ret_name};
  handle.id = idx;
  return handle;
}

static void add_join(DandelionComposition *comp, DandelionCall call,
                     DandelionJoin strategy, const char *param) {
  if (!modifiable(comp)) {
    return;
  }
  if (call.id >= comp->calls.len) {
    fail(comp, "invalid call handle");
    return;
  }
  Name param_name = intern(comp, param);
  size_t idx = push(comp, &comp->joins, sizeof(JoinEntry));
  if (comp->error != NULL) {
    return;
  }
  VEC_AT(comp->joins, JoinEntry, idx) =
      (JoinEntry){call.id, param_name, strategy};
}

void dandelion_comp_join(DandelionComposition *comp, DandelionCall call,
                         const char *first_param) {
  add_join(comp, call, DANDELION_JOIN_CROSS, first_param);
}

void dandelion_comp_join_then(DandelionComposition *comp, DandelionCall call,
                              DandelionJoin strategy, const char *param) {
  add_join(comp, call, strategy, param);
}

void dandelion_comp_output(DandelionComposition *comp, size_t idx,
                           DandelionSet set) {
  if (!modifiable(comp)) {
    return;
  }
  if (idx >= comp->outputs.len || set.id >= comp->sets.len) {
    fail(comp, "output index or set handle out of range");
    return;
  }
  // the parser accepts these, but drops the output position of an input and
  // keeps only the last position of a set returned twice
  if (VEC_AT(comp->sets, Set, set.id).call == INVALID) {
    fail(comp, "output has to be produced by a call");
    return;
  }
  for (size_t i = 0; i < comp->outputs.len; i++) {
    if (VEC_AT(comp->outputs, size_t, i) == set.id) {
      fail(comp, "set is already an output");
      return;
    }
  }
  if (VEC_AT(comp->outputs, size_t, idx) != INVALID) {
    fail(comp, "output is set twice");
    return;
  }
  VEC_AT(comp->outputs, size_t, idx) = set.id;
}

// ---------------------------------------------------------------------------
// rendering to composition DSL, writes into comp->source

static void write_bytes(DandelionComposition *comp, const char *bytes,
                        size_t len) {
  if (comp->error != NULL) {
    return;
  }
  if (vec_reserve(&comp->source, 1, len) != 0) {
    fail(comp, "out of memory");
    return;
  }
  memcpy((char *)comp->source.data + comp->source.len, bytes, len);
  comp->source.len += len;
}

static void write_c_str(DandelionComposition *comp, const char *str) {
  write_bytes(comp, str, c_str_len(str));
}

static void write_name(DandelionComposition *comp, Name name) {
  // the pool does not move while rendering
  write_bytes(comp, (char *)comp->pool.data + name.offset, name.len);
}

static void write_usize(DandelionComposition *comp, size_t value) {
  char digits[20];
  size_t len = 0;
  do {
    digits[sizeof(digits) - 1 - len++] = (char)('0' + value % 10);
    value /= 10;
  } while (value > 0);
  write_bytes(comp, digits + sizeof(digits) - len, len);
}

// inputs are in<i>, outputs out<i> and all other sets s<id>
static void write_set(DandelionComposition *comp, size_t id) {
  Set *set = &VEC_AT(comp->sets, Set, id);
  if (set->call == INVALID) {
    write_c_str(comp, "in");
    write_usize(comp, set->input);
    return;
  }
  for (size_t i = 0; i < comp->outputs.len; i++) {
    if (VEC_AT(comp->outputs, size_t, i) == id) {
      write_c_str(comp, "out");
      write_usize(comp, i);
      return;
    }
  }
  write_c_str(comp, "s");
  write_usize(comp, id);
}

static const char *sharding_keyword(DandelionSharding sharding) {
  switch (sharding) {
  case DANDELION_SHARDING_KEYED:
    return "keyed";
  case DANDELION_SHARDING_EACH:
    return "each";
  case DANDELION_SHARDING_ANY_KEYED:
    return "anyKeyed";
  case DANDELION_SHARDING_ANY_EACH:
    return "anyEach";
  default:
    return "all";
  }
}

static const char *join_keyword(DandelionJoin join) {
  switch (join) {
  case DANDELION_JOIN_INNER:
    return "inner";
  case DANDELION_JOIN_LEFT:
    return "left";
  case DANDELION_JOIN_RIGHT:
    return "right";
  case DANDELION_JOIN_FULL:
    return "full";
  default:
    return "cross";
  }
}

static void write_name_list(DandelionComposition *comp, size_t first,
                            size_t len) {
  write_c_str(comp, "(");
  for (size_t i = 0; i < len; i++) {
    if (i > 0) {
      write_c_str(comp, ", ");
    }
    write_name(comp, VEC_AT(comp->names, Name, first + i));
  }
  write_c_str(comp, ")");
}

static void write_call(DandelionComposition *comp, size_t call) {
  Function *function = &VEC_AT(comp->functions, Function,
                               VEC_AT(comp->calls, Call, call).function);
  write_c_str(comp, "    ");
  write_name(comp, function->name);
  write_c_str(comp, "(");
  const char *separator = "";
  for (size_t i = 0; i < comp->args.len; i++) {
    Arg *arg = &VEC_AT(comp->args, Arg, i);
    if (arg->call != call) {
      continue;
    }
    write_c_str(comp, separator);
    separator = ", ";
    write_name(comp, arg->param);
    write_c_str(comp, arg->optional ? " = optional " : " = ");
    write_c_str(comp, sharding_keyword(arg->sharding));
    write_c_str(comp, " ");
    write_set(comp, arg->set);
  }
  write_c_str(comp, ") => (");
  separator = "";
  for (size_t i = comp->num_inputs; i < comp->sets.len; i++) {
    Set *set = &VEC_AT(comp->sets, Set, i);
    if (set->call != call) {
      continue;
    }
    write_c_str(comp, separator);
    separator = ", ";
    write_set(comp, i);
    write_c_str(comp, " = ");
    write_name(comp, set->ret);
  }
  write_c_str(comp, ")");
  // `by first strategy second ...`, the strategy of the first is unused
  int first = 1;
  for (size_t i = 0; i < comp->joins.len; i++) {
    JoinEntry *join = &VEC_AT(comp->joins, JoinEntry, i);
    if (join->call != call) {
      continue;
    }
    if (first) {
      write_c_str(comp, " by ");
    } else {
      write_c_str(comp, " ");
      write_c_str(comp, join_keyword(join->strategy));
      write_c_str(comp, " ");
    }
    first = 0;
    write_name(comp, join->param);
  }
  write_c_str(comp, ";\n");
}

const char *dandelion_comp_render(DandelionComposition *comp, size_t *len) {
  if (comp == NULL || comp->error != NULL) {
    return NULL;
  }
  if (comp->source.len == 0) {
    for (size_t i = 0; i < comp->outputs.len; i++) {
      if (VEC_AT(comp->outputs, size_t, i) == INVALID) {
        fail(comp, "output is not set");
        return NULL;
      }
    }
    for (size_t i = 0; i < comp->functions.len; i++) {
      Function *function = &VEC_AT(comp->functions, Function, i);
      write_c_str(comp, "function ");
      write_name(comp, function->name);
      write_name_list(comp, function->first_name, function->num_params);
      write_c_str(comp, " => ");
      write_name_list(comp, function->first_name + function->num_params,
                      function->num_rets);
      write_c_str(comp, ";\n");
    }
    write_c_str(comp, "composition ");
    write_name(comp, comp->name);
    write_c_str(comp, "(");
    for (size_t i = 0; i < comp->num_inputs; i++) {
      write_c_str(comp, i > 0 ? ", " : "");
      write_set(comp, i);
    }
    write_c_str(comp, ") => (");
    for (size_t i = 0; i < comp->outputs.len; i++) {
      write_c_str(comp, i > 0 ? ", " : "");
      write_set(comp, VEC_AT(comp->outputs, size_t, i));
    }
    write_c_str(comp, ") {\n");
    for (size_t i = 0; i < comp->calls.len; i++) {
      write_call(comp, i);
    }
    write_c_str(comp, "}\n");
    if (comp->error != NULL) {
      return NULL;
    }
  }
  if (len != NULL) {
    *len = comp->source.len;
  }
  return comp->source.data;
}

int dandelion_comp_set_plan(DandelionComposition *comp) {
  if (comp == NULL) {
    return -1;
  }
  if (comp->num_inputs != dandelion_meta_input_count() ||
      comp->outputs.len != dandelion_meta_output_count()) {
    fail(comp, "number of inputs or outputs does not match the application");
    return -1;
  }
  size_t len = 0;
  const char *source = dandelion_comp_render(comp, &len);
  if (source == NULL) {
    return -1;
  }
  if (dandelion_meta_set_plan(source, len) != 0) {
    fail(comp, "could not set the plan");
    return -1;
  }
  return 0;
}
