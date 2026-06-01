/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 *
 */
#include "OutputStream.h"

#include "jni.h"
#include "jni_md.h"

// GetMethodID returns nullptr AND raises a pending Java exception (NoSuchMethodError)
// if the method is missing. The previous assert(...) calls were no-ops in release
// builds, which would let a null jmethodID slip through and crash the JVM later via
// SIGSEGV inside CallXxxMethod. We now rely on the pending Java exception surfacing
// back to the Java caller with a useful diagnostic.
OutputStream::OutputStream(JNIEnv* env, jobject javaStream)
    : mJNIEnv(env)
{
    mJavaStream = env->NewGlobalRef(javaStream);
    mJavaClass  = env->GetObjectClass(javaStream);

    mSeekMethodId     = env->GetMethodID(mJavaClass, "seek", "(J)V");
    mPositionMethodId = env->GetMethodID(mJavaClass, "position", "()J");
    mRemoveMethodId   = env->GetMethodID(mJavaClass, "clear", "()V");
    mWriteMethodId    = env->GetMethodID(mJavaClass, "write", "(Ljava/nio/ByteBuffer;J)V");
}

OutputStream::~OutputStream()
{
    mJNIEnv->DeleteGlobalRef(mJavaStream);
}

// After every Call*Method() that calls back into Java, we MUST check
// for a pending Java exception. See InputStream.cpp for the full
// rationale. If we ignore a pending exception, the next JNI call we
// make is undefined behavior. We just return early; the pending
// exception will surface on the Java side once we unwind.

void OutputStream::write(const void* buffer, std::uint64_t size)
{
    jobject byteBuffer = mJNIEnv->NewDirectByteBuffer(const_cast<void*>(buffer), static_cast<jlong>(size));
    mJNIEnv->CallVoidMethod(mJavaStream, mWriteMethodId, byteBuffer, size);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        // Pending Java exception -- bail without further JNI work.
        return;
    }
}

void OutputStream::seekp(std::uint64_t aPosition)
{
    mJNIEnv->CallVoidMethod(mJavaStream, mSeekMethodId, aPosition);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        return;
    }
}

std::uint64_t OutputStream::tellp()
{
    jlong pos = mJNIEnv->CallLongMethod(mJavaStream, mPositionMethodId);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        return 0;
    }
    return static_cast<std::uint64_t>(pos);
}

void OutputStream::remove()
{
    mJNIEnv->CallVoidMethod(mJavaStream, mRemoveMethodId);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        return;
    }
}
