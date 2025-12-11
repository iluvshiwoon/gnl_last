#include "get_next_line.h"
#include <stdio.h> // REMOVE

t_fd_list *get_fd_node(const int fd, t_fd_list **ref_head) {
  while (*ref_head) {
    if ((*ref_head)->fd == fd) {
      printf("%p: %d\n", (*ref_head), fd); // REMOVE
      return (*ref_head);
    }
    (*ref_head) = (*ref_head)->next;
  }
  *ref_head = malloc(sizeof(t_fd_list));
  if (*ref_head == NULL)
    return (*ref_head);
  **ref_head = (t_fd_list){};
  (*ref_head)->fd = fd;
  printf("%p: %d\n", (*ref_head), fd); // REMOVE
  return (*ref_head);
}

char *get_next_line(int fd) {
  char *result;
  static t_fd_list *head;
  t_fd_list *current;

  result = NULL;
  current = get_fd_node(fd, &head);
  if (current == NULL)
    return NULL; // Free all

  return result;
}
