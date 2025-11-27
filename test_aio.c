#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>

#define SYS_io_setup 206
#define SYS_io_destroy 207
#define SYS_io_getevents 208
#define SYS_io_submit 209
#define SYS_io_cancel 210
struct iocb {
  uint64_t aio_data;        // +0
  uint32_t aio_key;         // +8
  uint32_t aio_reserved1;   // +12  
  uint16_t aio_lio_opcode;  // +16
  int16_t  aio_reqprio;     // +18
  uint32_t aio_fildes;      // +20
  uint64_t aio_buf;         // +24
  uint64_t aio_nbytes;      // +32
  int64_t  aio_offset;      // +40
  uint64_t aio_reserved2;   // +48
  uint64_t aio_reserved3;   // +56
};

struct io_event {
  uint64_t data;
  uint64_t obj;
  int64_t  res;
  int64_t  res2;
};

static int x_io_setup(unsigned nr, uint64_t *ctxp) { return syscall(SYS_io_setup, nr, ctxp); }
static int x_io_destroy(uint64_t ctx) { return syscall(SYS_io_destroy, ctx); }
static int x_io_submit(uint64_t ctx, long nr, struct iocb **iocbpp) { return syscall(SYS_io_submit, ctx, nr, iocbpp); }
static int x_io_getevents(uint64_t ctx, long min_nr, long nr, struct io_event *events, struct timespec *timeout) { return syscall(SYS_io_getevents, ctx, min_nr, nr, events, timeout); }

static void fill(uint8_t *buf, size_t n, uint8_t seed) { for (size_t i = 0; i < n; i++) buf[i] = (uint8_t)(seed + i); }
static int verify(uint8_t *buf, size_t n, uint8_t seed) { for (size_t i = 0; i < n; i++) if (buf[i] != (uint8_t)(seed + i)) return -1; return 0; }

int main(int argc, char **argv) {
  const char *path = (argc > 1) ? argv[1] : "/ext2/tmp/aio.bin";
  int fd = open(path, O_CREAT | O_RDWR, 0644);
  if (fd < 0) { perror("open"); return 1; }

  const size_t N = 1 << 20;
  uint8_t *wbuf1 = malloc(N), *wbuf2 = malloc(N);
  uint8_t *rbuf1 = malloc(N), *rbuf2 = malloc(N);
  if (!wbuf1 || !wbuf2 || !rbuf1 || !rbuf2) { fprintf(stderr, "malloc\n"); return 1; }
  fill(wbuf1, N, 0x10); fill(wbuf2, N, 0x20); memset(rbuf1, 0, N); memset(rbuf2, 0, N);

  uint64_t ctx = 0;
  if (x_io_setup(4, &ctx) < 0) { perror("io_setup"); return 1; }

  struct iocb wr1 = {0}, wr2 = {0}, rd1 = {0}, rd2 = {0};
  wr1.aio_data = 1; wr1.aio_lio_opcode = 1; wr1.aio_fildes = (uint32_t)fd; wr1.aio_buf = (uint64_t)(uintptr_t)wbuf1; wr1.aio_nbytes = N; wr1.aio_offset = 0;
  wr2.aio_data = 2; wr2.aio_lio_opcode = 1; wr2.aio_fildes = (uint32_t)fd; wr2.aio_buf = (uint64_t)(uintptr_t)wbuf2; wr2.aio_nbytes = N; wr2.aio_offset = (int64_t)N;
  rd1.aio_data = 3; rd1.aio_lio_opcode = 0; rd1.aio_fildes = (uint32_t)fd; rd1.aio_buf = (uint64_t)(uintptr_t)rbuf1; rd1.aio_nbytes = N; rd1.aio_offset = 0;
  rd2.aio_data = 4; rd2.aio_lio_opcode = 0; rd2.aio_fildes = (uint32_t)fd; rd2.aio_buf = (uint64_t)(uintptr_t)rbuf2; rd2.aio_nbytes = N; rd2.aio_offset = (int64_t)N;

  struct iocb *subs[4] = { &wr1, &wr2, &rd1, &rd2 };
  int subm = x_io_submit(ctx, 4, subs);
  if (subm < 0) { perror("io_submit"); x_io_destroy(ctx); return 1; }

  struct io_event evs[4] = {0};
  struct timespec ts = { .tv_sec = 5, .tv_nsec = 0 };
  int got = x_io_getevents(ctx, 4, 4, evs, &ts);
  if (got < 0) { perror("io_getevents"); x_io_destroy(ctx); return 1; }

  int ok_w = 0, ok_r = 0;
  for (int i = 0; i < got; i++) {
    if (evs[i].res < 0) { fprintf(stderr, "event data=%llu err=%lld\n", (unsigned long long)evs[i].data, (long long)evs[i].res); }
    else {
      if (evs[i].data == 1 || evs[i].data == 2) ok_w++;
      if (evs[i].data == 3 || evs[i].data == 4) ok_r++;
    }
  }

  int v1 = verify(rbuf1, N, 0x10);
  int v2 = verify(rbuf2, N, 0x20);
  printf("submitted=%d got=%d write_ok=%d read_ok=%d verify=[%d,%d]\n", subm, got, ok_w, ok_r, v1 == 0, v2 == 0);

  x_io_destroy(ctx);
  close(fd);
  return (ok_w == 2 && ok_r == 2 && v1 == 0 && v2 == 0) ? 0 : 2;
}