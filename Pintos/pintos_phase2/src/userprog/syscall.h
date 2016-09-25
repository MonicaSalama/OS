#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>

void syscall_init (void);
void valid_ptr(const void *);
void memory_read (void*, void*,size_t);
struct file_desc* get_file_desc (int);

struct file_desc {
  int id;
  struct list_elem elem;
  struct file* file;
};

#endif /* userprog/syscall.h */
