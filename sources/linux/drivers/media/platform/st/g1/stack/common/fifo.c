/*
 * (C) COPYRIGHT 2012 HANTRO PRODUCTS
 *
 * Please contact: hantro-support@verisilicon.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#include "fifo.h"
#include "dwl.h"

#ifdef _ASSERT_USED
#ifndef ASSERT
#include <assert.h>
#define ASSERT(expr) assert(expr)
#endif
#else
#define ASSERT(expr)
#endif

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/* Container for instance. */
typedef struct fifo_instance_
{
    sem_t cs_semaphore;    /* Semaphore for critical section. */
    sem_t read_semaphore;  /* Semaphore for readers. */
    sem_t write_semaphore; /* Semaphore for writers. */
    u32 num_of_slots_;
    u32 num_of_objects_;
    u32 tail_index_;
    fifo_object* nodes_;
} fifo_instance;

fifo_ret fifo_init(u32 num_of_slots, fifo_inst* instance)
{
    fifo_instance* inst = DWLmalloc(sizeof(fifo_instance));
    if (inst == NULL)
        return FIFO_ERROR_MEMALLOC;
    inst->num_of_slots_ = num_of_slots;
    /* Allocate memory for the objects. */
    inst->nodes_ = DWLmalloc(sizeof(fifo_object) * num_of_slots);
    if (inst == NULL)
    {
        DWLfree(inst);
        return FIFO_ERROR_MEMALLOC;
    }
    /* Initialize binary critical section semaphore. */
    sem_init(&inst->cs_semaphore, 0, 1);
    /* Then initialize the read and write semaphores. */
    sem_init(&inst->read_semaphore, 0, 0);
    sem_init(&inst->write_semaphore, 0, num_of_slots);
    *instance = inst;
    return FIFO_OK;
}

u32 fifo_push(fifo_inst inst, fifo_object object)
{
    u32 ret = 0;
    fifo_instance* instance = (fifo_instance*)inst;
    sem_wait(&instance->write_semaphore);
    sem_wait(&instance->cs_semaphore);
    instance->nodes_[(instance->tail_index_ + instance->num_of_objects_) %
      instance->num_of_slots_] = object;
    instance->num_of_objects_++;
    ret = instance->num_of_objects_;
    sem_post(&instance->cs_semaphore);
    sem_post(&instance->read_semaphore);

    return ret;
}

u32 fifo_pop(fifo_inst inst, fifo_object* object)
{
    u32 ret;
    fifo_instance* instance = (fifo_instance*)inst;
    sem_wait(&instance->read_semaphore);
    sem_wait(&instance->cs_semaphore);
    *object = instance->nodes_[instance->tail_index_ % instance->num_of_slots_];
    instance->tail_index_++;
    instance->num_of_objects_--;
    ret = instance->num_of_objects_;
    sem_post(&instance->cs_semaphore);
    sem_post(&instance->write_semaphore);
    return ret;
}

u32 fifo_count(fifo_inst inst)
{
    u32 count;
    fifo_instance* instance = (fifo_instance*)inst;
    sem_wait(&instance->cs_semaphore);
    count = instance->num_of_objects_;
    sem_post(&instance->cs_semaphore);
    return count;
}

void fifo_release(fifo_inst inst)
{
    fifo_instance* instance = (fifo_instance*)inst;
    ASSERT(instance->num_of_objects_ == 0);
    sem_wait(&instance->cs_semaphore);
    sem_destroy(&instance->cs_semaphore);
    sem_destroy(&instance->read_semaphore);
    sem_destroy(&instance->write_semaphore);
    DWLfree(instance->nodes_);
    DWLfree(instance);
}

