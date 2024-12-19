use std::ptr::null;

use libc::{__errno_location, c_char, c_int, size_t};

use crate::{
    dandelion_structures::{
        dandelion_exit_check, initialize_dandelion, DandelionItem, DandelionSet,
    },
    runtime::dandelion_exit,
};

#[repr(C)]
struct DandelionStat {
    st_mode: size_t,
    hard_links: size_t,
    file_size: size_t,
    blk_size: size_t,
}

type ModeT = u32;

extern "C" {
    /// check if the file corresponding to the descriptor is connected to a terminal
    /// not testing isatty for now, as it is hard coded to be true on stdin, stdout and stderr and false otherwise
    // fn dandelion_isatty(file: c_int) -> c_int;
    /// link the file at one path to another path
    fn dandelion_link(old: *const c_char, new: *const c_char) -> c_int;
    /// remove path from pointing to file, if last path leading to file and if it is not open, remove file
    fn dandelion_unlink(name: *const c_char) -> c_int;
    /// open file at path with flags and read/write mode
    fn dandelion_open(name: *const c_char, flags: c_int, mode: ModeT) -> c_int;
    /// reposition file reading/writing offset
    fn dandelion_lseek(file: c_int, offset: c_int, whenece: c_int) -> c_int;
    /// read bytes from file corresponding to descriptor
    fn dandelion_read(
        file: c_int,
        buffer: *mut c_char,
        length: c_int,
        offset: c_int,
        options: c_char,
    ) -> c_int;
    /// write data to file corresponding to file descriptor
    fn dandelion_write(
        file: c_int,
        buffer: *const c_char,
        length: c_int,
        offset: c_int,
        options: c_char,
    ) -> c_int;
    /// close file corresponding to descriptor
    fn dandelion_close(file: c_int) -> c_int;
    /// get the stat for the file corresponding to the descriptor
    fn dandelion_fstat(file: c_int, st: *mut DandelionStat) -> c_int;
    /// get stat for a file using path
    fn dandelion_stat(name: *const c_char, st: *mut DandelionStat) -> c_int;
    /// initialize file system from input sets and create stdio
    fn fs_initialize(
        argc: *mut c_int,
        argv: *mut *const *const c_char,
        environ: *mut *const *const c_char,
    ) -> c_int;
    /// write files into contiguous buffers when necessary and add files as output buffers
    fn fs_terminate() -> c_int;
}

// need to use the dandelion definitions of the option variables
const O_RDONLY: i32 = 0x000;
const O_WRONLY: i32 = 0x001;
const O_RDWR: i32 = 0x002;
const O_APPEND: i32 = 0x008;
const O_CREAT: i32 = 0x200;
const O_TRUNC: i32 = 0x400;
const O_EXCL: i32 = 0x800;
const O_ACCMODE: i32 = O_RDONLY | O_WRONLY | O_RDWR;

const SEEK_SET: i32 = 0; /* set file offset to offset */
const SEEK_CUR: i32 = 1; /* set file offset to current plus offset */
const SEEK_END: i32 = 2; /* set file offset to EOF plus offset */

const S_IXUSR: u32 = 00100;
const S_IWUSR: u32 = 00200;
const S_IRUSR: u32 = 00400;
const S_IRWXU: u32 = S_IXUSR | S_IWUSR | S_IRUSR;

const S_IFDIR: i32 = 0040000;
const S_IFREG: i32 = 0100000;

const USE_OFFSET: c_char = 0x01;
const MOVE_OFFSET: c_char = 0x02;

fn test_write(test_slices: &[&[u8]], file_descriptor: i32, item_name: &str, options: c_char) {
    let heap_size = 16 * 4096;
    let setup = initialize_dandelion(heap_size, Vec::new(), vec!["stdio"]);
    dandelion_exit_check!(setup, "Should have initialized without error");

    // initialize file system
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    let mut total_written = 0;
    for write_slice in test_slices {
        // write to stdout
        let write_bytes = unsafe {
            dandelion_write(
                file_descriptor,
                write_slice.as_ptr() as *const i8,
                write_slice.len() as i32,
                total_written,
                options,
            )
        };
        assert_eq!(
            write_slice.len() as i32,
            write_bytes,
            "Should have written the entire string, errno: {}",
            unsafe { *__errno_location() }
        );
        total_written += write_bytes;
    }
    // should always be able to write to STDOUT and STDERR, which are filedescriptors 1 and 2 respectively
    // these should also be available without explicitly opening them
    let finalize_error = unsafe { fs_terminate() };
    assert_eq!(
        0, finalize_error,
        "finalizing file system should not have any errors"
    );

    unsafe { dandelion_exit(0) };
    dandelion_exit_check!(setup, "Should have exited at end of test without errors");
    // check that stdout and stderr are items in the set
    let item_slice = setup.get_item_data("stdio", item_name);
    let combined_output: Vec<u8> = test_slices
        .into_iter()
        .flat_map(|slice| slice.to_vec())
        .collect();
    assert_eq!(Some(combined_output.as_ref()), item_slice);
}

#[test]
fn write_test() {
    // test if writing works when write fits in a sinlge file chunk
    // use stdout and stderr, as they do not rely on file open to be able to write
    test_write(&["test".as_bytes()], 1, "stdout", MOVE_OFFSET);
    test_write(&["test".as_bytes()], 1, "stdout", USE_OFFSET);
    test_write(&["test".as_bytes()], 2, "stderr", MOVE_OFFSET);
    test_write(&["test".as_bytes()], 2, "stderr", USE_OFFSET);
    // write more that default chunck size so we know it works with mutliple file chunks
    let large_size = 4096 * 2 + 77;
    let mut large_vec = Vec::with_capacity(large_size);
    large_vec.resize(large_size, 7u8);
    test_write(&[&large_vec], 1, "stdout", MOVE_OFFSET);
    test_write(&[&large_vec], 1, "stdout", USE_OFFSET);
    test_write(&[&large_vec], 2, "stderr", MOVE_OFFSET);
    test_write(&[&large_vec], 2, "stderr", USE_OFFSET);

    // test that consecutive writes
    let consequtive_slices = ["first".as_bytes(), "second".as_bytes()];
    test_write(&consequtive_slices, 1, "stdout", MOVE_OFFSET);
    test_write(&consequtive_slices, 1, "stdout", USE_OFFSET);
    test_write(&consequtive_slices, 2, "stderr", MOVE_OFFSET);
    test_write(&consequtive_slices, 2, "stderr", USE_OFFSET);
}

fn test_read(stdin_content: &[u8], read_buffer_size: usize) {
    let heap_size = 16 * 4096;
    let setup = initialize_dandelion(
        heap_size,
        vec![DandelionSet {
            ident: "stdio",
            items: vec![DandelionItem {
                ident: "stdin",
                key: 0,
                data: stdin_content.to_vec(),
            }],
        }],
        Vec::new(),
    );
    dandelion_exit_check!(setup, "Should have initialized without error");

    // // initialize file system
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    // // read to stdin
    let mut read_buffer = Vec::with_capacity(read_buffer_size);
    read_buffer.resize(read_buffer_size, 0u8);
    let mut total_read = 0;
    for chunk in stdin_content.chunks(read_buffer_size) {
        // read with advancing the offset
        let read_bytes = unsafe {
            dandelion_read(
                0,
                read_buffer.as_mut_ptr() as *mut i8,
                read_buffer.len() as i32,
                0,
                MOVE_OFFSET,
            )
        };
        assert_eq!(
            chunk.len() as i32,
            read_bytes,
            "Reading returning not the expected amount of bytes for chunk size {}",
            read_buffer_size
        );
        assert_eq!(
            chunk,
            &read_buffer[0..read_bytes as usize],
            "Read content no what was expected",
        );
        // read without advancing directly using offsets
        let read_bytes = unsafe {
            dandelion_read(
                0,
                read_buffer.as_mut_ptr() as *mut i8,
                read_buffer.len() as i32,
                total_read,
                USE_OFFSET,
            )
        };
        assert_eq!(
            chunk.len() as i32,
            read_bytes,
            "Reading using offset not the expected amounts of bytes for chunk size {}",
            read_buffer_size
        );
        assert_eq!(
            chunk,
            &read_buffer[0..read_bytes as usize],
            "Content reading from offset not what expected"
        );
        total_read += read_bytes;
    }
}

#[test]
fn read_test() {
    // test if reading works, using stdin as it does not rely on open to work.
    // attempt to read from empty stdin as that should always be fine

    // start with no input to stdin to check default behaviour
    {
        let heap_size = 16 * 4096;
        let setup = initialize_dandelion(heap_size, Vec::new(), Vec::new());
        dandelion_exit_check!(setup, "Should have initialized without error");

        // initialize file system
        let mut argc = 0;
        let mut argv = null();
        let mut environ = null();
        let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
        assert_eq!(0, initialize_val, "Failed to initialize file system");

        // read to stdin
        let mut read_buffer = [0i8; 12];
        let read_bytes = unsafe {
            dandelion_read(
                0,
                read_buffer.as_mut_ptr() as *mut i8,
                read_buffer.len() as i32,
                0,
                MOVE_OFFSET,
            )
        };
        assert_eq!(
            0, read_bytes,
            "Reading empty stdin should always read 0 bytes",
        );
    }
    // test with different input and buffer lengths
    test_read("test".as_bytes(), 1);
    test_read("test".as_bytes(), 2);
    test_read("test".as_bytes(), 3);
    test_read("test".as_bytes(), 4);
    test_read("test".as_bytes(), 7);
    // 16 bit pattern to cycle through
    // use 2 8 bit counters, one counting up, the other counting down
    let mut lower_half = (1u8..=255u8).cycle();
    let mut upper_half = (1u8..=255u8).rev().cycle();
    let total_size = 2 * 4096 + 77;
    let mut data = Vec::new();
    for _ in 0..total_size {
        data.push(upper_half.next().unwrap());
        data.push(lower_half.next().unwrap());
    }
    test_read(&data, 1);
    test_read(&data, 2);
    test_read(&data, 3);
    test_read(&data, 4);
    test_read(&data, 77);
    test_read(&data, 999);
}

fn open_and_read(path: &str, expected_filedescriptor: c_int, expected_content: &[u8]) {
    let input_file_desc = unsafe { dandelion_open(path.as_ptr() as *const i8, O_RDONLY, 0) };
    assert_eq!(
        expected_filedescriptor, input_file_desc,
        "input file unexpected file descriptor for file {}",
        path
    );
    // read to stdin
    let mut read_buffer = Vec::with_capacity(expected_content.len());
    read_buffer.resize(expected_content.len(), 0u8);
    let read_bytes = unsafe {
        dandelion_read(
            input_file_desc,
            read_buffer.as_mut_ptr() as *mut i8,
            read_buffer.len() as i32,
            0,
            MOVE_OFFSET,
        )
    };
    assert_eq!(
        expected_content.len(),
        read_bytes as usize,
        "Should be able to read entire expected content"
    );
    assert_eq!(expected_content, &read_buffer);
}

fn create_and_write(path: &str, expected_filedescriptor: c_int, content: &[u8]) {
    let file_desc =
        unsafe { dandelion_open(path.as_ptr() as *const i8, O_WRONLY | O_CREAT, S_IWUSR) };
    assert_eq!(
        expected_filedescriptor, file_desc,
        "input file unexpected file descriptor for file {}",
        path
    );
    // read to stdin
    let written_bytes = unsafe {
        dandelion_write(
            file_desc,
            content.as_ptr() as *mut i8,
            content.len() as i32,
            0,
            MOVE_OFFSET,
        )
    };
    assert_eq!(
        content.len(),
        written_bytes as usize,
        "Should be able to write entire content"
    );
}

#[test]
fn open_test() {
    {
        let heap_size = 16 * 4096;
        let input_file_content = "abc".as_bytes();
        let file_in_folder_content = "def".as_bytes();
        let another_file_content = "ghi".as_bytes();
        let input_sets = vec![
            DandelionSet {
                ident: "input_folder",
                items: vec![DandelionItem {
                    ident: "input_file",
                    key: 0,
                    data: input_file_content.to_vec(),
                }],
            },
            DandelionSet {
                ident: "input_nested",
                items: vec![
                    DandelionItem {
                        ident: "nested_file/file_in_folder",
                        key: 0,
                        data: file_in_folder_content.to_vec(),
                    },
                    DandelionItem {
                        ident: "nested_file/another_folder/another_file",
                        key: 0,
                        data: another_file_content.to_vec(),
                    },
                ],
            },
        ];
        let output_sets = vec!["output_folder", "output_nested"];
        let setup = initialize_dandelion(heap_size, input_sets, output_sets);
        dandelion_exit_check!(setup, "Should have initialized without error");

        // initialize file system
        let mut argc = 0;
        let mut argv = null();
        let mut environ = null();
        let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
        assert_eq!(0, initialize_val, "Failed to initialize file system");

        // open the input files
        open_and_read("/input_folder/input_file\0", 3, input_file_content);
        // check that folders inside the item name works
        open_and_read(
            "/input_nested/nested_file/file_in_folder\0",
            4,
            file_in_folder_content,
        );
        open_and_read(
            "/input_nested/nested_file/another_folder/another_file\0",
            5,
            another_file_content,
        );
        create_and_write("/output_folder/output_file\0", 6, input_file_content);
        create_and_write(
            "/output_nested/nested_file/file_in_folder\0",
            7,
            file_in_folder_content,
        );
        create_and_write(
            "/output_nested/nested_file/another_folder/another_file\0",
            8,
            another_file_content,
        );
        // get all files written to the setup struct
        let finalize_error = unsafe { fs_terminate() };
        assert_eq!(
            0, finalize_error,
            "finalizing file system should not have any errors"
        );
        unsafe { dandelion_exit(0) };
        dandelion_exit_check!(setup, "Should have exited at end of test without errors");
        // check the correct output was presented
        let output_file_result = setup.get_item_data("output_folder", "output_file");
        assert_eq!(Some(input_file_content), output_file_result);
        let nested_file_result = setup.get_item_data("output_nested", "nested_file/file_in_folder");
        assert_eq!(Some(file_in_folder_content), nested_file_result);
        let another_file_result =
            setup.get_item_data("output_nested", "nested_file/another_folder/another_file");
        assert_eq!(Some(another_file_content), another_file_result);
    }
}

#[test]
fn close_test() {
    // set up dandelion and initialize file system
    let heap_size = 16 * 4096;
    let setup = initialize_dandelion(heap_size, Vec::new(), Vec::new());
    dandelion_exit_check!(setup, "Should have initialized without error");
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    // we can close stdin and check that we cannot read from it anymore
    let stdin_close = unsafe { dandelion_close(0) };
    assert_eq!(0, stdin_close);
    // check that we can't close it again
    let stdin_reclose = unsafe { dandelion_close(0) };
    assert_eq!(-libc::EBADF, stdin_reclose);
    // check that we can't read from it
    let mut test_slice = [0u8; 1];
    let read_bytes = unsafe {
        dandelion_read(
            0,
            test_slice.as_mut_ptr() as *mut i8,
            test_slice.len() as i32,
            0,
            MOVE_OFFSET,
        )
    };
    assert_eq!(-libc::EBADF, read_bytes);

    // close stdout and check we can't write to it
    let stdout_close = unsafe { dandelion_close(1) };
    assert_eq!(0, stdout_close);
    // check that we can't close it again
    let stdout_reclose = unsafe { dandelion_close(1) };
    assert_eq!(-libc::EBADF, stdout_reclose);
    let written_bytes = unsafe {
        dandelion_write(
            1,
            test_slice.as_ptr() as *const i8,
            test_slice.len() as i32,
            0,
            MOVE_OFFSET,
        )
    };
    assert_eq!(-libc::EBADF, written_bytes);
}

#[test]
fn lseek_test() {
    let heap_size = 16 * 4096;
    let mut input_slice = [1u8; 128];
    let mut file_size = input_slice.len();
    for index in 0..128 {
        input_slice[index] = (index + 1) as u8;
    }
    let inputs = vec![DandelionSet {
        ident: "input",
        items: vec![DandelionItem {
            ident: "input_file",
            key: 0,
            data: input_slice.to_vec(),
        }],
    }];
    let setup = initialize_dandelion(heap_size, inputs, Vec::new());
    dandelion_exit_check!(setup, "Should have initialized without error");

    // initialize file system
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    // open file
    let file_descriptor =
        unsafe { dandelion_open("/input/input_file\0".as_ptr() as *const i8, O_RDWR, 0) };
    assert!(file_descriptor > 0);

    let mut read_buffer = [0i8; 1];

    // seek into middle from begginning
    let mut expected_offset = 64;
    let mut current_offset = unsafe { dandelion_lseek(file_descriptor, expected_offset, SEEK_SET) };
    assert_eq!(expected_offset, current_offset);
    // try reading to check it is actually at the expected place
    let mut read_bytes =
        unsafe { dandelion_read(file_descriptor, read_buffer.as_mut_ptr(), 1, 0, MOVE_OFFSET) };
    assert_eq!(1, read_bytes);
    expected_offset += 1;
    assert_eq!([expected_offset as i8], read_buffer);
    // seek from current position
    current_offset = unsafe { dandelion_lseek(file_descriptor, 4, SEEK_CUR) };
    expected_offset += 4;
    assert_eq!(expected_offset, current_offset);
    read_bytes =
        unsafe { dandelion_read(file_descriptor, read_buffer.as_mut_ptr(), 1, 0, MOVE_OFFSET) };
    assert_eq!(1, read_bytes);
    expected_offset += 1;
    assert_eq!([expected_offset as i8], read_buffer);
    // seek from end
    current_offset = unsafe { dandelion_lseek(file_descriptor, 4, SEEK_END) };
    expected_offset = input_slice.len() as i32 + 4;
    file_size += 4;
    assert_eq!(
        expected_offset,
        current_offset,
        "failed with errno {}",
        unsafe { *__errno_location() }
    );
    // try read, should get nothing
    read_bytes =
        unsafe { dandelion_read(file_descriptor, read_buffer.as_mut_ptr(), 1, 0, MOVE_OFFSET) };
    assert_eq!(0, read_bytes);
    // set from begginning to previous length and read a zero
    current_offset =
        unsafe { dandelion_lseek(file_descriptor, input_slice.len() as i32, SEEK_SET) };
    assert_eq!(input_slice.len() as i32, current_offset);
    read_bytes =
        unsafe { dandelion_read(file_descriptor, read_buffer.as_mut_ptr(), 1, 0, MOVE_OFFSET) };
    assert_eq!(1, read_bytes);
    assert_eq!([0i8], read_buffer);
    // set from current and read a zero
    current_offset = unsafe { dandelion_lseek(file_descriptor, 1, SEEK_CUR) };
    assert_eq!(input_slice.len() as i32 + 2, current_offset);
    read_bytes =
        unsafe { dandelion_read(file_descriptor, read_buffer.as_mut_ptr(), 1, 0, MOVE_OFFSET) };
    assert_eq!(1, read_bytes);
    assert_eq!([0i8], read_buffer);
    // check negative offsets work, by going back from end
    expected_offset = 32;
    current_offset = unsafe {
        dandelion_lseek(
            file_descriptor,
            -(file_size as i32) + expected_offset,
            SEEK_END,
        )
    };
    assert_eq!(expected_offset, current_offset);
    read_bytes =
        unsafe { dandelion_read(file_descriptor, read_buffer.as_mut_ptr(), 1, 0, MOVE_OFFSET) };
    assert_eq!(1, read_bytes);
    expected_offset += 1;
    assert_eq!([expected_offset as i8], read_buffer);
}

#[test]
fn link_test() {
    // setup inputs, relink them to output folders and check if the outputs are available
    let heap_size = 16 * 4096;
    let file_name = "file";
    let file_content = "abc".as_bytes();
    let file_in_folder_name = "nested_file/file_in_folder";
    let file_in_folder_content = "def".as_bytes();
    let another_file_name = "nested_file/another_folder/another_file";
    let another_file_content = "ghi".as_bytes();
    let input_sets = vec![
        DandelionSet {
            ident: "input_folder",
            items: vec![DandelionItem {
                ident: file_name,
                key: 0,
                data: file_content.to_vec(),
            }],
        },
        DandelionSet {
            ident: "input_nested",
            items: vec![
                DandelionItem {
                    ident: file_in_folder_name,
                    key: 0,
                    data: file_in_folder_content.to_vec(),
                },
                DandelionItem {
                    ident: another_file_name,
                    key: 0,
                    data: another_file_content.to_vec(),
                },
            ],
        },
    ];
    let output_sets = vec!["output_folder", "output_nested"];
    let setup = initialize_dandelion(heap_size, input_sets, output_sets);
    dandelion_exit_check!(setup, "Should have initialized without error");
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    // relink inputs to output folders
    let file_old_path = format!("/input_folder/{}\0", file_name);
    let file_new_path = format!("/output_folder/{}\0", file_name);
    let link_result = unsafe {
        dandelion_link(
            file_old_path.as_ptr() as *const i8,
            file_new_path.as_ptr() as *const i8,
        )
    };
    assert_eq!(0, link_result);
    let file_old_path = format!("/input_nested/{}\0", file_in_folder_name);
    let file_new_path = format!("/output_nested/{}\0", file_in_folder_name);
    let link_result = unsafe {
        dandelion_link(
            file_old_path.as_ptr() as *const i8,
            file_new_path.as_ptr() as *const i8,
        )
    };
    assert_eq!(0, link_result);
    let file_old_path = format!("/input_nested/{}\0", another_file_name);
    let file_new_path = format!("/output_nested/{}\0", another_file_name);
    let link_result = unsafe {
        dandelion_link(
            file_old_path.as_ptr() as *const i8,
            file_new_path.as_ptr() as *const i8,
        )
    };
    assert_eq!(0, link_result);

    // try different linkings that should produce error
    // not existing old path
    let link_result = unsafe {
        dandelion_link(
            "/non_exitent/file\0".as_ptr() as *const i8,
            "\0".as_ptr() as *const i8,
        )
    };
    assert_eq!(-libc::ENOTDIR, link_result);
    // already exsiting new_path
    let link_result = unsafe {
        dandelion_link(
            file_old_path.as_ptr() as *const i8,
            file_new_path.as_ptr() as *const i8,
        )
    };
    assert_eq!(-libc::EEXIST, link_result);
    // should fail if the old file is a directory
    let link_result = unsafe {
        dandelion_link(
            "/input_nested".as_ptr() as *const i8,
            "/test_file".as_ptr() as *const i8,
        )
    };
    assert_eq!(-libc::ENOTDIR, link_result);

    // check the files we expect in the output with the expected content
    let finalize_error = unsafe { fs_terminate() };
    assert_eq!(
        0, finalize_error,
        "finalizing file system should not have any errors"
    );
    unsafe { dandelion_exit(0) };
    dandelion_exit_check!(setup, "Should have exited at end of test without errors");
    // check the correct output was presented
    let file_result = setup.get_item_data("output_folder", &file_name);
    assert_eq!(Some(file_content), file_result);
    let nested_file_result = setup.get_item_data("output_nested", "nested_file/file_in_folder");
    assert_eq!(Some(file_in_folder_content), nested_file_result);
    let another_file_result =
        setup.get_item_data("output_nested", "nested_file/another_folder/another_file");
    assert_eq!(Some(another_file_content), another_file_result);
}

#[test]
fn unlink_test() {
    // setup inputs, relink them to output folders and check if the outputs are available
    let heap_size = 16 * 4096;
    let file_name = "file";
    let file_content = "abc".as_bytes();
    let file_in_folder_name = "nested_file/file_in_folder";
    let file_in_folder_content = "def".as_bytes();
    let another_file_name = "nested_file/another_folder/another_file";
    let another_file_content = "ghi".as_bytes();
    let input_sets = vec![
        DandelionSet {
            ident: "folder",
            items: vec![DandelionItem {
                ident: file_name,
                key: 0,
                data: file_content.to_vec(),
            }],
        },
        DandelionSet {
            ident: "nested",
            items: vec![
                DandelionItem {
                    ident: file_in_folder_name,
                    key: 0,
                    data: file_in_folder_content.to_vec(),
                },
                DandelionItem {
                    ident: another_file_name,
                    key: 0,
                    data: another_file_content.to_vec(),
                },
            ],
        },
    ];
    let output_sets = vec!["folder", "nested"];
    let setup = initialize_dandelion(heap_size, input_sets, output_sets);
    dandelion_exit_check!(setup, "Should have initialized without error");
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    // unlink folder so they are not added to output
    let unlink_error = unsafe { dandelion_unlink("/folder/file\0".as_ptr() as *const i8) };
    assert_eq!(0, unlink_error);
    let unlink_error =
        unsafe { dandelion_unlink("/nested/nested_file/file_in_folder\0".as_ptr() as *const i8) };
    assert_eq!(0, unlink_error);
    let unlink_error = unsafe {
        dandelion_unlink("/nested/nested_file/another_folder/another_file\0".as_ptr() as *const i8)
    };
    assert_eq!(0, unlink_error);

    // check that doing it twice fails
    let unlink_error = unsafe {
        dandelion_unlink("/nested/nested_file/another_folder/another_file\0".as_ptr() as *const i8)
    };
    assert_eq!(-libc::ENOTDIR, unlink_error);

    // check the files we expect in the output with the expected content
    let finalize_error = unsafe { fs_terminate() };
    assert_eq!(
        0, finalize_error,
        "finalizing file system should not have any errors"
    );
    unsafe { dandelion_exit(0) };
    setup.print_sets_and_items();
    dandelion_exit_check!(setup, "Should have exited at end of test without errors");
    // check the correct output was presented
    let file_result = setup.get_item_data("folder", &file_name);
    assert_eq!(None, file_result);
    let nested_file_result = setup.get_item_data("nested", &file_in_folder_name);
    assert_eq!(None, nested_file_result);
    let another_file_result = setup.get_item_data("nested", &another_file_name);
    assert_eq!(None, another_file_result);
}

#[test]
fn stat_test() {
    // setup inputs, relink them to output folders and check if the outputs are available
    let heap_size = 16 * 4096;
    let file_name = "file";
    let file_content = "abcdefg".as_bytes();
    let input_sets = vec![DandelionSet {
        ident: "folder",
        items: vec![DandelionItem {
            ident: file_name,
            key: 0,
            data: file_content.to_vec(),
        }],
    }];

    let output_sets = vec!["folder", "nested"];
    let setup = initialize_dandelion(heap_size, input_sets, output_sets);
    dandelion_exit_check!(setup, "Should have initialized without error");
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    let mut stat: DandelionStat = unsafe { std::mem::zeroed() };
    let stat_result = unsafe { dandelion_stat("/folder/file\0".as_ptr() as *const i8, &mut stat) };
    assert_eq!(0, stat_result);
    assert_eq!(1, stat.hard_links);
    assert_eq!(7, stat.file_size);
}

#[test]
fn fstat_test() {
    // setup inputs, relink them to output folders and check if the outputs are available
    let heap_size = 16 * 4096;
    let file_name = "file";
    let file_content = "abcdefg".as_bytes();
    let input_sets = vec![DandelionSet {
        ident: "folder",
        items: vec![DandelionItem {
            ident: file_name,
            key: 0,
            data: file_content.to_vec(),
        }],
    }];

    let output_sets = vec!["folder", "nested"];
    let setup = initialize_dandelion(heap_size, input_sets, output_sets);
    dandelion_exit_check!(setup, "Should have initialized without error");
    let mut argc = 0;
    let mut argv = null();
    let mut environ = null();
    let initialize_val = unsafe { fs_initialize(&mut argc, &mut argv, &mut environ) };
    assert_eq!(0, initialize_val, "Failed to initialize file system");

    let file_descriptor = unsafe { dandelion_open("/folder/file\0".as_ptr() as *const i8, 0, 0) };
    assert_ne!(-1, file_descriptor);
    let mut stat: DandelionStat = unsafe { std::mem::zeroed() };
    let stat_result = unsafe { dandelion_fstat(file_descriptor, &mut stat) };
    assert_eq!(0, stat_result);
    assert_eq!(2, stat.hard_links);
    assert_eq!(7, stat.file_size);
}
// TODO permission checks, write to read only file, etc.
