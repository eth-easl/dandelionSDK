#ifndef _DANDELION_COMPOSITION_H
#define _DANDELION_COMPOSITION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*
    Builder for compositions, so that a planner can construct its plan as a
    graph instead of writing composition DSL by hand.

    A composition consists of
    - input sets and output sets, referred to by position,
    - declarations of the functions it calls,
    - calls of these functions, each binding sets to the function's
      parameters and producing one set per used return set.

    Example, planning `concat(first = all in0, second = all in1) => (out0)`:

        DandelionComposition *comp = dandelion_comp_new_plan("Plan");
        const char *params[] = {"first", "second"};
        const char *rets[] = {"joined"};
        DandelionFunction concat =
            dandelion_comp_function(comp, "concat", params, 2, rets, 1);
        DandelionCall call = dandelion_comp_call(comp, concat);
        dandelion_comp_arg(comp, call, "first", DANDELION_SHARDING_ALL,
                           dandelion_comp_input(comp, 0));
        dandelion_comp_arg(comp, call, "second", DANDELION_SHARDING_ALL,
                           dandelion_comp_input(comp, 1));
        dandelion_comp_output(comp, 0, dandelion_comp_ret(comp, call,
                                                          "joined"));
        int err = dandelion_comp_set_plan(comp);
        dandelion_comp_free(comp);

    Errors are sticky: the first failing call is recorded (see
    dandelion_comp_error) and every following call becomes a no-op that
    returns invalid handles. So a planner can build the whole composition
    and check for errors once at the end.

    Names (composition, functions, parameters and return sets) are
    null-terminated and copied, so they do not need to outlive the call.

    For now the composition is rendered to composition DSL, and the builder
    leaves most validation to the platform's parser, which rejects the plan
    with a diagnostic: names need to be identifiers, functions need to be
    registered with the declared parameters and return sets, calls can only
    use declared names, join orders need to be valid and the composition
    needs to be acyclic. The builder itself only rejects what it can not
    render or what the parser would silently accept.
*/

typedef struct DandelionComposition DandelionComposition;

// Handles into a composition, only valid for the composition that created
// them. A failed call returns a handle with id DANDELION_COMP_INVALID.
#define DANDELION_COMP_INVALID ((size_t)-1)
typedef struct DandelionFunction {
  size_t id;
} DandelionFunction;
typedef struct DandelionCall {
  size_t id;
} DandelionCall;
typedef struct DandelionSet {
  size_t id;
} DandelionSet;

// How a call is distributed over the items of an argument set
typedef enum DandelionSharding {
  // one instance gets all items
  DANDELION_SHARDING_ALL,
  // one instance per key, getting all items with that key
  DANDELION_SHARDING_KEYED,
  // one instance per item
  DANDELION_SHARDING_EACH,
  // like keyed / each, but the platform may group them as it sees fit
  DANDELION_SHARDING_ANY_KEYED,
  DANDELION_SHARDING_ANY_EACH,
} DandelionSharding;

// How keyed arguments are combined, see dandelion_comp_join_then
typedef enum DandelionJoin {
  DANDELION_JOIN_CROSS,
  DANDELION_JOIN_INNER,
  DANDELION_JOIN_LEFT,
  DANDELION_JOIN_RIGHT,
  DANDELION_JOIN_FULL,
} DandelionJoin;

// Creates an empty composition with the given name and number of input and
// output sets. Returns NULL if there is not enough memory.
DandelionComposition *dandelion_comp_new(const char *name, size_t num_inputs,
                                         size_t num_outputs);
// Like dandelion_comp_new, but with as many inputs and outputs as the
// metafunction application this planner is run for, as required by
// dandelion_comp_set_plan.
DandelionComposition *dandelion_comp_new_plan(const char *name);
// Frees the composition and everything it owns, including rendered source.
void dandelion_comp_free(DandelionComposition *comp);

// Returns a description of the first error, NULL if there was none.
const char *dandelion_comp_error(const DandelionComposition *comp);

// The input set at position idx.
DandelionSet dandelion_comp_input(DandelionComposition *comp, size_t idx);

// Declares a function with the given parameter and return set names. It
// needs to be registered on the platform under the same name.
DandelionFunction dandelion_comp_function(DandelionComposition *comp,
                                          const char *name,
                                          const char *const *params,
                                          size_t num_params,
                                          const char *const *rets,
                                          size_t num_rets);

// Adds a call of the function. Arguments and return sets are added to it
// with the functions below.
DandelionCall dandelion_comp_call(DandelionComposition *comp,
                                  DandelionFunction function);

// Binds set to the parameter of the call. Each parameter can be bound once.
void dandelion_comp_arg(DandelionComposition *comp, DandelionCall call,
                        const char *param, DandelionSharding sharding,
                        DandelionSet set);
// Like dandelion_comp_arg, but the call also runs if the set is empty.
void dandelion_comp_optional_arg(DandelionComposition *comp, DandelionCall call,
                                 const char *param, DandelionSharding sharding,
                                 DandelionSet set);

// The set produced by the return set of the call. Asking twice for the same
// return set gives the same set.
DandelionSet dandelion_comp_ret(DandelionComposition *comp, DandelionCall call,
                                const char *ret);

// Sets the order in which the keyed arguments of a call are joined, as in
// `by first strategy1 second strategy2 third`. Start with
// dandelion_comp_join and add each following parameter with the strategy
// joining it to the previous ones with dandelion_comp_join_then. Keyed
// arguments that are not listed are cross joined.
void dandelion_comp_join(DandelionComposition *comp, DandelionCall call,
                         const char *first_param);
void dandelion_comp_join_then(DandelionComposition *comp, DandelionCall call,
                              DandelionJoin strategy, const char *param);

// Makes set the output at position idx. The set has to be produced by a
// call, and a set can be only one output.
void dandelion_comp_output(DandelionComposition *comp, size_t idx,
                           DandelionSet set);

// Renders the composition as composition DSL. The source is owned by comp
// and valid until the next modification or until it is freed. Returns NULL
// on error, for example if an output is not set.
const char *dandelion_comp_render(DandelionComposition *comp, size_t *len);

// Renders the composition and sets it as plan of the metafunction with
// dandelion_meta_set_plan. Returns 0 on success, -1 otherwise.
int dandelion_comp_set_plan(DandelionComposition *comp);

#ifdef __cplusplus
}
#endif

#endif
