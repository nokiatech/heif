/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 *
 *
 */

#pragma once

#include <OutputStreamInterface.h>
#include <jni.h>

#include <cstdint>

class OutputStream : public HEIF::OutputStreamInterface
{
public:
    OutputStream(JNIEnv* env, jobject javaStream);

    ~OutputStream() override;

    // From StreamInterface
public:
    /** Sets current write position
     * @param [in]  aPosition Position in stream where next write will occur. */
    void seekp(std::uint64_t aPosition) override;

    /** Returns current position in stream.
     * @return Current position in stream. */
    std::uint64_t tellp() override;

    /** Writes data to stream
     * @param [in] buffer The buffer of date to write to the stream.
     * @param [in] size   The number of bytes to write to the stream.
     */
    void write(const void* buffer, std::uint64_t size) override;

    /** Request to remove the file.
     *  Called on error cases to cleanup partial files.
     */
    void remove() override;

private:
    JNIEnv* mJNIEnv;
    jobject mJavaStream;
    jclass mJavaClass;
    jmethodID mWriteMethodId;
    jmethodID mSeekMethodId;
    jmethodID mPositionMethodId;
    jmethodID mRemoveMethodId;
};
