use core::slice;
use std::ops::Rem;

use libc::{c_int, c_void, size_t};

use crate::dandelion_structures::{dandelion_exit_check, initialize_dandelion};

extern "C" {
    /// initialize dandelion
    pub fn dandelion_init();
    /// write output set buffers and exit
    pub fn dandelion_exit(exit_code: c_int);
    /// direct access to dandelion sbrk
    fn dandelion_sbrk(size: size_t) -> *mut c_void;
    /// dandelion internal allocator
    fn dandelion_alloc(size: size_t, alignment: size_t) -> *mut c_void;
    /// dandelion internal free
    fn dandelion_free(free_ptr: *mut c_void);
}

#[test]
fn test_sbrk_empty() {
    let _unused = initialize_dandelion(0, Vec::new(), Vec::new());
    let sbrk_result = unsafe { dandelion_sbrk(1024) };
    assert!(
        sbrk_result.is_null(),
        "sbrk with 0 heap size should always fail"
    );
    return;
}

#[test]
fn test_sbrk() {
    let chuncks = 8;
    let chunck_size = 1024;
    let total_size = chuncks * chunck_size;
    let setup = initialize_dandelion(total_size, Vec::new(), Vec::new());
    for index in 0..chuncks {
        let sbrk_result = unsafe { dandelion_sbrk(chunck_size) as *const u8 };
        dandelion_exit_check!(setup, "Expecting no error after sbrk for chunck {}", index);
        assert!(!(sbrk_result.is_null()), "sbrk failed for chunck {}", index);
        // expect chunk to be valid and within range of the original allocation
        assert!(
            setup.ptr_in_heap(&sbrk_result),
            "sbrk return pointer start not in range for chunck {}",
            index
        );
        let allocation_end = unsafe { sbrk_result.add(chunck_size - 1) };
        assert!(
            setup.ptr_in_heap(&allocation_end),
            "sbrk return end not in heap for chunk {} with allocation start at {} and end at {} heap offset",
            index,
            unsafe { sbrk_result.offset_from(setup.heap_range.start) },
            unsafe { allocation_end.offset_from(setup.heap_range.start) },
        );
    }
    // chould fail now, as the heap is full
    let sbrk_failed = unsafe { dandelion_sbrk(1) as *const u8 };
    dandelion_exit_check!(setup, "Expecting no error after sbrk");
    assert!(
        sbrk_failed.is_null(),
        "Allocation past capacity should fail"
    );
    return;
}

#[test]
fn test_alloc() {
    let heap_size = 8192;
    let setup = initialize_dandelion(heap_size, Vec::new(), Vec::new());
    // should be able to get at least 1 allocation with half of heap size
    for allocation_power in 1..heap_size.ilog2() {
        let mut allocations = Vec::new();
        let mut current_allocated = 0;
        let allocation_size = 2usize.pow(allocation_power);
        loop {
            let allocation = unsafe { dandelion_alloc(allocation_size, 1) };
            if allocation.is_null() {
                break;
            }
            let allocation_slice =
                unsafe { slice::from_raw_parts_mut(allocation as *mut u8, allocation_size) };
            for index in 0..allocation_size {
                allocation_slice[index] = 1 + u8::try_from(index.rem(255)).unwrap();
            }
            allocations.push(allocation);
            current_allocated = current_allocated + allocation_size;
        }
        assert_ne!(
            0, current_allocated,
            "Could not allocate any memory for allocation size {}",
            allocation_size
        );
        for allocation_ptr in allocations {
            unsafe { dandelion_free(allocation_ptr) };
            dandelion_exit_check!(setup, "Freeing memory failed");
        }
    }

    // test with pattern from bug
    {
        let mut allocations = Vec::new();
        let mut current_allocated = 0;
        // allocations are alligned on 8 bytes, force start and end bubbles with sizes and alignments
        // allocating with 16 allignment forces the start bubble
        let pair_array = vec![(1, 1), (1, 16), (1, 1)];
        for (allocation_size, alignment) in pair_array {
            let allocation = unsafe { dandelion_alloc(allocation_size, alignment) };
            if allocation.is_null() {
                break;
            }
            let allocation_slice =
                unsafe { slice::from_raw_parts_mut(allocation as *mut u8, allocation_size) };
            for index in 0..allocation_size {
                allocation_slice[index] = 1 + u8::try_from(index.rem(255)).unwrap();
            }
            allocations.push(allocation);
            current_allocated = current_allocated + allocation_size;
        }
        assert_ne!(0, allocations.len());
        for allocation_ptr in allocations {
            unsafe { dandelion_free(allocation_ptr) };
            dandelion_exit_check!(setup, "Freeing memory failed");
        }
    }
}
