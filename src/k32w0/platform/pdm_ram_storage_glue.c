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
#define mutex_lock OSA_MutexLock
#define mutex_unlock OSA_MutexUnlock
#else
#define mutex_lock(...)
#define mutex_unlock(...)
#endif

#define MAX_QUEUE_SIZE (16)

typedef struct
{
    ramBufferDescriptor *pvDataBuffer;
    uint16_t             u16IdValue;
} tsQueueEntry;

static tsQueueEntry asQueue[MAX_QUEUE_SIZE];
static uint8_t      u8QueueWritePtr;
static uint8_t      u8QueueReadPtr;
static osaMutexId_t asQueueMutex;

static uint8_t u8IncrementQueuePtr(uint8_t u8CurrentValue);

#endif /* PDM_SAVE_IDLE */

#if !ENABLE_STORAGE_DYNAMIC_MEMORY
#ifndef PDM_BUFFER_SIZE
#define PDM_BUFFER_SIZE (1024 + kRamDescSize) /* kRamBufferInitialSize is 1024 */
#endif
static uint8_t sPdmBuffer[PDM_BUFFER_SIZE] __attribute__((aligned(4))) = {0};

static uint8_t sPdmStagingBuffer[PDM_BUFFER_SIZE] __attribute__((aligned(4))) = {0};
#endif

#if PDM_ENCRYPTION

static PDM_portConfig_t pdm_PortContext = {NULL, 0, NULL, 0};

/* Initialize and enable PDM encryption context */
static void initPdmEncContext(PDM_portConfig_t *pdm_PortContext,
                              uint8_t *         stagingBuffer,
                              uint16_t          stagingBufferSize,
                              uint8_t *         encKey)
{
    if (!pdm_PortContext->pStaging_buf)
    {
        PDM_teStatus status = PDM_E_STATUS_OK;

        pdm_PortContext->pStaging_buf    = stagingBuffer;
        pdm_PortContext->taging_buf_size = stagingBufferSize;
        pdm_PortContext->pEncryptionKey  = encKey;
        pdm_PortContext->config_flags    = PDM_ENCRYPTION_ENABLED;

        status = PDM_SetEncryption((const PDM_portConfig_t *)pdm_PortContext);
        assert(status == PDM_E_STATUS_OK);
    }
}

#endif /* PDM_ENCRYPTION */

#if ENABLE_STORAGE_DYNAMIC_MEMORY

extern void *otPlatRealloc(void *ptr, size_t aSize);

static void HandleError(ramBufferDescriptor **buffer)
{
    if (*buffer != NULL)
    {
        otPlatFree(*buffer);
        *buffer = NULL;
    }
}

ramBufferDescriptor *getRamBuffer(uint16_t nvmId, uint16_t initialSize)
{
    ramBufferDescriptor *ramDescr         = NULL;
    bool_t               bLoadDataFromNvm = FALSE;
#if PDM_ENCRYPTION
    uint8_t *stagingBuffer     = NULL;
    uint16_t stagingBufferSize = 0;
#endif

    ramDescr = (ramBufferDescriptor *)otPlatCAlloc(1, kRamDescSize);
    otEXPECT_ACTION(ramDescr != NULL, HandleError(&ramDescr));

    ramDescr->header.maxLength = initialSize;
#if PDM_SAVE_IDLE
    ramDescr->header.mutexHandle = OSA_MutexCreate();
    otEXPECT_ACTION(ramDescr->header.mutexHandle != NULL, HandleError(&ramDescr));
#endif

    /* Check if dataset is present and get its size */
    if (PDM_bDoesDataExist(nvmId, &ramDescr->header.length))
    {
        bLoadDataFromNvm = TRUE;
        while (ramDescr->header.length > ramDescr->header.maxLength)
        {
            // increase size until NVM data fits
            ramDescr->header.maxLength += kRamBufferReallocSize;
        }
    }
    otEXPECT_ACTION(ramDescr->header.maxLength <= kRamBufferMaxAllocSize, HandleError(&ramDescr));

#if PDM_ENCRYPTION
    stagingBuffer     = (uint8_t *)otPlatCAlloc(1, ramDescr->header.maxLength);
    stagingBufferSize = stagingBuffer ? ramDescr->header.maxLength : 0;

    /* Use alocated staging buffer for encryption result.
     * Don't pass any encryption key, use the EFUSE key.
     */
    initPdmEncContext(&pdm_PortContext, stagingBuffer, stagingBufferSize, NULL);
#endif

    ramDescr->buffer = (uint8_t *)otPlatCAlloc(1, ramDescr->header.maxLength);
    otEXPECT_ACTION(ramDescr->buffer != NULL, HandleError(&ramDescr));

    if (bLoadDataFromNvm)
    {
        PDM_teStatus status =
            PDM_eReadDataFromRecord(nvmId, ramDescr->buffer, ramDescr->header.maxLength, &ramDescr->header.length);
        if ((PDM_E_STATUS_OK != status) || (ramDescr->header.length > ramDescr->header.maxLength))
        {
            ramDescr->header.length = 0;
            PDM_vDeleteDataRecord(nvmId);
        }
    }

exit:
    return ramDescr;
}

rsError ramStorageResize(ramBufferDescriptor *pBuffer, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
    rsError        err            = RS_ERROR_NONE;
    uint16_t       allocSize      = pBuffer->header.maxLength;
    const uint16_t newBlockLength = sizeof(struct settingsBlock) + aValueLength;
    uint8_t *      ptr            = NULL;

    otEXPECT_ACTION((NULL != pBuffer), err = RS_ERROR_NO_BUFS);

    if (allocSize < pBuffer->header.length + newBlockLength)
    {
        while ((allocSize < pBuffer->header.length + newBlockLength))
        {
            /* Need to realocate the memory buffer, increase size by kRamBufferReallocSize until NVM data fits */
            allocSize += kRamBufferReallocSize;
        }

        if (allocSize <= kRamBufferMaxAllocSize)
        {
            ptr = (uint8_t *)otPlatRealloc((void *)pBuffer->buffer, allocSize);
            otEXPECT_ACTION((NULL != ptr), err = RS_ERROR_NO_BUFS);
            pBuffer->buffer           = ptr;
            pBuffer->header.maxLength = allocSize;

#if PDM_ENCRYPTION
            pdm_PortContext.pStaging_buf = (uint8_t *)otPlatRealloc((void *)(pdm_PortContext.pStaging_buf), allocSize);
            pdm_PortContext.staging_buf_size = pdm_PortContext.pStaging_buf ? allocSize : 0;
#endif
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
    OT_UNUSED_VARIABLE(initialSize);
    ramBufferDescriptor *ramDescr  = (ramBufferDescriptor *)&sPdmBuffer;
    uint16_t             bytesRead = 0;

    ramDescr->header.maxLength   = PDM_BUFFER_SIZE - kRamDescSize;
    ramDescr->buffer             = (uint8_t *)&sPdmBuffer[kRamDescSize];
#if PDM_SAVE_IDLE
    ramDescr->header.mutexHandle = OSA_MutexCreate();
    otEXPECT(ramDescr->header.mutexHandle != NULL);
#endif

#if PDM_ENCRYPTION
    /* Use static staging buffer for encryption result.
     * Don't pass any encryption key, use the EFUSE key.
     */
    initPdmEncContext(&pdm_PortContext, sPdmStagingBuffer, PDM_BUFFER_SIZE, NULL);
#endif

    if (PDM_bDoesDataExist(nvmId, &ramDescr->header.length))
    {
        PDM_teStatus status =
            PDM_eReadDataFromRecord(nvmId, ramDescr->buffer, ramDescr->header.maxLength, &ramDescr->header.length);
        if ((PDM_E_STATUS_OK != status) || (ramDescr->header.length > ramDescr->header.maxLength))
        {
            ramDescr->header.length = 0;
            PDM_vDeleteDataRecord(nvmId);
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

PDM_teStatus FS_eSaveRecordDataInIdleTask(uint16_t u16IdValue, ramBufferDescriptor *pvDataBuffer)
{
    tsQueueEntry *psQueueEntry;
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
    bool_t  already_in = FALSE;
    while (u8QueuePtr != u8QueueWritePtr)
    {
        psQueueEntry = &asQueue[u8QueuePtr];
        if (psQueueEntry->u16IdValue == u16IdValue)
        {
            already_in = TRUE;
            break;
        }
        u8QueuePtr = u8IncrementQueuePtr(u8QueuePtr);
    }
    if (already_in == FALSE)
    {
        /* Write new entry to queue */
        psQueueEntry = &asQueue[u8QueueWritePtr];

        psQueueEntry->u16IdValue   = u16IdValue;
        psQueueEntry->pvDataBuffer = pvDataBuffer;

        /* Update write pointer */
        u8QueueWritePtr = u8IncrementQueuePtr(u8QueueWritePtr);
    }

    mutex_unlock(asQueueMutex);

    return status;
}

void FS_vIdleTask(uint8_t u8WritesAllowed)
{
    tsQueueEntry         currentEntry;
    ramBufferDescriptor *ramBuffer       = NULL;
    uint8_t *            buffer          = NULL;
    uint16_t             bufferSize      = 0;
    uint16_t             bufferAllocSize = 0;
    bool_t               doPdmSave       = FALSE;
    PDM_teStatus         pdmStatus       = PDM_E_STATUS_INTERNAL_ERROR;

    if (u8WritesAllowed > MAX_QUEUE_SIZE)
        u8WritesAllowed = MAX_QUEUE_SIZE;

    while ((u8QueueReadPtr != u8QueueWritePtr) && (u8WritesAllowed > 0))
    {
        mutex_lock(asQueueMutex, osaWaitForever_c);
        memcpy(&currentEntry, &asQueue[u8QueueReadPtr], sizeof(tsQueueEntry));
        u8QueueReadPtr = u8IncrementQueuePtr(u8QueueReadPtr);
        mutex_unlock(asQueueMutex);

        ramBuffer = currentEntry.pvDataBuffer;
        if (osaStatus_Success == mutex_lock(ramBuffer->header.mutexHandle, 0))
        {
            if ((buffer == NULL) || (bufferAllocSize < ramBuffer->header.length))
            {
                bufferAllocSize = ramBuffer->header.length;
                buffer          = (uint8_t *)otPlatRealloc(buffer, bufferAllocSize);
            }

            if (buffer != NULL)
            {
                memcpy(buffer, ramBuffer->buffer, ramBuffer->header.length);
                bufferSize = ramBuffer->header.length;
                doPdmSave  = TRUE;
            }

            mutex_unlock(ramBuffer->header.mutexHandle);
        }

        if (doPdmSave == TRUE)
        {
            pdmStatus = PDM_eSaveRecordData(currentEntry.u16IdValue, buffer, bufferSize);
            doPdmSave = FALSE;
        }

        if (pdmStatus != PDM_E_STATUS_OK)
        {
            FS_eSaveRecordDataInIdleTask(currentEntry.u16IdValue, ramBuffer);
        }

        u8WritesAllowed--;
        pdmStatus = PDM_E_STATUS_INTERNAL_ERROR;
    }

    if (buffer != NULL)
    {
        otPlatFree(buffer);
    }
}

#endif /* PDM_SAVE_IDLE */
