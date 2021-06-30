/*
 * Copyright (c) 2014 VMware, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "precomp.h"
#include "Recirc.h"
#ifdef OVS_DBG_MOD
#undef OVS_DBG_MOD
#endif
#define OVS_DBG_MOD OVS_DBG_OTHERS

#include "Debug.h"

extern NDIS_HANDLE gOvsExtDriverHandle;

_Use_decl_annotations_
VOID*
OvsAllocateMemoryWithTag(size_t size, ULONG tag)
{
    return NdisAllocateMemoryWithTagPriority(gOvsExtDriverHandle,
        (UINT32)size, tag, NormalPoolPriority);
}

VOID
OvsFreeMemoryWithTag(VOID *ptr, ULONG tag)
{
    ASSERT(ptr);
    NdisFreeMemoryWithTagPriority(gOvsExtDriverHandle, ptr, tag);
}

_Use_decl_annotations_
VOID *
OvsAllocateMemory(size_t size)
{
    return NdisAllocateMemoryWithTagPriority(gOvsExtDriverHandle,
        (UINT32)size, OVS_MEMORY_TAG, NormalPoolPriority);
}

_Use_decl_annotations_
VOID *
OvsAllocateAlignedMemory(size_t size, UINT16 align)
{
    ASSERT((align == 8) || (align == 16));

    if ((align == 8) || (align == 16)) {
        /*
         * XXX: NdisAllocateMemory*() functions don't talk anything about
         * alignment. Hence using ExAllocatePool*();
         */
        return (VOID *)ExAllocatePoolWithTagPriority(NonPagedPoolNx, size,
                                                     OVS_MEMORY_TAG,
                                                     NormalPoolPriority);
    }

    /* Invalid user input. */
    return NULL;
}

VOID
OvsFreeMemory(VOID *ptr)
{
    ASSERT(ptr);
    NdisFreeMemoryWithTagPriority(gOvsExtDriverHandle, ptr, OVS_MEMORY_TAG);
}

VOID
OvsFreeAlignedMemory(VOID *ptr)
{
    ASSERT(ptr);
    ExFreePoolWithTag(ptr, OVS_MEMORY_TAG);
}

VOID
OvsAppendList(PLIST_ENTRY dst, PLIST_ENTRY src)
{
    PLIST_ENTRY srcFirst, srcLast, dstLast;
    if (IsListEmpty(src)) {
        return;
    }
    srcFirst = src->Flink;
    srcLast = src->Blink;
    dstLast = dst->Blink;

    dstLast->Flink = srcFirst;
    srcFirst->Blink = dstLast;

    srcLast->Flink = dst;
    dst->Blink = srcLast;

    src->Flink = src;
    src->Blink = src;
}

BOOLEAN
OvsCompareString(PVOID string1, PVOID string2)
{
    /*
     * Not a super-efficient string compare since we walk over the strings
     * twice: to initialize, and then to do the comparison.
     */
    STRING str1, str2;

    RtlInitString(&str1, string1);
    RtlInitString(&str2, string2);
    return RtlEqualString(&str1, &str2, FALSE);
}

VOID *
OvsAllocateMemoryPerCpu(size_t size,
                        size_t count,
                        ULONG tag)
{
    VOID *ptr = NULL;

    ASSERT(KeQueryActiveGroupCount() == 1);

    if (!count) {
        count = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
    }

    ptr = OvsAllocateMemoryWithTag(count * size, tag);
    if (ptr) {
        RtlZeroMemory(ptr, count * size);
    }

    return ptr;
}

/*
 * --------------------------------------------------------------------------
 * OvsPerCpuDataInit --
 *     The function allocates necessary per-processor resources.
 * --------------------------------------------------------------------------
 */
NTSTATUS
OvsPerCpuDataInit()
{
    return OvsDeferredActionsInit();
}

/*
 * --------------------------------------------------------------------------
 * OvsPerCpuDataCleanup --
 *     The function frees all per-processor resources.
 * --------------------------------------------------------------------------
 */
VOID
OvsPerCpuDataCleanup()
{
    OvsDeferredActionsCleanup();
}
