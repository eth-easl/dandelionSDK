#[cfg(test)]
mod fs_interface;
#[cfg(test)]
mod runtime;

#[cfg(test)]
pub(crate) mod dandelion_structures {

    use core::{slice, str};
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
        _input_sets: Vec<IoSetInfo>,
        _input_buffers: Vec<IoBuffer>,
        _input_data: Vec<Vec<u8>>,
        output_sets: Vec<IoSetInfo>,
    }

    impl CurrentSetup {
        pub fn ptr_in_heap(&self, to_check: &*const u8) -> bool {
            return self.heap_range.contains(to_check);
        }
        pub fn exit_code(&self) -> i32 {
            return unsafe { (*self.guard.system_data).exit_code };
        }
        fn out_buffer_slice(&self) -> &[IoBuffer] {
            let out_buff_number = self.output_sets[self.output_sets.len() - 1].offset;
            if out_buff_number == 0 {
                return &[];
            }
            assert_ne!(core::ptr::null(), self.guard.system_data);
            let system_data = unsafe { &*self.guard.system_data };
            assert_ne!(core::ptr::null(), system_data.output_bufs);
            return unsafe {
                core::slice::from_raw_parts(system_data.output_bufs, out_buff_number)
            };
        }
        pub fn get_item_data(&self, set_name: &str, item_name: &str) -> Option<&[u8]> {
            // find set in set
            let set_index = self
                .output_sets
                .iter()
                .position(|set| {
                    if set.ident_len != set_name.len() {
                        return false;
                    }
                    for name_index in 0..set_name.len() {
                        if unsafe { *set.ident.wrapping_add(name_index) }
                            != set_name.as_bytes()[name_index] as i8
                        {
                            return false;
                        }
                    }
                    true
                })
                .expect("Should find set");
            // there needs to be at least the sentinel set after
            assert!(self.output_sets.len() > set_index + 1);
            // get the slice of buffers belonging to this set
            let first_buffer = self.output_sets[set_index].offset;
            let past_last_buffer = self.output_sets[set_index + 1].offset;
            let out_buffer_slice = &self.out_buffer_slice()[first_buffer..past_last_buffer];
            let buffer_data = match out_buffer_slice.iter().find(|buffer| {
                if buffer.ident_len != item_name.len() {
                    return false;
                }
                for name_index in 0..item_name.len() {
                    let ident_char = unsafe { *buffer.ident.wrapping_add(name_index) };
                    let item_char = item_name.as_bytes()[name_index] as i8;
                    if ident_char != item_char {
                        return false;
                    }
                }
                true
            }) {
                Some(buffer_ref) => buffer_ref,
                None => return None,
            };
            if buffer_data.data_len != 0 && buffer_data.data.is_null() {
                panic!("Non zero length data has NULL pointer");
            }
            return unsafe {
                Some(core::slice::from_raw_parts(
                    buffer_data.data as *const u8,
                    buffer_data.data_len,
                ))
            };
        }

        #[allow(unused)]
        pub fn print_sets_and_items(&self) {
            for sets in self.output_sets.windows(2) {
                let name = str::from_utf8(unsafe {
                    slice::from_raw_parts(sets[0].ident as *const u8, sets[0].ident_len)
                })
                .unwrap();
                println!("set name: {}", name);
                let output_buffer_slice = &self.out_buffer_slice()[sets[0].offset..sets[1].offset];
                for item in output_buffer_slice {
                    let item_name = str::from_utf8(unsafe {
                        slice::from_raw_parts(item.ident as *const u8, item.ident_len)
                    })
                    .unwrap();
                    println!("\titem name: {}", item_name);
                }
            }
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

    pub struct DandelionSet {
        pub ident: &'static str,
        pub items: Vec<DandelionItem>,
    }

    pub struct DandelionItem {
        pub ident: &'static str,
        pub key: usize,
        pub data: Vec<u8>,
    }

    #[cfg(test)]
    pub fn initialize_dandelion(
        heap_size: usize,
        input_sets: Vec<DandelionSet>,
        output_sets: Vec<&'static str>,
    ) -> CurrentSetup {
        use std::{
            ptr::{addr_of_mut, null, null_mut},
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

        let mut heap = Vec::with_capacity(heap_size);
        heap.resize(heap_size, 0u8);
        let heap_end = unsafe { heap.as_ptr().byte_add(heap_size) };
        let mut input_set_array = Vec::new();
        let mut input_buffer_array = Vec::new();
        let mut input_data = Vec::new();
        for input_set in input_sets.into_iter() {
            input_set_array.push(IoSetInfo {
                ident: input_set.ident.as_ptr() as *const i8,
                ident_len: input_set.ident.len(),
                offset: input_data.len(),
            });
            for item in input_set.items.into_iter() {
                input_data.push(item.data);
                input_buffer_array.push(IoBuffer {
                    ident: item.ident.as_ptr() as *const i8,
                    ident_len: item.ident.len(),
                    key: item.key,
                    data: input_data[input_data.len() - 1].as_ptr() as *const c_void,
                    data_len: input_data[input_data.len() - 1].len(),
                })
            }
        }
        // always need sentinel set
        input_set_array.push(IoSetInfo {
            ident: null(),
            ident_len: 0,
            offset: input_data.len(),
        });

        let mut output_set_array = Vec::new();
        for output_set in &output_sets {
            // write set name into heap memory
            output_set_array.push(IoSetInfo {
                ident: output_set.as_ptr() as *const i8,
                ident_len: output_set.len(),
                offset: 0,
            });
        }
        // always need to set sentinel set
        output_set_array.push(IoSetInfo {
            ident: null(),
            ident_len: 0,
            offset: 0,
        });
        let new_sys_data = DandelionSystemData {
            exit_code: 0,
            heap_begin: heap.as_ptr() as usize,
            heap_end: heap_end as usize,
            input_sets_len: input_set_array.len() - 1, // do not count sentinel set
            input_sets: input_set_array.as_mut_ptr(),
            outout_sets_len: output_sets.len(),
            output_sets: output_set_array.as_mut_ptr(),
            input_bufs: input_buffer_array.as_mut_ptr(),
            output_bufs: core::ptr::null_mut(),
        };
        unsafe { *lock_guard.system_data = new_sys_data };
        unsafe { runtime::dandelion_init() };
        let setup = CurrentSetup {
            guard: lock_guard,
            heap_range: heap.as_ptr_range(),
            _heap: heap,
            output_sets: output_set_array,
            _input_sets: input_set_array,
            _input_buffers: input_buffer_array,
            _input_data: input_data,
        };
        dandelion_exit_check!(setup, "Should not have error after init");
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
