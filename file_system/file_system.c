#include "file_system.h"
#include "include/fs_interface.h"

#include <stdint.h>

#include "dandelion/runtime.h"
#include "dandelion/system/system.h"
#include "paths.h"

// static variables
D_File *fs_root;
OpenFile *open_files;

D_File *create_file(Path *name, char *content, size_t length, uint32_t mode) {
  D_File *new_file = dandelion_alloc(sizeof(D_File), _Alignof(D_File));
  if (new_file == NULL) {
    return NULL;
  }
  if (name->length > FS_NAME_LENGHT) {
    return NULL;
  }
  memcpy(new_file->name, name->path, name->length);
  new_file->type = FILE;
  if (content != NULL) {
    FileChunk *new_content =
        dandelion_alloc(sizeof(FileChunk), _Alignof(FileChunk));
    if (new_content == NULL) {
      return NULL;
    }
    new_content->next = NULL;
    new_content->data = content;
    new_content->capacity = length;
    new_content->used = length;
    new_file->content = new_content;
  } else {
    new_file->content = NULL;
  }
  new_file->hard_links = 0;
  new_file->open_descripotors = 0;
  new_file->mode = mode | S_IFREG;
  return new_file;
}

D_File *create_directory(Path *name, uint32_t mode) {
  D_File *new_file = dandelion_alloc(sizeof(D_File), _Alignof(D_File));
  if (name->length > FS_NAME_LENGHT) {
    return NULL;
  }
  memcpy(new_file->name, name->path, name->length);
  new_file->type = DIRECTORY;
  new_file->child = NULL;
  new_file->hard_links = 0;
  // directory where user has full permissions
  new_file->mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR;
  return new_file;
}

int link_file_to_folder(D_File *folder, D_File *file) {
  if (folder->type != DIRECTORY) {
    // TODO set proper error number
    return -1;
  };
  // set file parent to folder
  file->parent = folder;
  // increase the number of hard links to the file
  file->hard_links += 1;
  if (folder->child == NULL) {
    folder->child = file;
    file->next = NULL;
  } else {
    D_File *current = folder->child;
    if (namecmp(file->name, current->name, FS_NAME_LENGHT) < 0) {
      file->next = current;
      folder->child = file;
    } else {
      while (current->next != NULL) {
        if (namecmp(file->name, current->next->name, FS_NAME_LENGHT) < 0) {
          break;
        } else {
          current = current->next;
        }
      }
      file->next = current->next;
      current->next = file;
    }
  }
  return 0;
}

D_File *find_file_in_dir(D_File *directory, Path file) {
  // handle special cases for . and ..
  if (file.length == 1 && file.path[0] == '.') {
    return directory;
  }
  if (file.length == 2 && file.path[0] == '.' && file.path[1] == '.') {
    return directory->parent;
  }
  for (D_File *current = directory->child; current != NULL;
       current = current->next) {
    int cmp_result =
        namecmp(current->name, file.path, MIN(file.length, FS_NAME_LENGHT));
    if (cmp_result == 0) {
      return current;
    } else if (cmp_result > 0) {
      // can stop since the list is sorted
      return NULL;
    }
  }
  return NULL;
}

D_File *find_file_path(Path file_path) {
  D_File *current = fs_root;
  for (Path subpath = get_component_advance(&file_path); subpath.length != 0;
       subpath = get_component_advance(&file_path)) {
    if (current->type != DIRECTORY) {
      return NULL;
    }
    current = find_file_in_dir(current, subpath);
    if (current == NULL) {
      return NULL;
    }
  }
  return current;
}

D_File *find_file(const char *name) {
  Path file_path = path_from_string(name);
  return find_file_path(file_path);
}

// follow a path and create all directories on the way that do not already
// exist prevent up prevents moving up in the file tree, to prevent input
// items getting written into different input sets by going up the file tree
D_File *create_directories(D_File *directory, Path path, char prevent_up) {
  if (directory->type != DIRECTORY) {
    return NULL;
  }
  for (Path current_path = get_component_advance(&path);
       current_path.length > 0; current_path = get_component_advance(&path)) {
    if (current_path.length > FS_NAME_LENGHT) {
      return NULL;
    } else if (current_path.length == 1 && current_path.path[0] == '.') {
      // handle special case of single dot for current directory
      continue;
    } else if (current_path.length == 2 && current_path.path[0] == '.' &&
               current_path.path[1] == '.') {
      // handle ".." for moving up one directory
      if (prevent_up != 0) {
        return NULL;
      } else if (directory->parent == NULL) {
        continue;
      } else {
        directory = directory->parent;
      }
    } else {
      // check if direcotory exists, otherwise create it and move into it
      D_File *candidate_dir = find_file_in_dir(directory, current_path);
      if (candidate_dir != NULL) {
        directory = candidate_dir;
        continue;
      }
      D_File *new_dir = dandelion_alloc(sizeof(D_File), _Alignof(D_File));
      new_dir->type = DIRECTORY;
      memcpy(new_dir->name, current_path.path, current_path.length);
      new_dir->child = NULL;
      int error = link_file_to_folder(directory, new_dir);
      if (error < 0) {
        dandelion_free(new_dir);
        return NULL;
      }
      directory = new_dir;
    }
  }
  return directory;
}

// free all file chunks in a chunk list and their data
void free_file_chunks(FileChunk *first) {
  FileChunk *next_chunk = NULL;
  for (FileChunk *chunck = first; chunck != NULL; chunck = next_chunk) {
    next_chunk = chunck->next;
    dandelion_free(chunck->data);
    dandelion_free(chunck);
  }
}

int free_data(D_File *file) {
  if (file->hard_links != 0 || file->open_descripotors != 0) {
    return 0;
  }
  switch (file->type) {
  case DIRECTORY:
    for (D_File *child = file->child; child != NULL; child = child->next) {
      free_data(child);
    }
    break;
  case FILE:
    free_file_chunks(file->content);
    break;
  default:
    // unkown file type
    return -1;
  }
  dandelion_free(file);
  return 0;
}

int remove_file(D_File *file) {
  D_File *parent = file->parent;
  // iterate through children until we find the file
  if (parent == NULL || parent->type != DIRECTORY) {
    // parent not directory
    return -1;
  }
  D_File *child = parent->child;
  if (child == file) {
    parent->child = file->next;
  } else {
    while (child->next != NULL && child->next != file) {
      child = child->next;
    }
    if (child->next == file) {
      child->next = file->next;
    } else {
      // file not in parent directory
      return -1;
    }
  }
  file->hard_links -= 1;
  int error = free_data(file);
  if (error != 0)
    return error;
  return 0;
}

// open file for which the path was already present and the file exists
// assumes file is non null
// Flags that need to be supported / checked on layers above:
// O_CREAT, O_EXCL, O_TMPFILE
int open_existing_file(unsigned int index, D_File *file, int flags,
                       uint32_t mode, char skip_checks) {
  // Currently ignoring the following flags, noting here so we know they
  // were not forgotten and nulling them so we don't read them on accident
  // flags &= ~(O_ASYNC | O_CLOEXEC | O_DIRECT | O_DIRECTORY | O_DSYNC |
  //  O_LARGEFILE | O_NOATIME | O_NOCTTY | O_NOFOLLOW | O_NONBLOCK |
  //  O_NDELAY | O_PATH | O_SYNC);
  // the original posix only knows there so we don't need to zero the others
  // out for now
  // flags &= ~(O_ASYNC | O_NOCTTY | O_NONBLOCK | O_NDELAY | O_SYNC);

  // check access mode is valid
  int access_mode = flags & O_ACCMODE;

  // on file creation skip access checks as per spec
  // (a create file open can return a read/write file descriptor, even when
  // creating read only file)
  if (skip_checks) {
    // TODO implement full checking when implementing more access control as
    // well as user checking check if opened for reading against read
    // permissions
    if ((!(file->mode & S_IRUSR)) && (access_mode & O_RDONLY)) {
      return -EACCES;
    }
    // check if opened for writing against write permissions
    if ((!(file->mode & S_IWUSR)) && (access_mode & O_WRONLY)) {
      return -EACCES;
    }
  }

  // need to remember if the file was opened with O_APPEND for later when
  // writing, as on first write the write goes to end of file, but reads
  // before are from the start
  access_mode |= flags & O_APPEND;

  // check for truncation
  if ((flags & O_TRUNC) &&
      ((access_mode & O_WRONLY) && (file->mode & S_IWUSR) || skip_checks) &&
      (file->type == FILE)) {
    free_file_chunks(file->content);
    file->content = NULL;
  }

  FileChunk *new_chunk = NULL;

  if (file->type == FILE) {
    new_chunk = file->content;
  }
  // an open file stays open even if it is removed from the file system, so
  // we add a hard link while it is open
  file->hard_links += 1;
  // mark that the file is open
  file->open_descripotors += 1;

  OpenFile new_file = {
      .file = file,
      .offset = 0,
      .current_chunk = new_chunk,
      .open_flags = access_mode,
  };
  open_files[index] = new_file;

  return 0;
}

void setup_charpparray(char *data, size_t length, int *entries,
                       char ***pp_array) {
  if (data == NULL || length == 0) {
    *entries = 0;
    *pp_array = dandelion_alloc(sizeof(char *), _Alignof(char *));
    pp_array[0] = NULL;
    return;
  }
  // find the number of arguments
  size_t arguments = 0;
  size_t data_index = 0;
  while (data_index < length) {
    // skip over leading spaces
    while (data_index < length && data[data_index] == ' ')
      data_index++;
    if (data_index >= length)
      break;
    arguments++;
    // are not at the end of the input and have a non space character so are at
    // the start of an argument skip over current argument
    while (data_index < length && data[data_index] != ' ') {
      // if is escape character, skip to end of escape
      if (data[data_index] == '\'' || data[data_index] == '\"') {
        char escape_char = data[data_index];
        data_index++;
        while (data_index < length && data[data_index] != escape_char)
          data_index++;
      }
      // either move past the character or over the ending escape character
      data_index++;
    }
  }

  // allocate space for the string pointers
  char **pp_local =
      dandelion_alloc(sizeof(char *) * arguments + 1, _Alignof(char *));
  pp_local[arguments] = NULL;

  // reset index
  data_index = 0;
  size_t current_arg = 0;
  while (data_index < length) {
    // skip over spaces
    while (data_index < length && data[data_index] == ' ')
      data_index++;
    if (data_index >= length)
      break;
    // count the number of character the final string will have
    size_t num_characters = 0;
    size_t arg_index = data_index;
    while (data_index < length && data[data_index] != ' ') {
      // if is escape character, skip to end of escape
      if (data[data_index] == '\'' || data[data_index] == '\"') {
        char escape_char = data[data_index];
        data_index++;
        while (data_index < length && data[data_index] != escape_char) {
          num_characters++;
          data_index++;
        }
      } else {
        num_characters++;
      }
      // either move past the character or over the ending escape character
      data_index++;
    }
    // allocate string
    char *target_string = dandelion_alloc(num_characters + 1, _Alignof(char));
    target_string[num_characters] = '\0';
    size_t chars_copied = 0;
    for (; arg_index < data_index; arg_index++) {
      if (data[arg_index] == '\'' || data[arg_index] == '\"') {
        char escape_char = data[arg_index];
        arg_index++;
        for (; arg_index < data_index && data[arg_index] != escape_char;
             arg_index++) {
          target_string[chars_copied] = data[arg_index];
          chars_copied++;
        }
      } else {
        target_string[chars_copied] = data[arg_index];
        chars_copied++;
      }
    }
    pp_local[current_arg] = target_string;
    current_arg++;
  }

  *entries = arguments;
  *pp_array = pp_local;
  return;
}

int fs_initialize(int *argc, char ***argv, char ***environ) {
  // error value
  int error;

  // create root folder
  fs_root = dandelion_alloc(sizeof(D_File), _Alignof(D_File));
  if (fs_root == NULL) {
    dandelion_exit(ENOMEM);
    return -1;
  }
  fs_root->name[0] = '/';
  fs_root->name[1] = 0;
  fs_root->type = DIRECTORY;
  fs_root->next = NULL;
  fs_root->parent = NULL;
  fs_root->child = NULL;
  fs_root->hard_links = 1;

  // create stdio folder and stdout/stderr file
  D_File *stdio_folder = dandelion_alloc(sizeof(D_File), _Alignof(D_File));
  if (stdio_folder == NULL) {
    dandelion_exit(ENOMEM);
    return -1;
  }
  memcpy(stdio_folder->name, "stdio", 6);
  stdio_folder->type = DIRECTORY;
  stdio_folder->child = NULL;
  stdio_folder->hard_links = 0;
  if ((error = link_file_to_folder(fs_root, stdio_folder)) != 0) {
    return error;
  }

  // allocate the file table
  open_files =
      dandelion_alloc(sizeof(OpenFile) * FS_MAX_FILES, _Alignof(OpenFile));
  if (open_files == NULL) {
    dandelion_exit(ENOMEM);
    return -1;
  }
  // make sure file pointers are zeroed, as we used them to detect used files
  for (size_t index = 0; index < FS_MAX_FILES; index++) {
    open_files[index].file = NULL;
  }

  // create and open stderr
  Path stderr_path = path_from_string("stderr");
  D_File *stderr_file = create_file(&stderr_path, NULL, 0, S_IWUSR);
  if (stderr_file == NULL)
    return -1;
  error = open_existing_file(STDERR_FILENO, stderr_file, O_WRONLY, 0, 0);
  if (error != 0)
    return error;
  error = link_file_to_folder(stdio_folder, stderr_file);
  if (error != 0)
    return error;

  // create and open stdout
  Path stdout_path = path_from_string("stdout");
  D_File *stdout_file = create_file(&stdout_path, NULL, 0, S_IWUSR);
  if (stdout_file == NULL)
    return -1;
  error = open_existing_file(STDOUT_FILENO, stdout_file, O_WRONLY, 0, 0);
  if (error != 0)
    return error;
  error = link_file_to_folder(stdio_folder, stdout_file);
  if (error != 0)
    return error;

  // set to NULL to be able to check if it was set by inputs
  *argc = 0;
  *argv = NULL;
  *environ = NULL;

  // add all named items from all names sets
  size_t input_sets = dandelion_input_set_count();
  for (size_t set_index = 0; set_index < input_sets; set_index++) {
    // get set information
    Path set_path = {.path = dandelion_input_set_ident(set_index),
                     .length = dandelion_input_set_ident_len(set_index)};
    if (set_path.length == 0)
      continue;
    // create directories for set
    D_File *set_directory = create_directories(fs_root, set_path, 1);
    if (set_directory == NULL) {
      // TODO write to stderr on what happened
      return -1;
    }
    int is_stdio_folder =
        namecmp(set_path.path, "stdio", MIN(set_path.length, 5));
    size_t input_items = dandelion_input_buffer_count(set_index);
    for (size_t item_index = 0; item_index < input_items; item_index++) {
      IoBuffer *item_buffer = dandelion_get_input(set_index, item_index);

      Path total_path = {.path = item_buffer->ident,
                         .length = item_buffer->ident_len};
      if (total_path.length == 0)
        continue;
      Path dir_path = get_directories(total_path);
      Path file_path = get_file(total_path);
      D_File *item_dir = create_directories(set_directory, dir_path, 1);
      if (item_dir == NULL) {
        return -1;
      }
      D_File *item_file = create_file(&file_path, item_buffer->data,
                                      item_buffer->data_len, S_IRWXU);
      if (item_file == NULL) {
        return -1;
      }
      error = link_file_to_folder(item_dir, item_file);
      if (error < 0) {
        // TODO write to stderr on what happened
        return -1;
      }
      if (is_stdio_folder == 0) {
        int is_stdin =
            namecmp(file_path.path, "stdin", MIN(file_path.length, 5));
        if (is_stdin == 0) {
          error = open_existing_file(STDIN_FILENO, item_file, O_RDONLY, 0, 0);
          if (error != 0)
            return error;
        }
        int is_argv = namecmp(file_path.path, "argv", MIN(file_path.length, 4));
        if (is_argv == 0) {
          setup_charpparray(item_buffer->data, item_buffer->data_len, argc,
                            argv);
        }
        int is_environ =
            namecmp(file_path.path, "environ", MIN(file_path.length, 7));
        if (is_environ == 0) {
          int envc;
          setup_charpparray(item_buffer->data, item_buffer->data_len, &envc,
                            environ);
        }
      }
    }
  }

  // also add folders from stdout sets
  size_t output_sets = dandelion_output_set_count();
  for (size_t set_index = 0; set_index < output_sets; set_index++) {
    // get set information
    Path set_path = {.path = dandelion_output_set_ident(set_index),
                     .length = dandelion_output_set_ident_len(set_index)};
    if (set_path.length == 0)
      continue;
    // create directories for set
    D_File *set_directory = create_directories(fs_root, set_path, 1);
    if (set_directory == NULL) {
      // TODO write to stderr on what happened
      return -1;
    }
  }

  // if stdin has not been given as an input set create an empty file and
  // open it at index 0
  if (open_files[STDIN_FILENO].file == NULL) {
    Path stdin_path = path_from_string("stdin");
    D_File *stdin_file = create_file(&stdin_path, NULL, 0, S_IRUSR);
    if (stdin_file == NULL) {
      return -1;
    }
    // do not need to link it to directory, as it will not be part of input
    // or output set only open it to handle attempts to read from stdin
    error = open_existing_file(STDIN_FILENO, stdin_file, O_RDONLY, 0, 0);
    if (error != 0) {
      return error;
    }
    if (link_file_to_folder(stdio_folder, stdin_file) != 0) {
      return -1;
    }
  }

  // if argc or argv have not been initialized do it now
  if (*argv == NULL) {
    *argv = dandelion_alloc(sizeof(char *), _Alignof(char *));
    **argv = NULL;
  }
  if (*environ == NULL) {
    *environ = dandelion_alloc(sizeof(char *), _Alignof(char *));
    **environ = NULL;
  }
  return 0;
}

int add_output_from_file(D_File *file, Path previous_path, size_t set_index) {
  // need to have at least one statement (definition not statement) after a
  // label, so define this here so we can have a statement after the switch
  // labels.
  size_t name_length = 0;
  char *new_buffer = NULL;
  switch (file->type) {
  case FILE:
    // check name length an create string with complete file name
    name_length = namelen(file->name, FS_NAME_LENGHT);
    new_buffer = dandelion_alloc(previous_path.length + name_length, 1);
    if (new_buffer == NULL) {
      return -1;
    }
    memcpy(new_buffer, previous_path.path, previous_path.length);
    memcpy(new_buffer + previous_path.length, file->name, name_length);
    // check for content
    char *content_buf = NULL;
    size_t buff_len = 0;
    FileChunk *content = file->content;
    if (content != NULL) {
      // get file content into contiguous memory
      if (content->next == NULL) {
        content_buf = content->data;
        buff_len = content->used;
      } else {
        // find total size needed
        size_t total_size = 0;
        for (FileChunk *chunck = content; chunck != NULL;
             chunck = chunck->next) {
          total_size += chunck->used;
        }
        content_buf = dandelion_alloc(total_size, _Alignof(max_align_t));
        if (content_buf == NULL) {
          dandelion_exit(ENOMEM);
          return -1;
        }
        for (FileChunk *chunk = content; chunk != NULL; chunk = chunk->next) {
          memcpy(content_buf + buff_len, chunk->data, chunk->used);
          buff_len += chunk->used;
        }
      }
    }
    IoBuffer new_out = {.data = content_buf,
                        .data_len = buff_len,
                        .ident = new_buffer,
                        .ident_len = name_length + previous_path.length,
                        .key = 0};
    dandelion_add_output(set_index, new_out);
    return 0;
  case DIRECTORY:
    // check name length and create a new string / path to recurse further
    name_length = namelen(file->name, FS_NAME_LENGHT);
    new_buffer = dandelion_alloc(previous_path.length + name_length + 1, 1);
    if (new_buffer == NULL) {
      dandelion_exit(ENOMEM);
      return -1;
    }
    memcpy(new_buffer, previous_path.path, previous_path.length);
    memcpy(new_buffer + previous_path.length, file->name, name_length);
    new_buffer[previous_path.length + name_length] = '/';
    Path new_path = {
        .length = previous_path.length + name_length + 1,
        .path = new_buffer,
    };
    int error = 0;
    for (D_File *child = file->child; child != NULL; child = child->next) {
      if ((error = add_output_from_file(child, new_path, set_index)) != 0)
        return error;
    }
    dandelion_free(new_buffer);
    return 0;
  default:
    return -1;
  }
  return 0;
}

int fs_terminate() {
  // if fs has not been initialized properly, nothing we can do
  if (fs_root == NULL) {
    return 0;
  }
  // go through output set names and find all files in folders that are
  // named after them
  size_t output_sets = dandelion_output_set_count();
  for (size_t set_index = 0; set_index < output_sets; set_index++) {
    Path set_ident = {.path = dandelion_output_set_ident(set_index),
                      .length = dandelion_output_set_ident_len(set_index)};
    D_File *set_directory = find_file_in_dir(fs_root, set_ident);
    if (set_directory == NULL) {
      continue;
    }
    // create a output for each file in the directory
    Path empty_path = {.path = NULL, .length = 0};
    for (D_File *out_file = set_directory->child; out_file != NULL;
         out_file = out_file->next) {
      // ignore argv, environ and stdin in the stdio folder
      if (namecmp(set_ident.path, "stdio", MIN(set_ident.length, 5)) == 0) {
        if (namecmp(out_file->name, "environ", 7) == 0 ||
            namecmp(out_file->name, "argv", 4) == 0 ||
            namecmp(out_file->name, "stdin", 5) == 0)
          continue;
      }
      add_output_from_file(out_file, empty_path, set_index);
    }
  }
  return 0;
}
