#ifndef MY_TASKS_H
#define MY_TASKS_H
#include "allocator.h"
#include "fptr.h"
#include <assert.h>
#include <time.h>
typedef struct timespec timespec;
[[gnu::pure]]
static inline timespec floatTspec(double seconds) {
  seconds = seconds < 0 ? -1 * seconds : seconds;
  umax iseconds = (umax)(seconds * 1000000000);
  timespec res;
  res.tv_sec = (typeof(res.tv_sec))(iseconds / 1000000000);
  res.tv_nsec = (typeof(res.tv_nsec))(iseconds % 1000000000);
  return res;
}
#include <threads.h>
typedef struct {
  thrd_t handle;
  thrd_start_t function;
  uchar block : 1;
  uchar fin : 1;
  size_t alen;
  [[clang::counted_by(alen)]] void *arg[];
} task;
#define tnew_ref_helper(var) &(var)
#define task_new_s(funct, blocking, arglength)                             \
  ({                                                                       \
    task *res = (task *)alloca(sizeof(task) + sizeof(void *) * arglength); \
    res->alen = arglength;                                                 \
    res->block = blocking;                                                 \
    res->function = funct;                                                 \
    res->fin = 0;                                                          \
    res;                                                                   \
  })
static task *task_new_h(
    const My_allocator *allocator,
    thrd_start_t funct,
    uchar blocking,
    size_t arglength
) {
  task *res =
      (task *)aAlloc(allocator, sizeof(task) + sizeof(void *) * arglength);
  res->alen = arglength;
  res->block = blocking;
  res->function = funct;
  res->fin = 0;
  return res;
}
#define task_setArgs(task, ...) \
  memcpy(task->arg, (void *[]){APPLY_N(tnew_ref_helper, __VA_ARGS__)}, task->alen * sizeof(void *));

static int thread_handle(void *arg) {
  task *t = (task *)arg;
  int res = t->function(arg);
  t->fin = 1;
  return res;
}
static int task_start(task t[]) {
  int res = 0;
  switch (t->block) {
  case 1:
    thrd_create(&t->handle, t->function, t);
    return thrd_join(t->handle, &res);
  default:
    thrd_create(&t->handle, thread_handle, t);
    thrd_detach(t->handle);
    break;
  }
  return res;
}
static void task_await(task t[], const timespec *interval) {
  assert(!t->block);
  while (!t->fin)
    thrd_sleep(interval, NULL);
}
#endif // MY_TASKS_H
