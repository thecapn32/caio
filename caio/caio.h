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
#ifndef CAIO_CAIO_H_
#define CAIO_CAIO_H_


enum caio_taskstatus {
    CAIO_IDLE = 1,
    CAIO_RUNNING = 2,
    CAIO_WAITING = 4,
    CAIO_TERMINATING = 8,
    CAIO_TERMINATED = 16,
};


typedef struct caio *caio_t;
struct caio_task;
typedef void (*caio_invoker) (struct caio_task *self);


struct caio_basecall {
    struct caio_basecall *parent;
    int line;
    caio_invoker invoke;
};


struct caio_task {
    struct caio* caio;
    struct caio_basecall *current;
    enum caio_taskstatus status;
    int eno;
};


/* Modules */
struct caio_module;
typedef void (*caio_hook) (struct caio_module *m, caio_t c);
struct caio_module {
    caio_hook loopstart;
    caio_hook tick;
    caio_hook loopend;
};


caio_t
caio_create(size_t maxtasks);


int
caio_destroy(caio_t c);


struct caio_task *
caio_task_new(caio_t c);


int
caio_task_dispose(struct caio_task *task);


void
caio_task_killall(caio_t c);


int
caio_loop(caio_t c);


/* Generic stuff */
#define CAIO_NAME_PASTER(x, y) x ## _ ## y
#define CAIO_NAME_EVALUATOR(x, y)  CAIO_NAME_PASTER(x, y)
#define CAIO_NAME(n) CAIO_NAME_EVALUATOR(CAIO_ENTITY, n)


#define ASYNC void
#define CAIO_AWAIT(task, entity, coro, ...) \
    do { \
        (task)->current->line = __LINE__; \
        if (entity ## _call_new(task, coro, __VA_ARGS__)) { \
            (task)->status = CAIO_TERMINATING; \
        } \
        return; \
        case __LINE__:; \
    } while (0)


#define CAIO_BEGIN(task) \
    switch ((task)->current->line) { \
        case 0:


#define CAIO_FINALLY(task) \
        case -1:; } \
    (task)->status = CAIO_TERMINATED


#define CAIO_RETURN(task) \
    (task)->eno = 0; \
    (task)->status = CAIO_TERMINATING; \
    return


#define CAIO_THROW(task, n) \
    (task)->eno = n; \
    (task)->status = CAIO_TERMINATING; \
    return


#define CAIO_RETHROW(task) \
    (task)->status = CAIO_TERMINATING; \
    return


#define CAIO_HASERROR(task) (task->eno != 0)
#define CAIO_ISERROR(task, e) (CAIO_HASERROR(task) && (task->eno == e))
#define CAIO_CLEARERROR(task) task->eno = 0


#endif  // CAIO_CAIO_H_
