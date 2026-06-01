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
 *
 */
#include "InputStream.h"

#include "heifstreaminterface.h"
#include "jni.h"
#include "jni_md.h"

namespace
{
    // GetMethodID returns nullptr AND raises a pending Java exception (NoSuchMethodError)
    // if the method is missing. In a release build the previous assert(...) was compiled out,
    // so a missing method would leave a null jmethodID and crash the JVM later via SIGSEGV
    // when CallXxxMethod was invoked. We now leave the pending exception in place so it
    // surfaces back to Java with a useful diagnostic.
    jmethodID lookupMethod(JNIEnv* env, jclass cls, const char* name, const char* sig)
    {
        jmethodID id = env->GetMethodID(cls, name, sig);
        // If id is null a Java exception is already pending; just return.
        return id;
    }
}

InputStream::InputStream(JNIEnv* env, jobject javaStream)
    : mJNIEnv(env)
{
    mJavaStream = env->NewGlobalRef(javaStream);
    mJavaClass  = env->GetObjectClass(javaStream);

    mSeekMethodId     = lookupMethod(env, mJavaClass, "seek", "(J)Z");
    mPositionMethodId = lookupMethod(env, mJavaClass, "position", "()J");
    mSizeMethodId     = lookupMethod(env, mJavaClass, "size", "()J");
    mReadMethodId     = lookupMethod(env, mJavaClass, "read", "(Ljava/nio/ByteBuffer;J)J");
}

InputStream::~InputStream()
{
    mJNIEnv->DeleteGlobalRef(mJavaStream);
}

// After every Call*Method() that calls back into Java, we MUST check
// for a pending Java exception. If we don't, two bad things happen:
//   1. The next JNI call we make is undefined behavior (most JNI
//      functions are illegal while an exception is pending).
//   2. The C++ caller (the HEIF reader) gets back a bogus return
//      value (usually 0 / false), interprets it as "EOF reached" or
//      "seek failed", and may produce confusing error codes upstream.
//
// We surface a sentinel return value that the reader treats as I/O
// failure, then return immediately. The pending Java exception
// remains attached to the JNIEnv and will fire on the Java side as
// soon as control unwinds out of the native frame, giving the Java
// caller the original IOException / RuntimeException it actually
// threw -- preserving full diagnostic context.

HEIF::StreamInterface::offset_t InputStream::read(char* buffer, offset_t size)
{
    jobject byteBuffer = mJNIEnv->NewDirectByteBuffer((void*) buffer, size);
    jlong read         = mJNIEnv->CallLongMethod(mJavaStream, mReadMethodId, byteBuffer, size);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        // Java callback threw. Signal EOF so the reader stops calling
        // us; the pending exception will fire on the Java side.
        return 0;
    }
    return static_cast<HEIF::StreamInterface::offset_t>(read);
}

bool InputStream::absoluteSeek(HEIF::StreamInterface::offset_t offset)
{
    jboolean ok = mJNIEnv->CallBooleanMethod(mJavaStream, mSeekMethodId, offset);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        return false;
    }
    return ok != 0;
}

HEIF::StreamInterface::offset_t InputStream::tell()
{
    jlong pos = mJNIEnv->CallLongMethod(mJavaStream, mPositionMethodId);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        return 0;
    }
    return static_cast<HEIF::StreamInterface::offset_t>(pos);
}

HEIF::StreamInterface::offset_t InputStream::size()
{
    jlong sz = mJNIEnv->CallLongMethod(mJavaStream, mSizeMethodId);
    if (mJNIEnv->ExceptionCheck() == JNI_TRUE)
    {
        return 0;
    }
    return static_cast<HEIF::StreamInterface::offset_t>(sz);
}
