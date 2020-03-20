/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_GOLDFISH_OPENGL_SYSTEM_CBMANAGER_CBMANAGER_H
#define ANDROID_GOLDFISH_OPENGL_SYSTEM_CBMANAGER_CBMANAGER_H

#include <cutils/native_handle.h>
#include <log/log.h>

#include <memory>
#include <android/hardware/graphics/common/1.0/types.h>
#include <android/hardware/graphics/mapper/2.0/IMapper.h>

#include "gralloc_cb.h"

namespace android {

class CbManager {
public:
    typedef hardware::graphics::common::V1_0::BufferUsage BufferUsage;
    typedef hardware::graphics::common::V1_0::PixelFormat PixelFormat;
    typedef hardware::graphics::mapper::V2_0::YCbCrLayout YCbCrLayout;
    typedef hardware::hidl_bitfield<BufferUsage> BufferUsageBits;

    CbManager();

    class CbManagerImpl {
    public:
        virtual ~CbManagerImpl() {}
        virtual native_handle_t* allocateBuffer(int width,
                                                int height,
                                                PixelFormat format,
                                                BufferUsageBits usage) = 0;
        virtual void freeBuffer(const native_handle_t* h) = 0;

        virtual int lockBuffer(native_handle_t& handle,
             BufferUsageBits usage,
             int left, int top, int width, int height,
             void** vaddr) = 0;

        virtual int lockYCbCrBuffer(native_handle_t& handle,
             BufferUsageBits usage,
             int left, int top, int width, int height,
             YCbCrLayout* ycbcr) = 0;

        virtual int unlockBuffer(native_handle_t& handle) = 0;
    };

    native_handle_t* allocateBuffer(
        int width, int height, PixelFormat format, BufferUsageBits usage) {
        return mImpl->allocateBuffer(width, height, format, usage);
    }

    void freeBuffer(const native_handle_t* h) {
        mImpl->freeBuffer(h);
    }

    int lockBuffer(native_handle_t& handle,
             BufferUsageBits usage,
             int left, int top, int width, int height,
             void** vaddr) {
        return mImpl->lockBuffer(handle, usage, left, top, width, height, vaddr);
    }

    int lockBuffer(buffer_handle_t h,
             BufferUsageBits usage,
             int left, int top, int width, int height,
             void** vaddr) {
        native_handle_t* cb = const_cast<native_handle_t*>(h);
        if (cb) {
            return lockBuffer(*cb, usage, left, top, width, height, vaddr);
        } else {
            return -1;
        }
    }


    int lockYCbCrBuffer(native_handle_t& handle,
                        BufferUsageBits usage,
                        int left, int top, int width, int height,
                        YCbCrLayout* ycbcr) {
        return mImpl->lockYCbCrBuffer(handle, usage, left, top, width, height, ycbcr);
    }

    int lockYCbCrBuffer(buffer_handle_t h,
                        BufferUsageBits usage,
                        int left, int top, int width, int height,
                        YCbCrLayout* ycbcr) {
        native_handle_t* cb = const_cast<native_handle_t*>(h);
        if (cb) {
            return lockYCbCrBuffer(*cb, usage, left, top, width, height, ycbcr);
        } else {
            return -1;
        }
    }

    int unlockBuffer(native_handle_t& handle) {
        return mImpl->unlockBuffer(handle);
    }

    int unlockBuffer(buffer_handle_t h) {
        native_handle_t* cb = const_cast<native_handle_t*>(h);
        if (cb) {
            return unlockBuffer(*cb);
        } else {
            return -1;
        }
    }

    // Specific to goldfish, for obtaining offsets
    // into host coherent memory
    // (requires address space devce)
    static uint64_t getOffset(const buffer_handle_t h) {
        const cb_handle_t* cb = cb_handle_t::from(h);
        if (!cb->isValid()) {
            ALOGE("%s: FATAL: using incompatible native_handle_t for "
                  "host coherent mapping offset",
                  __func__);
            abort();
        }
        return cb ? cb->getMmapedOffset() : -1;
    }

private:
    std::unique_ptr<CbManagerImpl> mImpl;
};

}  // namespace android

#endif  // ANDROID_GOLDFISH_OPENGL_SYSTEM_CBMANAGER_CBMANAGER_H
