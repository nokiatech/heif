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
#include "OutputStream.h"
#include <assert.h>
#include <cstring>

OutputStream::OutputStream(JNIEnv* env, jobject javaStream)
    : mJNIEnv(env)
{
    mJavaStream = env->NewGlobalRef(javaStream);
    mJavaClass  = env->GetObjectClass(javaStream);
    assert(mJavaClass != 0);

    mSeekMethodId = env->GetMethodID(mJavaClass, "seek", "(J)V");
    assert(mSeekMethodId != 0);
    mPositionMethodId = env->GetMethodID(mJavaClass, "position", "()J");
    assert(mPositionMethodId != 0);
    mRemoveMethodId = env->GetMethodID(mJavaClass, "clear", "()V");
    assert(mRemoveMethodId != 0);
    mWriteMethodId = env->GetMethodID(mJavaClass, "write", "(Ljava/nio/ByteBuffer;J)V");
    assert(mWriteMethodId != 0);
}

OutputStream::~OutputStream()
{
    mJNIEnv->DeleteGlobalRef(mJavaStream);
}

void OutputStream::write(const void* buffer, std::uint64_t size)
{
    jobject byteBuffer = mJNIEnv->NewDirectByteBuffer(const_cast<void*>(buffer), size);
    mJNIEnv->CallVoidMethod(mJavaStream, mWriteMethodId, byteBuffer, size);
}

void OutputStream::seekp(std::uint64_t aPosition)
{
    return mJNIEnv->CallVoidMethod(mJavaStream, mSeekMethodId, aPosition);
}

std::uint64_t OutputStream::tellp()
{
    return (std::uint64_t)mJNIEnv->CallLongMethod(mJavaStream, mPositionMethodId);
}

void OutputStream::remove()
{
    return mJNIEnv->CallVoidMethod(mJavaStream, mRemoveMethodId);
}
