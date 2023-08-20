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
#include <stddef.h>

#include <clog.h>

#include "caio.h"


#undef GARR_TYPE
#define GARR_TYPE caiotask
#include "generic_array.h"
#include "generic_array.c"


#undef GSTACK_TYPE
#define GSTACK_TYPE caiocall
#include "generic_stack.c"


static size_t _callstack_size = 0;
static struct caiotask_array _tasks;


#include "caio.h"


int
caio_init(size_t maxtasks, size_t callstack_size) {
    if (caiotask_array_init(&_tasks, maxtasks)) {
        return -1;
    }

    _callstack_size = callstack_size;
    return 0;
}


void
caio_deinit() {
    caiotask_array_deinit(&_tasks);
}


static void
_caio_task_dispose(struct caiotask *task) {
    struct caiocall *call;

    while (task->callstack.count) {
        call = caiocall_stack_pop(&task->callstack);
        free(call);
    }
    caiocall_stack_deinit(&task->callstack);
    caiotask_array_del(&_tasks, task->index);
    free(task);
}


int
caio_task_new(caiocoro coro, void *state) {
    int index;
    struct caiotask *task = malloc(sizeof(struct caiotask));
    if (task == NULL) {
        return -1;
    }
    task->running_coros = 0;

    /* Initialize callstack */
    if (caiocall_stack_init(&task->callstack, _callstack_size)) {
        free(task);
        return -1;
    }

    /* Register task */
    index = caiotask_array_append(&_tasks, task);
    if (index == -1) {
        _caio_task_dispose(task);
        return -1;
    }
    task->index = index;

    if (caio_call_new(task, coro, state)) {
        _caio_task_dispose(task);
        return -1;
    }

    return 0;
}


int
caio_call_new(struct caiotask *task, caiocoro coro, void *state) {
    struct caiocall *call = malloc(sizeof(struct caiocall));
    if (call == NULL) {
        _caio_task_dispose(task);
        return -1;
    }

    call->coro = coro;
    call->state = state;
    if (caiocall_stack_push(&task->callstack, call) == -1) {
        _caio_task_dispose(task);
        return -1;
    }

    task->running_coros++;
    return 0;
}


int
caio_task_step(struct caiotask *task) {
    struct caiocall *call = caiocall_stack_last(&task->callstack);

    /* Get a shot of whiskey to coro */
    enum caiocoro_status status = call->coro(task, call->state);
    if (status == ccs_done) {
        caiocall_stack_pop(&task->callstack);
        free(call);
        task->running_coros--;
    }

    if (task->running_coros == 0) {
        _caio_task_dispose(task);
    }

    return 0;
}


int
caio_forever() {
    int taskindex;
    struct caiotask *task = NULL;

    while (_tasks.count) {
        for (taskindex = 0; taskindex < _tasks.size; taskindex++) {
            task = caiotask_array_get(&_tasks, taskindex);
            if (task == NULL) {
                continue;
            }
            caio_task_step(task);
        }
    }

    return EXIT_SUCCESS;
}
