/*
 *  Copyright (c) 2022, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file pdm_ram_storage_glue.c
 * File used for the glue between PDM and RAM Buffer
 *
 */

#include "PDM.h"
#include "platform-k32w.h"
#include "ram_storage.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <utils/code_utils.h>
#include <openthread/platform/memory.h>

#if PDM_SAVE_IDLE
#include "fsl_os_abstraction.h"

#if defined(USE_RTOS) && (USE_RTOS == 1)
#include "FreeRTOS.h"
#include "task.h"

#define sched_disable vTaskSuspendAll
#define sched_enable xTaskResumeAll
#define mutex_lock OSA_MutexLock
#define mutex_unlock OSA_MutexUnlock
#else
#define sched_disable(...)
#define sched_enable(...)
#define mutex_lock(...)
#define mutex_unlock(...)
#endif

#define MAX_QUEUE_SIZE (16)

typedef struct
{
    void *   pvDataBuffer;
    uint16_t u16IdValue;
    uint16_t u16Datalength;
} tsQueueEntry;

static tsQueueEntry asQueue[MAX_QUEUE_SIZE];
static uint8_t      u8QueueWritePtr;
static uint8_t      u8QueueReadPtr;
static osaMutexId_t asQueueMutex;

static uint8_t u8IncrementQueuePtr(uint8_t u8CurrentValue);

#endif /* PDM_SAVE_IDLE */

#if !ENABLE_STORAGE_DYNAMIC_MEMORY
#ifndef PDM_BUFFER_SIZE
#define PDM_BUFFER_SIZE (1024 + sizeof(ramBufferDescriptor)) /* kRamBufferInitialSize is 1024 */
#endif
static uint8_t sPdmBuffer[PDM_BUFFER_SIZE] __attribute__((aligned(4))) = {0};
#endif

#if ENABLE_STORAGE_DYNAMIC_MEMORY

extern void *otPlatRealloc(void *ptr, size_t aSize);

ramBufferDescriptor *getRamBuffer(uint16_t nvmId, uint16_t initialSize)
{
    ramBufferDescriptor *ramDescr         = NULL;
    bool_t               bLoadDataFromNvm = FALSE;
    uint16_t             bytesRead        = 0;
    uint16_t             recordSize       = 0;
    uint16_t             allocSize        = initialSize;

    /* Check if dataset is present and get its size */
    if (PDM_bDoesDataExist(nvmId, &recordSize))
    {
        bLoadDataFromNvm = TRUE;
        while (recordSize > allocSize)
        {
            // increase size until NVM data fits
            allocSize += kRamBufferReallocSize;
        }
    }

    if (allocSize <= kRamBufferMaxAllocSize)
    {
        ramDescr = (ramBufferDescriptor *)otPlatCAlloc(1, allocSize);
        if (ramDescr)
        {
            ramDescr->ramBufferLen = 0;

            if (bLoadDataFromNvm)
            {
                /* Try to load the dataset in RAM */
                if (PDM_E_STATUS_OK != PDM_eReadDataFromRecord(nvmId, ramDescr, recordSize, &bytesRead))
                {
                    memset(ramDescr, 0, allocSize);
                }
            }

            /* ramBufferMaxLen should hold the runtime allocated size */
            ramDescr->ramBufferMaxLen = allocSize - kRamDescHeaderSize;
        }
    }

    return ramDescr;
}

rsError ramStorageResize(ramBufferDescriptor **pBuffer, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
    rsError              err            = RS_ERROR_NONE;
    uint16_t             allocSize      = (*pBuffer)->ramBufferMaxLen;
    const uint16_t       newBlockLength = sizeof(struct settingsBlock) + aValueLength;
    ramBufferDescriptor *ptr            = NULL;

    otEXPECT_ACTION((NULL != *pBuffer), err = RS_ERROR_NO_BUFS);

    if (allocSize < (*pBuffer)->ramBufferLen + newBlockLength)
    {
        while ((allocSize < (*pBuffer)->ramBufferLen + newBlockLength))
        {
            /* Need to realocate the memory buffer, increase size by kRamBufferReallocSize until NVM data fits */
            allocSize += kRamBufferReallocSize;
        }

        allocSize += kRamDescHeaderSize;

        if (allocSize <= kRamBufferMaxAllocSize)
        {
            ptr = (ramBufferDescriptor *)otPlatRealloc((void *)(*pBuffer), allocSize);
            otEXPECT_ACTION((NULL != ptr), err = RS_ERROR_NO_BUFS);
            *pBuffer                    = ptr;
            (*pBuffer)->ramBufferMaxLen = allocSize - kRamDescHeaderSize;
        }
        else
        {
            err = RS_ERROR_NO_BUFS;
        }
    }

exit:
    return err;
}

#else
ramBufferDescriptor *getRamBuffer(uint16_t nvmId, uint16_t initialSize)
{
    ramBufferDescriptor *ramDescr  = (ramBufferDescriptor *)&sPdmBuffer;
    uint16_t             bytesRead = 0;

    ramDescr->ramBufferMaxLen = PDM_BUFFER_SIZE - kRamDescHeaderSize;
    assert(initialSize <= ramDescr->ramBufferMaxLen);

    if (PDM_bDoesDataExist(nvmId, &bytesRead))
    {
        /* Try to load the dataset in RAM */
        if (PDM_E_STATUS_OK != PDM_eReadDataFromRecord(nvmId, ramDescr, PDM_BUFFER_SIZE, &bytesRead))
        {
            memset(ramDescr, 0, PDM_BUFFER_SIZE);
        }
    }

    return ramDescr;
}
#endif

#if PDM_SAVE_IDLE

uint8_t u8IncrementQueuePtr(uint8_t u8CurrentValue)
{
    uint8_t u8IncrementedPtr;

    u8IncrementedPtr = u8CurrentValue + 1;
    if (u8IncrementedPtr == MAX_QUEUE_SIZE)
    {
        /* Wrap pointer back to start */
        u8IncrementedPtr = 0;
    }

    return u8IncrementedPtr;
}

PDM_teStatus FS_eSaveRecordDataInIdleTask(uint16_t u16IdValue, void *pvDataBuffer, uint16_t u16Datalength)
{
    tsQueueEntry *psQueueEntry;
    uint8_t       u8WriteNext;
    PDM_teStatus  status = PDM_E_STATUS_OK;

#if defined(USE_RTOS) && (USE_RTOS == 1)
    OSA_InterruptDisable();
    if (asQueueMutex == NULL)
    {
        asQueueMutex = OSA_MutexCreate();
        if (asQueueMutex == NULL)
        {
            status = PDM_E_STATUS_NOT_SAVED;
        }
    }
    OSA_InterruptEnable();

    if (status != PDM_E_STATUS_OK)
    {
        return status;
    }
#endif

    mutex_lock(asQueueMutex, osaWaitForever_c);

    /* Instead of updating PDM immediately we queue request until later. If
     * queue is full, performs first write in queue synchronously, to avoid
     * data loss. Queue is implemented as a wrap-around with read and write
     * pointers, so adding or removing item from queue is quick
     */

    /* Look ahead in Queue to find if the same u16IdValue is already engaged */
    uint8_t u8QueuePtr = u8QueueReadPtr;
    bool_t  already_in = false;
    while (u8QueuePtr != u8QueueWritePtr)
    {
        psQueueEntry = &asQueue[u8QueuePtr];
        if (psQueueEntry->u16IdValue == u16IdValue)
        {
            /* If an entry with the given id is found, update both buffer
             * address and length. Some buffers may be dynamically allocated
             * and reallocated at a later time with a possibility of address
             * change. Changing the pointer here is a safe solution as for
             * the moment the FreeRTOS scheduler is suspended/enabled before/
             * after FS_vIdleTask so there is no risk of other usage for this
             * buffer.
             */
            psQueueEntry->pvDataBuffer  = pvDataBuffer;
            psQueueEntry->u16Datalength = u16Datalength;
            already_in                  = true;
            break;
        }
        u8QueuePtr = u8IncrementQueuePtr(u8QueuePtr);
    }
    if (already_in == FALSE)
    {
        u8WriteNext = u8IncrementQueuePtr(u8QueueWritePtr);

        if (u8WriteNext == u8QueueReadPtr)
        {
            /* Queue is full, so perform first queued write synchronously
             * to make space. This should never happen with the current
             * implementation.
             */
            psQueueEntry = &asQueue[u8QueueReadPtr];

            status =
                PDM_eSaveRecordData(psQueueEntry->u16IdValue, psQueueEntry->pvDataBuffer, psQueueEntry->u16Datalength);

            /* Move read pointer onwards */
            u8QueueReadPtr = u8IncrementQueuePtr(u8QueueReadPtr);
        }

        /* Write new entry to queue */
        psQueueEntry = &asQueue[u8QueueWritePtr];

        psQueueEntry->u16IdValue    = u16IdValue;
        psQueueEntry->pvDataBuffer  = pvDataBuffer;
        psQueueEntry->u16Datalength = u16Datalength;

        /* Update write pointer */
        u8QueueWritePtr = u8WriteNext;
    }

    mutex_unlock(asQueueMutex);

    return status;
}

void FS_vIdleTask(uint8_t u8WritesAllowed)
{
    /* Processes queued write requests */
    tsQueueEntry *psQueueEntry;

    if (u8WritesAllowed > MAX_QUEUE_SIZE)
        u8WritesAllowed = MAX_QUEUE_SIZE;

    /* temporary workaround: suspend/enable
     * the FreeRTOS scheduler during PDM writes.
     */
    sched_disable();
    while ((u8QueueReadPtr != u8QueueWritePtr) && (u8WritesAllowed > 0))
    {
        psQueueEntry = &asQueue[u8QueueReadPtr];

        (void)PDM_eSaveRecordData(psQueueEntry->u16IdValue, psQueueEntry->pvDataBuffer, psQueueEntry->u16Datalength);

        /* Move read pointer onwards */
        u8QueueReadPtr = u8IncrementQueuePtr(u8QueueReadPtr);
        u8WritesAllowed--;
    }
    sched_enable();
}

#endif /* PDM_SAVE_IDLE */
