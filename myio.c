/*
 * myio.c
 *
 * authors: Chris Fetterolf, Kyle Schlanger
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define VERBOSE TRUE
#define BUFFER_SIZE 1000

struct Buffer {
  int size, offset;
  char *buf;
};

/* function declarations */
void usage(char *argv[]);
int myopen(const char *pathname, int flags, struct Buffer *my_buf) ;
int myclose(int fd, struct Buffer *my_buf);
ssize_t mywrite(int fd, const char *buf, size_t count, struct Buffer *my_buf);
ssize_t myread(int fd, char *buf, size_t count, struct Buffer *my_buf);
ssize_t myflush(int fd, struct Buffer *my_buf);


int main(int argc, char *argv[]) {

  char *buf, *my_buf_pointer, *src_filename, *dst_filename;
  int src_fd, dst_fd;

  /* check command line args */
  if (argc < 3) {
    usage(argv);
    exit(1);
  }
  src_filename = argv[1];
  dst_filename = argv[2];

  struct Buffer *my_buf;

  /* open source and dest file */
  // src_fd = myopen(src_filename, O_RDONLY);
  // if (src_fd < 0) {
  //   perror("open");
  //   exit(2);
  // }
  // dst_fd = myopen(dst_filename, O_CREAT | O_WRONLY);
  // if (dst_fd < 0) {
  //   perror("open");
  //   exit(3);
  // }


  return 0;
}


/* -------------------------------- */

void usage(char *argv[]) {
    printf("usage: %s src_filename dst_filename buffer_size\n", argv[0]);
}

int myopen(const char *pathname, int flags, struct Buffer *my_buf) {
  /* allocate space for our buffer */
  my_buf->size = BUFFER_SIZE;
  my_buf->buf = (char *) malloc(BUFFER_SIZE);
  my_buf->offset = 0;

  /* open the file, pass flags */
  int src_fd = open(pathname, flags);
  return src_fd;
}

int myclose(int fd, struct Buffer *my_buf) {
  /* free allocated memory and close file */
  free((char *) my_buf->buf);
  return close(fd);
}

ssize_t myread(int fd, char *buf, size_t count, struct Buffer *my_buf) {
  int n, filler;

  /* Nothing in our buffer, so read in BUFFER_SIZE from fd */
  if (my_buf->offset == 0) {
    n = read(fd, my_buf->buf, BUFFER_SIZE);
    memcpy(buf, my_buf->buf, count);
    buf += count; // spoof their pointer
    my_buf->offset += count;
  }

  /* They request more to read than we have stored, so memcpy and read in another BUFFER_SIZE bytes */
  else if ((my_buf->offset + count) > my_buf->size) {
    filler = my_buf->size - my_buf->offset;
    memcpy(buf, my_buf->buf + my_buf->offset, filler);
    my_buf->offset = 0;
    buf += filler;

    /* fill our buffer BUFFER_SIZE then transfer the leftover bytes */
    int leftover = count - filler;
    n = myread(fd, buf, leftover, my_buf);
  }

  /* They request what we have already read in, so just copy to their buf */
  else {
    memcpy(buf, my_buf->buf + my_buf->offset, count);
    buf += count; // spoof their pointer
    my_buf->offset += count;
    n = count;
  }
  return n;
}

ssize_t mywrite(int fd, const char *buf, size_t count, struct Buffer *my_buf) {
  int n, filler; // return value

  /* if we hit the end of our buffer */
  if ((my_buf->offset + count) > my_buf->size) {

    /* copy until we fill buffer, then flush */
    filler = my_buf->size - my_buf->offset;
    memcpy(my_buf->buf + my_buf->offset, buf, filler);
    buf += filler;
    my_buf->offset = my_buf->size;
    n = myflush(fd, my_buf); // sets offset to 0

    /* fill our empty buffer with remaining bytes in buf, increment offset */
    memcpy(my_buf->buf, buf, count-filler);
    my_buf->offset += (count-filler);
  }

  /* otherwise fake "write" to our buffer */
  else {
    memcpy(my_buf->buf + my_buf->offset, buf, count);
    my_buf->offset += count;
    buf += count;
    n = count;
  }
  return n;
}

ssize_t myflush(int fd, struct Buffer *my_buf) {
  /* write from our buffer to fd */
  int n = write(fd, my_buf->buf, my_buf->offset);
  my_buf->offset = 0;
  return n;
}
