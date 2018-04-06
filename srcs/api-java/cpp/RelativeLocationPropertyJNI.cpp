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

#include <jni.h>
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_RelativeLocationProperty_##method_name
#include <DescriptiveProperty.h>
#include "Helpers.h"
extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::RelativeLocationProperty *nativeObject = new HEIFPP::RelativeLocationProperty(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(void, setHorizontalOffsetNative)(JNIEnv *env, jobject obj, jint offset)
    {
        NATIVE_RELATIVE_LOCATION_PROPERTY(nativeHandle, obj);
        nativeHandle->mRelativeLocation.horizontalOffset = offset;
    }

    JNI_METHOD(void, setVerticalOffsetNative)(JNIEnv *env, jobject obj, jint offset)
    {
        NATIVE_RELATIVE_LOCATION_PROPERTY(nativeHandle, obj);
        nativeHandle->mRelativeLocation.verticalOffset = offset;
    }

    JNI_METHOD(int, getHorizontalOffsetNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_RELATIVE_LOCATION_PROPERTY(nativeHandle, obj);
        return nativeHandle->mRelativeLocation.horizontalOffset;
    }

    JNI_METHOD(int, getVerticalOffsetNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_RELATIVE_LOCATION_PROPERTY(nativeHandle, obj);
        return nativeHandle->mRelativeLocation.verticalOffset;
    }
}
