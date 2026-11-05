#ifndef _DANDELION_META_FUNCTION_H
#define _DANDELION_META_FUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*
    Helpers for writing the planner of a metafunction.

    A metafunction is applied in a composition as
        plan (info = x) (a, b, c) => (d);
    Its planner gets the planner params (`info`) as the first input sets, as
    for any other function, and in addition:
    - the input set "input_metadata", describing the data inputs (`a, b, c`)
    - the input set "output_metadata", naming the outputs (`d`)
    It returns the source of a composition in the output set "composition".
    That composition is run on the data inputs and produces the outputs, both
    matched by position: its first input set gets `a`, its first output set
    becomes `d` and so on.
*/

#define DANDELION_META_INPUT_METADATA_SET "input_metadata"
#define DANDELION_META_OUTPUT_METADATA_SET "output_metadata"
#define DANDELION_META_COMPOSITION_SET "composition"

// Description of a data input of the metafunction application
typedef struct DandelionMetaInput {
  // Name of the set in the composition applying the metafunction
  // (not necessarily null-terminated)
  const char *ident;
  size_t ident_len;
  // Number of items in the set
  size_t num_items;
  // Summed size of all items in the set in bytes
  size_t total_size;
} DandelionMetaInput;

// Description of an output of the metafunction application
typedef struct DandelionMetaOutput {
  // Name of the set in the composition applying the metafunction
  // (not necessarily null-terminated)
  const char *ident;
  size_t ident_len;
} DandelionMetaOutput;

// Number of data inputs of the application, 0 if the function is no planner
size_t dandelion_meta_input_count(void);
// Fills in the description of the data input at position idx.
// Returns 0 on success, -1 if there is no such input or it is malformed.
int dandelion_meta_get_input(size_t idx, DandelionMetaInput *input);

// Number of outputs of the application, 0 if the function is no planner
size_t dandelion_meta_output_count(void);
// Fills in the description of the output at position idx.
// Returns 0 on success, -1 if there is no such output.
int dandelion_meta_get_output(size_t idx, DandelionMetaOutput *output);

// Sets the composition to run in place of the metafunction, given as source
// with exactly as many input and output sets as the application has. The
// source is copied, so it does not need to outlive the call. Can only be set
// once. Returns 0 on success, -1 if the function is no planner, the plan was
// already set or there is not enough memory.
int dandelion_meta_set_plan(const char *composition, size_t composition_len);

#ifdef __cplusplus
}
#endif

#endif
