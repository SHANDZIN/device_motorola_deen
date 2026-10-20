/*
 * Copyright (c) 2011-2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <new>

#include <hardware/gralloc1.h>
#include <log/log.h>

#include "gr_allocator.h"

namespace gralloc1 {

struct AllocData {
    void *base;
    int fd;
    int ion_handle;
    unsigned int offset;
    unsigned int size;
    unsigned int align;
    uintptr_t handle;
    bool uncached;
    unsigned int flags;
    unsigned int heap_id;
    unsigned int alloc_type;
};

class Allocator {
public:
    Allocator();
    ~Allocator();
    bool Init();
    int AllocateMem(AllocData *data, gralloc1_producer_usage_t producer_usage,
                    gralloc1_consumer_usage_t consumer_usage);
    int FreeBuffer(void *base, unsigned int size, unsigned int offset, int fd, int handle);

private:
    gralloc::Allocator *allocator_;
};

static_assert(sizeof(void *) == 4);
static_assert(sizeof(Allocator) == 4);
static_assert(sizeof(AllocData) == 44);
static_assert(offsetof(AllocData, ion_handle) == 8);
static_assert(offsetof(AllocData, size) == 16);
static_assert(offsetof(AllocData, uncached) == 28);
static_assert(offsetof(AllocData, alloc_type) == 40);

Allocator::Allocator() : allocator_(nullptr) {}

Allocator::~Allocator() {
    delete allocator_;
}

bool Allocator::Init() {
    if (allocator_) {
        return true;
    }
    auto *allocator = new (std::nothrow) gralloc::Allocator;
    if (!allocator) {
        return false;
    }
    if (!allocator->Init()) {
        delete allocator;
        return false;
    }
    allocator_ = allocator;
    return true;
}

int Allocator::AllocateMem(AllocData *data, gralloc1_producer_usage_t producer_usage,
                            gralloc1_consumer_usage_t consumer_usage) {
    if (!allocator_ || !data) {
        return -EINVAL;
    }
    if (producer_usage != GRALLOC1_PRODUCER_USAGE_NONE ||
        consumer_usage != GRALLOC1_CONSUMER_USAGE_NONE) {
        ALOGE("Unsupported metadata usage: producer=%#llx consumer=%#llx",
              static_cast<unsigned long long>(producer_usage),
              static_cast<unsigned long long>(consumer_usage));
        return -EINVAL;
    }

    gralloc::AllocData current;
    current.base = data->base;
    current.fd = data->fd;
    current.ion_handle = data->ion_handle;
    current.offset = data->offset;
    current.size = data->size;
    current.align = data->align;
    current.handle = data->handle;
    current.uncached = data->uncached;
    current.flags = data->flags;
    current.heap_id = data->heap_id;
    current.alloc_type = data->alloc_type;

    int result = allocator_->AllocateMem(&current, 0, 0);

    data->base = current.base;
    data->fd = current.fd;
    data->ion_handle = current.ion_handle;
    data->offset = current.offset;
    data->size = current.size;
    data->align = current.align;
    data->handle = current.handle;
    data->uncached = current.uncached;
    data->flags = current.flags;
    data->heap_id = current.heap_id;
    data->alloc_type = current.alloc_type;
    return result;
}

int Allocator::FreeBuffer(void *base, unsigned int size, unsigned int offset, int fd, int handle) {
    if (!allocator_) {
        return -EINVAL;
    }
    return allocator_->FreeBuffer(base, size, offset, fd, handle);
}

}
