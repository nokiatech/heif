/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

#include <jni.h>
#include <OutputStreamInterface.h>
#include <cstdint>

class OutputStream : public HEIF::OutputStreamInterface
{
public:
    OutputStream(JNIEnv* env, jobject javaStream);

    virtual ~OutputStream();

    // From StreamInterface
public:

    /** Sets current write position
    @param [aPosition] Position in stream where next write will occur. */
    virtual void seekp(std::uint64_t aPosition);

    /** Returns current position in stream.
    @return Current position in stream. */
    virtual std::uint64_t tellp();

    /** Writes data to stream
    @param [buffer] The buffer of date to write to the stream
    @param [size]   The number of bytes to write to the stream
    */
    virtual void write(const void* aBuffer, std::uint64_t aSize);

    /** Request to remove the file.
        Called on error cases to cleanup partial files.
    */
    virtual void remove();

private:
    JNIEnv* mJNIEnv;
    jobject mJavaStream;
    jclass mJavaClass;
    jmethodID mWriteMethodId;
    jmethodID mSeekMethodId;
    jmethodID mPositionMethodId;
    jmethodID mRemoveMethodId;
};