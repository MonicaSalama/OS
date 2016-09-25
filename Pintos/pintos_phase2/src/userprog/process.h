#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/synch.h"
#include "threads/thread.h"
#include <user/syscall.h>

struct child_process {
  pid_t pid;
  struct list_elem elem;
  struct semaphore wait_load;
  struct semaphore waiting;
  int loaded;
  bool exited;
  bool wait;
  int exit_state;
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
struct child_process* add_child (int);
struct child_process* get_child_process (pid_t);
void remove_child_process (tid_t);
void arguments_push (void **, const char*, char**, int);
void remove_child_processes (void);


#endif /* userprog/process.h */
