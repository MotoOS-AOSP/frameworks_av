/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_AAUDIO_AUDIO_ENDPOINT_H
#define ANDROID_AAUDIO_AUDIO_ENDPOINT_H

#include <mutex>

#include <aaudio/AAudio.h>

#include "binding/AAudioServiceMessage.h"
#include "binding/AudioEndpointParcelable.h"
#include "fifo/FifoBuffer.h"

namespace aaudio {

#define ENDPOINT_DATA_QUEUE_SIZE_MIN   48

/**
 * A sink for audio.
 * Used by the client code.
 */
class AudioEndpoint {

public:
    AudioEndpoint() = default;

    /**
     * Configure based on the EndPointDescriptor_t.
     */
    aaudio_result_t configure(const EndpointDescriptor *pEndpointDescriptor,
                              aaudio_direction_t direction);

    aaudio_result_t configureDataQueue(const RingBufferDescriptor &descriptor,
                            aaudio_direction_t direction);

    /**
     * Read from a command passed up from the Server.
     * @return 1 if command received, 0 for no command, or negative error.
     */
    aaudio_result_t readUpCommand(AAudioServiceMessage *commandPtr);

    int32_t getEmptyFramesAvailable(android::WrappingBuffer *wrappingBuffer);

    int32_t getEmptyFramesAvailable();

    int32_t getFullFramesAvailable(android::WrappingBuffer *wrappingBuffer);

    int32_t getFullFramesAvailable();

    android::fifo_frames_t read(void* buffer, android::fifo_frames_t numFrames);

    android::fifo_frames_t write(void* buffer, android::fifo_frames_t numFrames);

    void advanceReadIndex(int32_t deltaFrames);

    void advanceWriteIndex(int32_t deltaFrames);

    /**
     * Set the read index in the downData queue.
     * This is needed if the reader is not updating the index itself.
     */
    void setDataReadCounter(android::fifo_counter_t framesRead);

    android::fifo_counter_t getDataReadCounter() const;

    void setDataWriteCounter(android::fifo_counter_t framesWritten);

    android::fifo_counter_t getDataWriteCounter() const;

    /**
     * The result is not valid until after configure() is called.
     *
     * @return true if the output buffer read position is not updated, eg. DMA
     */
    bool isFreeRunning() const { return mFreeRunning; }

    int32_t setBufferSizeInFrames(int32_t requestedFrames,
                                  int32_t *actualFrames);
    int32_t getBufferSizeInFrames() const;

    int32_t getBufferCapacityInFrames() const;

    void setThreshold(int32_t frames) {
        mDataQueue->setThreshold(frames);
    }

    int32_t getThreshold() {
        return mDataQueue->getThreshold();
    }

    /**
     * Write zeros to the data queue memory.
     */
    void eraseDataMemory();

    void eraseEmptyDataMemory(int32_t numFrames);

    void freeDataQueue() { mDataQueue.reset(); }

    void dump() const;

private:
    std::unique_ptr<android::FifoBufferIndirect> mUpCommandQueue;
    std::unique_ptr<android::FifoBufferIndirect> mDataQueue;
    bool                    mFreeRunning{false};
    android::fifo_counter_t mDataReadCounter{0}; // only used if free-running
    android::fifo_counter_t mDataWriteCounter{0}; // only used if free-running
};

} // namespace aaudio

#endif //ANDROID_AAUDIO_AUDIO_ENDPOINT_H
