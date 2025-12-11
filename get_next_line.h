#ifndef GET_NEXT_LINE_H
#define GET_NEXT_LINE_H
#define BUFFER_SIZE 10 // Change to 4096
#include <stdlib.h>
#include <unistd.h>

typedef struct s_fd_list {
  int fd;
  char *stash;
  size_t stash_len;
  struct s_fd_list *next;
} t_fd_list;

char *get_next_line(int fd);

#endif
