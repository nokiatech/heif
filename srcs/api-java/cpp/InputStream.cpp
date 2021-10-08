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
#include "InputStream.h"

#include <cassert>
#include <cstring>

InputStream::InputStream(JNIEnv* env, jobject javaStream)
    : mJNIEnv(env)
{
    mJavaStream = env->NewGlobalRef(javaStream);
    mJavaClass  = env->GetObjectClass(javaStream);
    assert(mJavaClass != nullptr);

    mSeekMethodId = env->GetMethodID(mJavaClass, "seek", "(J)Z");
    assert(mSeekMethodId != nullptr);
    mPositionMethodId = env->GetMethodID(mJavaClass, "position", "()J");
    assert(mPositionMethodId != nullptr);
    mSizeMethodId = env->GetMethodID(mJavaClass, "size", "()J");
    assert(mSizeMethodId != nullptr);
    mReadMethodId = env->GetMethodID(mJavaClass, "read", "(Ljava/nio/ByteBuffer;J)J");
    assert(mReadMethodId != nullptr);
}

InputStream::~InputStream()
{
    mJNIEnv->DeleteGlobalRef(mJavaStream);
}

HEIF::StreamInterface::offset_t InputStream::read(char* buffer, offset_t size)
{
    jobject byteBuffer = mJNIEnv->NewDirectByteBuffer((void*) buffer, size);
    jlong read         = mJNIEnv->CallLongMethod(mJavaStream, mReadMethodId, byteBuffer, size);
    return static_cast<HEIF::StreamInterface::offset_t>(read);
}

bool InputStream::absoluteSeek(HEIF::StreamInterface::offset_t offset)
{
    return mJNIEnv->CallBooleanMethod(mJavaStream, mSeekMethodId, offset) != 0;
}

HEIF::StreamInterface::offset_t InputStream::tell()
{
    return mJNIEnv->CallLongMethod(mJavaStream, mPositionMethodId);
}

HEIF::StreamInterface::offset_t InputStream::size()
{
    return mJNIEnv->CallLongMethod(mJavaStream, mSizeMethodId);
}
