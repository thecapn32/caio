// Copyright 2023 Vahid Mardani
/*
 * This file is part of caio.
 *  caio is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  caio is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with caio. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
#ifndef CAIO_H_
#define CAIO_H_


#include <errno.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include <clog.h>


/* Generic stuff */
#define CAIO_NAME_PASTER(x, y) x ## _ ## y
#define CAIO_NAME_EVALUATOR(x, y)  CAIO_NAME_PASTER(x, y)
#define CAIO_NAME(n) CAIO_NAME_EVALUATOR(CAIO_ENTITY, n)


#define ASYNC void


#define CORO_START \
    switch ((self)->current->line) { \
        case 0:


#define CORO_FINALLY \
        case -1:; } \
    (self)->status = CAIO_TERMINATED


#define CORO_WAIT(coro, state) \
    do { \
        (self)->current->line = __LINE__; \
        if (caio_call_new(self, (caio_coro)coro, (void *)state)) { \
            (self)->status = CAIO_TERMINATING; \
        } \
        return; \
        case __LINE__:; \
    } while (0)


#define CORO_REJECT(fmt, ...) \
    if (fmt) { \
        ERROR(fmt, ## __VA_ARGS__); \
    } \
    (self)->status = CAIO_TERMINATING; \
    return


#define CORO_RETURN \
    (self)->status = CAIO_TERMINATING; \
    return


#define CORO_WAITFD(fd, events) \
    do { \
        (self)->current->line = __LINE__; \
        if (caio_evloop_register(self, fd, events)) { \
            (self)->status = CAIO_TERMINATING; \
        } \
        else { \
            (self)->status = CAIO_WAITINGIO; \
        } \
        return; \
        case __LINE__:; \
    } while (0)


#define CORO_MUSTWAITFD() \
    ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINPROGRESS))


#define CAIO_SPAWN(coro, state) caio_spawn((caio_coro)(coro), (void*)(state))
#define CAIO_FOREVER(coro, state, maxtasks) \
    caio_forever((caio_coro)(coro), (void*)(state), maxtasks)


enum caio_flags {
    CAIO_NONE = 0,
    CAIO_SIG = 1,
};


enum caio_taskstatus {
    CAIO_RUNNING,
    CAIO_WAITINGIO,
    CAIO_TERMINATING,
    CAIO_TERMINATED,
};


enum caio_fdflags {
    CAIO_IN = EPOLLIN,
    CAIO_OUT = EPOLLOUT,
    CAIO_RDHUP = EPOLLRDHUP,
    CAIO_PRI = EPOLLPRI,
    CAIO_ERR = EPOLLERR,
    CAIO_HUP = EPOLLHUP,
    CAIO_ET = EPOLLET,
    CAIO_ONESHOT = EPOLLONESHOT,
    CAIO_WAKEUP = EPOLLWAKEUP,
    CAIO_EXCLUSIVE = EPOLLEXCLUSIVE,
};


struct caio_task;
typedef void (*caio_coro) (struct caio_task *self, void *state);
typedef void (*caio_invoker) (struct caio_task *self);


struct caio_call {
    struct caio_call *parent;
    int line;
    caio_coro coro;
    void *state;
    caio_invoker invoke;
};


struct caio_task {
    int index;
    enum caio_taskstatus status;
    struct caio_call *current;
};


struct caio_taskpool {
    struct caio_task **pool;
    size_t size;
    size_t count;
};


int
caio_forever(caio_coro coro, void *state, size_t maxtasks);


int
caio_start();


int
caio_init(size_t maxtasks, int flags);


void
caio_deinit();


int
caio_loop();


struct caio_task *
caio_task_new();


int
caio_call_new(struct caio_task *task, caio_coro coro, void *state);


void
caio_task_dispose(struct caio_task *task);


void
caio_task_killall();


int
caio_evloop_register(struct caio_task *task, int fd, int events);


int
caio_evloop_unregister(int fd);


int
caio_spawn(caio_coro coro, void *state);


#endif  // CAIO_H_
