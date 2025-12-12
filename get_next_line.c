#include "get_next_line.h"
#include <stddef.h>
#include <stdio.h> // REMOVE
#include <stdlib.h>
#include <string.h>

t_fd_list *get_fd_node(const int fd, t_fd_list **ref_head) {
  t_fd_list *cursor;

  cursor = *ref_head;
  while (cursor) {
    if (cursor->fd == fd) {
      return (cursor);
    }
    cursor = cursor->next;
  }
  cursor = malloc(sizeof(t_fd_list));
  if (cursor == NULL)
    return NULL;
  *cursor = (t_fd_list){};
  cursor->fd = fd;
  cursor->next = *ref_head;
  *ref_head = cursor;
  return cursor;
}

void *ft_memcpy(void *dst, const void *src, size_t n) {
  unsigned char *d;
  const unsigned char *s;

  d = dst;
  s = src;
  while (n--)
    *d++ = *s++;
  return dst;
}

char *get_nl_addr(char *start, size_t n) {
  while (n--) {
    if (*start == '\n')
      return start;
    start++;
  }
  return NULL;
}

// t_return check_stash(char **r_value, t_fd_list *node) {
//   char *nl_addr;
//   char *remainder;
//   size_t remainder_len;
//
//   if (node->stash_len == 0)
//     return SUCCESS;
//   nl_addr = get_nl_addr(node->stash, node->stash_len);
//   if (nl_addr == NULL)
//     return SUCCESS;
//   remainder_len = node->stash_len - (nl_addr - node->stash + 1);
//   remainder = NULL;
//   if (remainder_len > 0) {
//     remainder = malloc(
//         sizeof(char) *
//         (remainder_len + 1)); // +1 so if it fits exactly a line + \n we can
//     if (remainder == NULL)
//       return ERROR;
//     ft_memcpy(remainder, nl_addr + 1, remainder_len);
//   }
//   *r_value = node->stash;
//   (*r_value)[nl_addr - *r_value + 1] = '\0';
//   node->stash = remainder;
//   node->stash_len = remainder_len;
//   return SUCCESS;
// }
//
t_return check_stash(char **r_value, t_fd_list *node) {
  char *nl_addr;
  char *line; // New variable for the exact-fit line
  char *remainder;
  size_t remainder_len;
  size_t line_len; // New variable to track line size

  if (node->stash_len == 0)
    return SUCCESS;
  nl_addr = get_nl_addr(node->stash, node->stash_len);
  if (nl_addr == NULL)
    return SUCCESS;

  // 1. Calculate precise lengths
  line_len = nl_addr - node->stash + 1;
  remainder_len = node->stash_len - line_len;

  // 2. Allocate the line (EXACT FIT)
  line = malloc(sizeof(char) * (line_len + 1));
  if (line == NULL)
    return ERROR;
  ft_memcpy(line, node->stash, line_len);
  line[line_len] = '\0';

  // 3. Handle the remainder
  remainder = NULL;
  if (remainder_len > 0) {
    remainder = malloc(sizeof(char) * (remainder_len + 1));
    if (remainder == NULL) {
      free(line); // <--- IMPORTANT: Clean up if second malloc fails
      return ERROR;
    }
    ft_memcpy(remainder, nl_addr + 1, remainder_len);
    // No null terminator needed for stash as per your logic,
    // but safe to add if you want: remainder[remainder_len] = '\0';
  }

  // 4. Swap and Free
  free(node->stash); // Destroy the oversized old stash
  node->stash = remainder;
  node->stash_len = remainder_len;
  *r_value = line; // Return the perfectly sized line

  return SUCCESS;
}
t_return ensure_capacity(char **buffer, size_t *capacity, size_t used_len,
                         size_t needed_size) {
  char *new_buffer;
  size_t new_capacity;

  if (*capacity >= needed_size)
    return SUCCESS;
  new_capacity = *capacity;
  if (new_capacity == 0)
    new_capacity = needed_size;
  while (new_capacity < needed_size)
    new_capacity *= 2;
  new_buffer = malloc((sizeof(char) * (new_capacity + 1)));
  if (new_buffer == NULL)
    return ERROR;
  if (*buffer != NULL && used_len > 0)
    ft_memcpy(new_buffer, *buffer, used_len);
  free(*buffer);
  *buffer = new_buffer;
  *capacity = new_capacity;
  return SUCCESS;
}

t_return copy_to_stash(char *buffer, t_fd_list *node, char *nl_addr,
                       size_t len) {
  size_t remainder;

  remainder = len - (nl_addr - buffer + 1);
  if (remainder == 0)
    return SUCCESS;
  node->stash = malloc(sizeof(char) * (remainder + 1));
  if (node->stash == NULL)
    return ERROR;
  node->stash_len = remainder;
  ft_memcpy(node->stash, nl_addr + 1, remainder);
  return SUCCESS;
}

t_return read_line(char **buffer, t_fd_list *node) {
  size_t capacity;
  size_t len;
  ssize_t bytes_read;
  char *nl_addr;

  nl_addr = NULL;
  *buffer = node->stash;
  node->stash = NULL;
  capacity = node->stash_len;
  len = node->stash_len;
  node->stash_len = 0;
  while (1) {
    if (ensure_capacity(buffer, &capacity, len, len + BUFFER_SIZE) == ERROR)
      return (free(*buffer), ERROR);
    bytes_read = read(node->fd, *buffer + len, BUFFER_SIZE);
    if (bytes_read < 0)
      return (free(*buffer), ERROR);
    if (bytes_read == 0 && len > 0)
      break;
    if (bytes_read == 0)
      return (free(*buffer), EOF);
    nl_addr = get_nl_addr((*buffer) + len, bytes_read);
    len += bytes_read;
    if (nl_addr != NULL)
      break;
  }
  // if (nl_addr != NULL) {
  //   if (copy_to_stash(*buffer, node, nl_addr, len) == ERROR)
  //     return ERROR;
  //   (*buffer)[nl_addr - *buffer + 1] = '\0';
  // } else
  //   (*buffer)[len] = '\0';
  // return SUCCESS;
  size_t final_len;
  char *exact_line;

  if (nl_addr != NULL) {
    if (copy_to_stash(*buffer, node, nl_addr, len) == ERROR)
      return ERROR;
    final_len = nl_addr - *buffer + 1;
  } else {
    final_len = len;
  }

  // Allocate EXACT size
  exact_line = malloc(sizeof(char) * (final_len + 1));
  if (exact_line == NULL)
    return (free(*buffer), ERROR);

  // Copy and Null-terminate
  ft_memcpy(exact_line, *buffer, final_len);
  exact_line[final_len] = '\0';

  // Free the big buffer and replace it
  free(*buffer);
  *buffer = exact_line;

  return SUCCESS;
}

void free_gnl(t_fd_list **head, int fd) {
  t_fd_list *to_free;
  t_fd_list **cursor;

  cursor = head;
  while (*cursor) {
    if ((*cursor)->fd == fd) {
      to_free = *cursor;
      *cursor = (*cursor)->next;
      free(to_free->stash);
      free(to_free);
      return;
    }
    cursor = &(*cursor)->next;
  }
}

char *get_next_line(int fd) {
  char *r_value;
  static t_fd_list *head;
  t_fd_list *node;
  t_return status;

  r_value = NULL;
  node = get_fd_node(fd, &head);
  if (node == NULL)
    return NULL;
  if (check_stash(&r_value, node) == ERROR)
    return (free_gnl(&head, fd), NULL);
  if (r_value != NULL) // found line in stash;
    return r_value;
  status = read_line(&r_value, node);
  if (status == ERROR || (int)status == EOF)
    return (free_gnl(&head, fd), NULL);
  return r_value;
}
