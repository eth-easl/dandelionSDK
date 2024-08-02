#[cfg(test)]
mod runtime;

#[cfg(test)]
pub(crate) mod dandelion_structures {

    use libc::{c_char, c_void, size_t};
    use std::sync::{Mutex, MutexGuard};

    extern "C" {
        static mut __dandelion_system_data: DandelionSystemData;
    }

    pub struct SystemDataWrapper {
        system_data: *mut DandelionSystemData,
    }
    unsafe impl Send for SystemDataWrapper {}

    pub struct CurrentSetup {
        guard: MutexGuard<'static, SystemDataWrapper>,
        _heap: Vec<u8>,
        pub heap_range: core::ops::Range<*const u8>,
    }

    impl CurrentSetup {
        pub fn ptr_in_heap(&self, to_check: &*const u8) -> bool {
            return self.heap_range.contains(to_check);
        }
        pub fn exit_code(&self) -> i32 {
            return unsafe { (*self.guard.system_data).exit_code };
        }
    }

    macro_rules! dandelion_exit_check {
        ($lock_guard:expr, $($x:expr), *) => {
            assert_eq!(
                0,
                $lock_guard.exit_code(),
                $(
                    $x,
                )*
            );
        };
    }
    pub(crate) use dandelion_exit_check;

    #[cfg(test)]
    pub fn initialize_dandelion(heap_size: usize) -> CurrentSetup {
        use std::{
            ptr::{addr_of_mut, null_mut},
            sync::Once,
        };

        use crate::runtime;

        static mut LOCK: Mutex<SystemDataWrapper> = Mutex::new(SystemDataWrapper {
            system_data: null_mut(),
        });
        static ONCE: Once = Once::new();
        unsafe {
            ONCE.call_once(|| {
                LOCK = Mutex::new(SystemDataWrapper {
                    system_data: addr_of_mut!(__dandelion_system_data),
                });
            })
        }
        let mut lock_guard = unsafe {
            match LOCK.lock() {
                Ok(guard) => guard,
                Err(poison_error) => {
                    LOCK.clear_poison();
                    poison_error.into_inner()
                }
            }
        };

        println!("aquired lock");
        let mut heap = Vec::with_capacity(heap_size);
        heap.resize(heap_size, 0u8);
        let heap_end = unsafe { heap.as_ptr().byte_add(heap_size) };
        println!(
            "finished allocataion, heap starts at {:?} and ends at {:?}",
            heap.as_ptr(),
            heap_end
        );
        let new_sys_data = DandelionSystemData {
            exit_code: 0,
            heap_begin: heap.as_ptr() as usize,
            heap_end: heap_end as usize,
            input_sets_len: 0,
            input_sets: core::ptr::null_mut(),
            outout_sets_len: 0,
            output_sets: core::ptr::null_mut(),
            input_bufs: core::ptr::null_mut(),
            output_bufs: core::ptr::null_mut(),
        };
        unsafe { *lock_guard.system_data = new_sys_data };
        println!("before init");
        unsafe { runtime::dandelion_init() };
        let setup = CurrentSetup {
            guard: lock_guard,
            heap_range: heap.as_ptr_range(),
            _heap: heap,
        };
        dandelion_exit_check!(setup, "Should not have error after init");
        println!("after init");
        return setup;
    }
    /// dandelion structures to use
    #[repr(C)]
    struct DandelionSystemData {
        /// exit code of the function
        exit_code: i32,
        /// address at which the heap starts
        heap_begin: size_t,
        /// address at which the heap ends
        heap_end: size_t,
        /// number of input sets
        input_sets_len: size_t,
        /// pointer to the array of input set descriptors
        input_sets: *mut IoSetInfo,
        /// number of output sets
        outout_sets_len: size_t,
        /// pointer to the array of output set descriptors
        output_sets: *mut IoSetInfo,
        /// Buffers array with information about input sets
        input_bufs: *mut IoBuffer,
        /// Buffer array with information about output sets
        output_bufs: *mut IoBuffer,
    }

    /// description of a set in the system data
    #[repr(C)]
    struct IoSetInfo {
        /// name of the set (may not be null terminated)
        ident: *const c_char,
        /// length of the name of the set
        ident_len: size_t,
        /// offset in the buffer array where the set begins
        offset: size_t,
    }

    #[repr(C)]
    struct IoBuffer {
        ident: *const c_char,
        ident_len: size_t,
        data: *const c_void,
        data_len: size_t,
        key: size_t,
    }
}
