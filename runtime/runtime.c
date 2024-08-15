#include "../include/dandelion/runtime.h"
#include "../include/dandelion/system/system.h"

#include "runtime.h"

#define sysdata __dandelion_system_data

RuntimeData __runtime_global_data;

// internal state of the runtime
// ---------------------------------
// current lower end of sbrk
static size_t alloc_base;
// root index for alloc
static size_t *free_root;
// end of allocation for alloc
static size_t last_descriptor;
// ---------------------------------

void dandelion_init(void) {
  __dandelion_system_init();

  // initialize all static variables needed in the runtime
  // mainly do this to make it easier to test
  free_root = NULL;
  alloc_base = 0;
  last_descriptor = 0;

  // parse raw input data into tree structure
  rtdata.input_sets =
      dandelion_alloc(sysdata.input_sets_len * sizeof(IoSet), _Alignof(IoSet));
  if (rtdata.input_sets == NULL && sysdata.input_sets_len != 0) {
    sysdata.exit_code = DANDELION_OOM;
    __dandelion_system_exit();
    return;
  }
  for (size_t i = 0; i < sysdata.input_sets_len; ++i) {
    IoSet *set = &rtdata.input_sets[i];
    size_t num_bufs =
        sysdata.input_sets[i + 1].offset - sysdata.input_sets[i].offset;
    set->ident = sysdata.input_sets[i].ident;
    set->ident_len = sysdata.input_sets[i].ident_len;
    set->buffers =
        dandelion_alloc(num_bufs * sizeof(IoBuffer), _Alignof(IoBuffer));
    if (set->buffers == NULL && num_bufs != 0) {
      sysdata.exit_code = DANDELION_OOM;
      __dandelion_system_exit();
      return;
    }
    set->buffers_len = num_bufs;
    set->buffers_cap = num_bufs;
    for (size_t j = 0; j < num_bufs; ++j) {
      size_t buf_offset = sysdata.input_sets[i].offset + j;
      set->buffers[j].ident = sysdata.input_bufs[buf_offset].ident;
      set->buffers[j].ident_len = sysdata.input_bufs[buf_offset].ident_len;
      set->buffers[j].data = sysdata.input_bufs[buf_offset].data;
      set->buffers[j].data_len = sysdata.input_bufs[buf_offset].data_len;
    }
  }

  rtdata.output_sets =
      dandelion_alloc(sysdata.output_sets_len * sizeof(IoSet), _Alignof(IoSet));
  if (rtdata.output_sets == NULL && sysdata.output_sets_len != 0) {
    sysdata.exit_code = DANDELION_OOM;
    __dandelion_system_exit();
    return;
  }
  for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
    rtdata.output_sets[i].ident = sysdata.output_sets[i].ident;
    rtdata.output_sets[i].ident_len = sysdata.output_sets[i].ident_len;
    rtdata.output_sets[i].buffers = NULL;
    rtdata.output_sets[i].buffers_len = 0;
    rtdata.output_sets[i].buffers_cap = 0;
  }
}

void dandelion_exit(int exit_code) {
  sysdata.exit_code = exit_code;
  // convert tree structure into raw output data
  size_t num_output_bufs = 0;
  for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
    num_output_bufs += rtdata.output_sets[i].buffers_len;
  }

  sysdata.output_bufs =
      dandelion_alloc(num_output_bufs * sizeof(IoBuffer), _Alignof(IoBuffer));
  if (sysdata.output_bufs == NULL && num_output_bufs != 0) {
    sysdata.exit_code = DANDELION_OOM;
    __dandelion_system_exit();
    return;
  }

  size_t current_offset = 0;
  for (size_t i = 0; i < sysdata.output_sets_len; ++i) {
    sysdata.output_sets[i].offset = current_offset;

    IoSet *tree_set = &rtdata.output_sets[i];
    current_offset += tree_set->buffers_len;
    for (size_t j = 0; j < tree_set->buffers_len; ++j) {
      IoBuffer *tree_buf = &tree_set->buffers[j];
      IoBuffer *raw_buf =
          &sysdata.output_bufs[sysdata.output_sets[i].offset + j];
      raw_buf->ident = tree_buf->ident;
      raw_buf->ident_len = tree_buf->ident_len;
      raw_buf->data = tree_buf->data;
      raw_buf->data_len = tree_buf->data_len;
    }
  }
  // sentinel set output
  sysdata.output_sets[sysdata.output_sets_len].offset = current_offset;

  __dandelion_system_exit();
}

void dandelion_set_thread_pointer(void *ptr) {
  return __dandelion_system_set_thread_pointer(ptr);
}

/// @brief move current heap end up by size
/// @param size amount of bytes to move up heap end
/// @return pointer to start of new allocation or NULL if there is not enough
/// memory left
/// allocations are always rounded to allign on size of void*
void *dandelion_sbrk(size_t size) {
  if (alloc_base == 0) {
    alloc_base = sysdata.heap_begin;
    // allign to allignment of void*
    size_t alloc_rounding = alloc_base % sizeof(void *);
    alloc_base += (alloc_rounding == 0) ? 0 : (sizeof(void *) - alloc_rounding);
  }
  if (alloc_base + size > sysdata.heap_end) {
    return NULL;
  }
  void *result = (void *)alloc_base;
  // rount size up to multiple of size_t
  size_t size_rounding = size % sizeof(void *);
  alloc_base += size;
  alloc_base += (size_rounding == 0) ? 0 : (sizeof(void *) - size_rounding);
  return result;
}

static const size_t DEFAULT_ALLOCATION =
    4096; // assumed to be multiple of sizeof(size_t)
static const size_t OCCUPIED_FLAG = ((size_t)-1) << ((sizeof(size_t) * 8 - 1));
static const size_t MAX_VAL = (size_t)-1;

static inline int is_occupied(size_t allocation) {
  return (OCCUPIED_FLAG & allocation) != 0;
}

// need to have space for at least the amount of additional memory that was
// asked for, some buffer to alighn, and 4 descriptors, one to possibly end a
// skip, one for the start, one for the end and one for a possible skip.
// Rounding up to next multiple of DEFAULT_ALLOCATION
static inline size_t get_sbrk_size(size_t size, size_t alignment) {
  return ((size * sizeof(size_t) + 4 * sizeof(size_t) + alignment +
           DEFAULT_ALLOCATION - 1) /
          DEFAULT_ALLOCATION) *
         DEFAULT_ALLOCATION;
}

/// @brief memmory allocation for internal usage
/// @param size size of the allocation
/// @param alignment byte allignment requirement for the allocation, will be
/// rounded to next mutiple of size_of(size_t)
/// @return returns pointer to the allocation to be used or NULL if there is
/// free root is a pointer to a descriptor number pointing to the first slab of
/// allocated memory.
/// Each slab contains two descriptors, one at the start one at the end.
/// Each descriptor is an index into the memory array relative to free_root.
/// The entierty of memory can be represented as an array of these numbers
/// The one at the start showing the occupation and the index of the end
/// descriptor. The one at the end giving the index of the start one as well as
/// occupation. The next slab can be found by indexing 1 past the start/end
/// descriptor. MAX_VALUE is used as a place holder past the last allocation, to
/// ensure we have space for a descriptor to describe how far it is to any new
/// allocation. If the new allocation is not contiguous with the current one,
/// this descriptor can be used to mark the space between the allocations as
/// occupied.
///
/// Example:
/// Normal allocation
/// | Free(2) |_| Free(0) | Occupied(8) |_|_|_|_| Occupied(3) | MAX_VAL |
/// Skiping other allocation
/// | ... | MAX_VAL | ->
/// | ... | Occupied(..) | Other allocation | Occupied(..) | ... | MAX_VAL |
/// Need to account for one descriptor for closing the previous MAX_VAL
/// descriptor
void *dandelion_alloc(size_t size, size_t alignment) {
  // reject any size allocations that are large enough to interfere with our
  // occupied markings root is not null, so find if any of the free memory can
  // be used
  if (is_occupied(size)) {
    return NULL;
  }

  if (size == 0) {
    return NULL;
  }

  // convert to multiples of sizeof(size_t)
  size_t local_size = (size + sizeof(size_t) - 1) / sizeof(size_t);
  // convert alignment to multiples of sizeof(size_t), and at least 1
  size_t local_alignment = (alignment + sizeof(size_t) - 1) / sizeof(size_t);
  local_alignment = local_alignment == 0 ? 1 : local_alignment;

  if (free_root == NULL) {
    // add 2 size_t to  ask size for minimal bookkeeping we need and alignment
    // so we guarantee to get a usable allocation. Round size to multiple of
    // default allocation.
    size_t allocation_size = get_sbrk_size(local_size, local_alignment);
    free_root = dandelion_sbrk(allocation_size);
    if (free_root == NULL) {
      return NULL;
    }
    size_t indices_allocated = allocation_size / sizeof(size_t);
    free_root[0] = indices_allocated - 2;
    free_root[indices_allocated - 2] = 0;
    free_root[indices_allocated - 1] = MAX_VAL;
    last_descriptor = indices_allocated - 1;
  }

  // go through list and find the smallest allocation that can fit the
  // request
  size_t smallest = MAX_VAL;
  size_t smallest_size = MAX_VAL;
  size_t current = 0;
  while (free_root[current] != MAX_VAL) {
    size_t end_index = free_root[current] & ~OCCUPIED_FLAG;
    // check if the current allocation is occupied
    if (is_occupied(free_root[current])) {
      current = end_index + 1;
      continue;
    }
    // is not occupied, check if allocation could fit
    // find how many indices until alignment
    size_t index_mod = (current + 1) % local_alignment;
    size_t rounding_indices =
        index_mod == 0 ? 0 : local_alignment - (index_mod);
    // need to subtract one as current is occupied
    size_t potential_size = end_index - (current + rounding_indices) - 1;
    if (local_size <= potential_size && potential_size < smallest_size) {
      smallest = current;
      smallest_size = potential_size;
    }
    current = end_index + 1;
  }

  // if we have not found allocation that fits ask for more space
  if (smallest_size == MAX_VAL) {
    size_t allocation_size = get_sbrk_size(local_size, local_alignment);
    size_t *new_allocation = dandelion_sbrk(allocation_size);
    if (new_allocation == NULL) {
      return NULL;
    }
    // check if contiuous with previous
    size_t new_size = allocation_size / sizeof(size_t);
    if (new_allocation == free_root + last_descriptor + 1) {
      size_t new_last = last_descriptor + new_size;
      // mark reserved space at end
      free_root[new_last] = MAX_VAL;
      // check if last space bevore spaceholeder was occupied
      if (!is_occupied(free_root[last_descriptor - 1])) {
        smallest = free_root[last_descriptor - 1];
        free_root[smallest] = new_last - 1;
        free_root[new_last - 1] = smallest;
      } else {
        smallest = last_descriptor;
        free_root[last_descriptor] = new_last - 1;
        free_root[new_last - 1] = last_descriptor;
      }
      last_descriptor = new_last;
    } else {
      // find the index of the start of this allocation
      size_t skip_end_index = new_allocation - free_root;
      free_root[last_descriptor] = skip_end_index | OCCUPIED_FLAG;
      free_root[skip_end_index] = last_descriptor | OCCUPIED_FLAG;
      last_descriptor = skip_end_index + new_size - 1;
      free_root[skip_end_index + 1] = last_descriptor - 1;
      free_root[last_descriptor - 1] = skip_end_index + 1;
      free_root[last_descriptor] = MAX_VAL;
      smallest = skip_end_index + 1;
    }
  }
  // smallest has the index ready to take the allocation

  // check for alignment
  size_t start = smallest;
  size_t end = free_root[start];
  size_t skip_indices = (start + 1) % local_alignment;
  skip_indices = skip_indices == 0 ? 0 : local_alignment - skip_indices;
  // if less than 3 indices are skipped not worth to make a new allocation,
  // try to extend previous one,
  size_t actual_start = start + skip_indices;
  if (skip_indices != 0) {
    if (skip_indices < 3) {
      if (start != 0) {
        // know that previous was occupied, otherwise would have been fused with
        // start
        size_t previous_start = free_root[start - 1] & ~OCCUPIED_FLAG;
        free_root[previous_start] = (actual_start - 1) | OCCUPIED_FLAG;
        free_root[actual_start - 1] = previous_start | OCCUPIED_FLAG;
      } else {
        // mark as occupied, this is either one or two indices. If it is one we
        // write the same thing to it twice, if it is two we have an empty free
        // allocation
        free_root[0] = (actual_start - 1);
        free_root[actual_start - 1] = 0;
      }
    } else {
      free_root[start] = actual_start - 1;
      free_root[actual_start - 1] = start;
    }
  }

  size_t actual_end = actual_start + local_size + 1;
  // check distance between actual end and end
  if (end < actual_end) {
    // should never happend, as the allocation should be big enough at this
    // point
    sysdata.exit_code = skip_indices;
    __dandelion_system_exit();
    return NULL;
  } else if (end > actual_end + 2) {
    // make a new allocation after the end of this
    // can never merge with next one, since next is occupied, would have been
    // merge on free otherwise
    free_root[actual_end + 1] = end;
    free_root[end] = actual_end + 1;
  } else {
    // only one or two indices to spare before next allocation, so just extend
    // this allocation
    actual_end = end;
  }
  free_root[actual_start] = actual_end | OCCUPIED_FLAG;
  free_root[actual_end] = actual_start | OCCUPIED_FLAG;
  return &free_root[actual_start + 1];
}

void dandelion_free(void *free_ptr) {
  // before the allocation there should be a descriptor inidacting it is
  // occupied
  size_t free_index = ((size_t *)free_ptr) - free_root;
  size_t free_end = free_root[free_index - 1];
#ifdef DEBUG
  if (!is_occupied(free_end)) {
    sysdata.exit_code = DANDELION_ALLOC_FREE_UNOCCUPIED;
    __dandelion_system_exit();
    return;
  }
#endif
  free_end = free_end & ~OCCUPIED_FLAG;
  size_t free_start = free_root[free_end];
#ifdef DEBUG
  if (!is_occupied(free_start)) {
    sysdata.exit_code = DANDELION_ALLOC_FREE_UNOCCUPIED;
    __dandelion_system_exit();
    return;
  }
#endif
  free_start = free_start & ~OCCUPIED_FLAG;
#ifdef DEBUG
  if (free_start != free_index - 1) {
    sysdata.exit_code = DANDELION_ALLOC_OVERLAP;
    __dandelion_system_exit();
    return;
  }
#endif
  // check if there is something to merge with in front
  if (free_start != 0 && !is_occupied(free_root[free_start - 1])) {
    free_start = free_root[free_start - 1];
  }
  // check if there is something after to merge with
  if (!is_occupied(free_root[free_end + 1])) {
    free_end = free_root[free_end + 1];
  }
  free_root[free_start] = free_end;
  free_root[free_end] = free_start;
  return;
}

size_t dandelion_input_set_count(void) { return sysdata.input_sets_len; }

size_t dandelion_output_set_count(void) { return sysdata.output_sets_len; }

size_t dandelion_input_buffer_count(size_t set_idx) {
  if (set_idx >= sysdata.input_sets_len) {
    return 0;
  }
  return rtdata.input_sets[set_idx].buffers_len;
}

IoBuffer *dandelion_get_input(size_t set_idx, size_t buf_idx) {
  if (set_idx >= sysdata.input_sets_len) {
    return NULL;
  }
  if (buf_idx >= rtdata.input_sets[set_idx].buffers_len) {
    return NULL;
  }
  return &rtdata.input_sets[set_idx].buffers[buf_idx];
}

void dandelion_add_output(size_t set_idx, IoBuffer buf) {
  if (set_idx >= sysdata.output_sets_len) {
    return;
  }
  IoSet *set = &rtdata.output_sets[set_idx];
  if (set->buffers_len == set->buffers_cap) {
    size_t new_cap = set->buffers_cap * 2;
    if (new_cap == 0) {
      new_cap = 1;
    }
    IoBuffer *new_bufs =
        dandelion_alloc(new_cap * sizeof(IoBuffer), _Alignof(IoBuffer));
    for (size_t i = 0; i < set->buffers_len; ++i) {
      new_bufs[i] = set->buffers[i];
    }
    set->buffers = new_bufs;
    set->buffers_cap = new_cap;
  }
  set->buffers[set->buffers_len++] = buf;
}

size_t dandelion_output_buffer_count(size_t set_idx) {
  if (set_idx >= sysdata.output_sets_len) {
    return 0;
  }
  return rtdata.output_sets[set_idx].buffers_len;
}

const char *dandelion_input_set_ident(size_t set_idx) {
  if (set_idx >= sysdata.input_sets_len) {
    return NULL;
  }
  return rtdata.input_sets[set_idx].ident;
}

size_t dandelion_input_set_ident_len(size_t set_idx) {
  if (set_idx >= sysdata.input_sets_len) {
    return 0;
  }
  return rtdata.input_sets[set_idx].ident_len;
}

const char *dandelion_output_set_ident(size_t set_idx) {
  if (set_idx >= sysdata.output_sets_len) {
    return NULL;
  }
  return rtdata.output_sets[set_idx].ident;
}

size_t dandelion_output_set_ident_len(size_t set_idx) {
  if (set_idx >= sysdata.output_sets_len) {
    return 0;
  }
  return rtdata.output_sets[set_idx].ident_len;
}