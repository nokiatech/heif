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
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_RotateProperty_##method_name
#include "Helpers.h"
#include "TransformativeProperty.h"
extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::RotateProperty *nativeObject = new HEIFPP::RotateProperty(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(void, setRotationNative)(JNIEnv *env, jobject obj, jint rotation)
    {
        NATIVE_ROTATE_PROPERTY(nativeHandle, obj);
        nativeHandle->mRotate.angle = rotation;
    }

    JNI_METHOD(jint, getRotationNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_ROTATE_PROPERTY(nativeHandle, obj);
        return nativeHandle->mRotate.angle;
    }
}