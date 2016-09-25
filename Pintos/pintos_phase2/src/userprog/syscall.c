#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

#define ERROR -1
struct lock filesys_lock;

static void syscall_handler (struct intr_frame *);

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  if(!is_user_vaddr(uaddr))
    return -1;
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  if(!is_user_vaddr(udst))
    return false;
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}


static bool is_valid_pointer(void * esp, uint8_t argc){
  uint8_t i = 0;
  for (; i < argc; ++i)
  {
    if (get_user(((uint8_t *)esp)+i) == -1){
      return false;
    }
  }
  return true;
}

static bool is_valid_string(void * str)
{
  int ch=-1;
  while((ch=get_user((uint8_t*)str++))!='\0' && ch!=-1);
    if(ch=='\0')
      return true;
    else
      return false;
}

// void valid_ptr (const void *ptr) {
//   if(!is_user_vaddr(ptr)) {
//     exit(ERROR);
//   }
// }
//
// void memory_read (void *src , void *dst, size_t bytes) {
//   int* temp = (int *)src; int i = 0;
//   while (i < bytes) {
//     valid_ptr((const void *)temp);
//     temp = temp - 1;
//     i++;
//   }
//   memcpy(src, dst, bytes);
// }

void
syscall_init (void)
{
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  if (!is_valid_pointer(f->esp, 4)){
    exit(ERROR);
    return;
  }
  int syscall_number = * (int *)f->esp;
  // memory_read(f->esp, &syscall_number, sizeof(syscall_number));
  switch (syscall_number) {
    case SYS_HALT:                   /* Halt the operating system. */
    {
      halt();
      break;
    }
    case SYS_EXIT:                   /* Terminate this process. */
    {
      int status;
      if (is_valid_pointer(f->esp + 4, 4))
        status = *((int*)f->esp+1);
      else
        exit(ERROR);
      // memory_read(f->esp + 4, &status, sizeof(status));
      exit(status);
      break;
    }
    case SYS_EXEC:                   /* Start another process. */
    {
      if (!is_valid_pointer(f->esp +4, 4) ||
        !is_valid_string(*(char **)(f->esp + 4))) {
          exit(ERROR);
      }
      char *cmd_line = *(char **)(f->esp + 4);
      // memory_read(f->esp + 4, &cmd_line, sizeof(cmd_line));
      f->eax = exec(cmd_line);
      break;
    }
    case SYS_WAIT:                   /* Wait for a child process to die. */
    {
      pid_t pid;
      if (is_valid_pointer(f->esp + 4, 4))
        pid = *((int*)f->esp+1);
      else
        exit(ERROR);
      // memory_read(f->esp + 4, &pid, sizeof(pid));
      f->eax = wait(pid);
      break;
    }
    case SYS_CREATE:                 /* Create a file. */
    {
      if (
        !is_valid_pointer(f->esp + 4, 4) ||
        !is_valid_string(*(char **)(f->esp + 4)) ||
        !is_valid_pointer(f->esp + 8, 4)){
        exit(ERROR);
      }
      char *filename = *(char **)(f->esp + 4);
      unsigned initial_size = *(int *)(f->esp + 8);
      // char* filename;
      // unsigned initial_size;
      // memory_read(f->esp + 4, &filename, sizeof(filename));
      // memory_read(f->esp + 8, &initial_size, sizeof(initial_size));
      f->eax = create(filename, initial_size);
      break;
    }
    case SYS_REMOVE:                 /* Delete a file. */
    {
      if (!is_valid_pointer(f->esp +4, 4) || !is_valid_string(*(char **)(f->esp + 4))){
        exit(ERROR);
      }
      char *filename = *(char **)(f->esp + 4);
      // char* filename;
      // memory_read(f->esp + 4, &filename, sizeof(filename));
      f->eax = remove(filename);
      break;
    }
    case SYS_OPEN:                   /* Open a file. */
    {
      if (!is_valid_pointer(f->esp +4, 4) || !is_valid_string(*(char **)(f->esp + 4))){
        exit(ERROR);
      }
      char *filename = *(char **)(f->esp + 4);
      // const char* filename;
      // memory_read(f->esp + 4, &filename, sizeof(filename));
      f->eax = open(filename);
      break;
    }
    case SYS_FILESIZE:               /* Obtain a file's size. */
    {
      int fd;
      if (is_valid_pointer(f->esp + 4, 4))
        fd = *((int*)f->esp+1);
      else
        exit(ERROR);
      // memory_read(f->esp + 4, &fd, sizeof(fd));
      f->eax = filesize(fd);
      break;
    }
    case SYS_READ:                   /* Read from a file. */
    {
      if (!is_valid_pointer(f->esp + 4, 12)){
        exit(ERROR);
      }
      int fd = *(int *)(f->esp + 4);
      void *buffer = *(char**)(f->esp + 8);
      unsigned size = *(unsigned *)(f->esp + 12);
      if (!is_valid_pointer(buffer, 1) || !is_valid_pointer(buffer + size,1)){
        exit(ERROR);
      }
      // int fd;
      // void *buffer;
      // unsigned size;
      //
      // memory_read(f->esp + 4, &fd, sizeof(fd));
      // memory_read(f->esp + 8, &buffer, sizeof(buffer));
      // memory_read(f->esp + 12, &size, sizeof(size));

      f->eax = read(fd, buffer, size);
      break;
    }
    case SYS_WRITE:                  /* Write to a file. */
    {
      if (!is_valid_pointer(f->esp + 4, 12)){
        exit(ERROR);
      }
      int fd = *(int *)(f->esp + 4);
      void *buffer = *(char**)(f->esp + 8);
      unsigned size = *(unsigned *)(f->esp + 12);
      if (!is_valid_pointer(buffer, 1) || !is_valid_pointer(buffer + size,1)){
        exit(ERROR);
      }
      // int fd;
      // void *buffer;
      // unsigned size;
      //
      // memory_read(f->esp + 4, &fd, sizeof(fd));
      // memory_read(f->esp + 8, &buffer, sizeof(buffer));
      // memory_read(f->esp + 12, &size, sizeof(size));

      f->eax = write(fd, buffer, size);
      break;
    }
    case SYS_SEEK:                   /* Change position in a file. */
    {
      if (!is_valid_pointer(f->esp +4, 8)){
        exit(ERROR);
      }
      int fd = *(int *)(f->esp + 4);
      unsigned position = *(unsigned *)(f->esp + 8);
      // int fd;
      // unsigned position;
      //
      // memory_read(f->esp + 4, &fd, sizeof(fd));
      // memory_read(f->esp + 8, &position, sizeof(position));

      seek(fd, position);
      break;
    }
    case SYS_TELL:                   /* Report current position in a file. */
    {
      int fd;
      if (is_valid_pointer(f->esp + 4, 4))
        fd = *((int*)f->esp+1);
      else
        exit(ERROR);
      // memory_read(f->esp + 4, &fd, sizeof(fd));
      f->eax = tell(fd);
      break;
    }
    case SYS_CLOSE:                  /* Close a file. */
    {
      int fd;
      if (is_valid_pointer(f->esp + 4, 4))
        fd = *((int*)f->esp+1);
      else
        exit(ERROR);
      // memory_read(f->esp + 4, &fd, sizeof(fd));
      close(fd);
      break;
    }
    default:
    {
      exit(ERROR);
    }
  }
}

/**
* Takes a file id ,and returns it's descriptor. Returns NULL otherwise.
*/
struct file_desc* get_file_desc (int fd) {
  struct thread *t = thread_current();
  ASSERT (t != NULL);
  struct list_elem *e;

  if (fd < 3) {
    return NULL;
  }

  for(e = list_begin(&t->file_descriptors);
      e != list_end(&t->file_descriptors); e = list_next(e))
  {
     struct file_desc *desc = list_entry(e, struct file_desc, elem);
     if (desc->id == fd) {
       return desc;
    }
  }

  return NULL;
}

void halt () {
  shutdown_power_off();
}

void exit (int status) {
  struct thread* current = thread_current();
  if(current->parent_child_list != NULL){
    current->parent_child_list->exit_state = status;
  }
  // printf ("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}

pid_t exec (const char * cmd_line) {
  pid_t child_tid = process_execute(cmd_line);
  struct child_process* cp = get_child_process(child_tid);
  if (cp != NULL && cp->loaded == 0) {
    sema_down(&cp->wait_load);
  }
  if (cp != NULL && cp->loaded == -1)
      return ERROR;
  return child_tid;
}

int wait (pid_t pid ) {
  return process_wait(pid);
}

bool create (const char * file , unsigned initial_size) {
  lock_acquire(&filesys_lock);
  bool success = filesys_create(file, initial_size);
  lock_release(&filesys_lock);
  return success;
}

bool remove (const char * file) {
  lock_acquire(&filesys_lock);
  bool success = filesys_remove(file);
  lock_release(&filesys_lock);
  return success;
}

int open (const char * file) {
  lock_acquire(&filesys_lock);
  struct file* opened_file;
  struct file_desc* fd = palloc_get_page(0);

  opened_file = filesys_open(file);
  if (!opened_file) {
    lock_release(&filesys_lock);
    return -1;
  }

  fd->file = opened_file;

  struct list* fd_list = &thread_current()->file_descriptors;
  // 0, 1, 2 are reserved for STDIN, STDOUT, STDERR (from the manual).
  if (list_empty(fd_list)) {
    fd->id = 3;
  } else {
    fd->id = (list_entry(list_back(fd_list), struct file_desc, elem)->id) + 1;
  }
  list_push_back(fd_list, &(fd->elem));
  lock_release(&filesys_lock);
  return fd->id;
}

int filesize (int fd) {
  struct file_desc* descriptor = get_file_desc(fd);

  if (descriptor == NULL) {
    return -1;
  }

  lock_acquire(&filesys_lock);
  int size =  file_length(descriptor->file);
  lock_release(&filesys_lock);
  return size;
}

int read (int fd , void * buffer , unsigned size) {
  if (fd == STDIN_FILENO) {
    unsigned i;
    for (i = 0; i < size; i++) {
      ((uint8_t *) buffer)[i] = input_getc();
    }
    return size;
  }

  struct file_desc* descriptor = get_file_desc(fd);

  if (descriptor == NULL) {
    return -1;
  }

  lock_acquire(&filesys_lock);
  int bytes = file_read(descriptor->file, buffer, size);
  lock_release(&filesys_lock);
  return bytes;
}

int write (int fd , const void * buffer , unsigned size) {
  if (fd == STDOUT_FILENO) {
    putbuf(buffer, size);
    return size;
  }

  struct file_desc* descriptor = get_file_desc(fd);

  if (descriptor == NULL) {
    return -1;
  }

  lock_acquire(&filesys_lock);
  int bytes = file_write (descriptor->file, buffer, size);
  lock_release(&filesys_lock);
  return bytes;
}

void seek (int fd , unsigned position) {
  struct file_desc* descriptor = get_file_desc(fd);

  if (descriptor == NULL) {
    return;
  }

  lock_acquire(&filesys_lock);
  file_seek(descriptor->file, position);
  lock_release(&filesys_lock);
}

unsigned tell (int fd) {
  struct file_desc* descriptor = get_file_desc(fd);

  if (descriptor == NULL) {
    return -1;
  }

  lock_acquire(&filesys_lock);
  off_t pos = file_tell(descriptor->file);
  lock_release(&filesys_lock);
  return pos;
}

void close (int fd) {
  struct file_desc* file_d = get_file_desc(fd);

  if(file_d == NULL || file_d->file == NULL) {
    return;
  }

  lock_acquire(&filesys_lock);
  file_close(file_d->file);
  list_remove(&(file_d->elem));
  palloc_free_page(file_d);
  lock_release(&filesys_lock);
}
