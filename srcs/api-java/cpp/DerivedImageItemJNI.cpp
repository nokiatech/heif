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
#include "DerivedImageItem.h"
#include "Helpers.h"
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_DerivedImageItem_##method_name

extern "C"
{
    JNI_METHOD(jint, getSourceImageCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_DERIVED_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->getSourceImageCount();
    }

    JNI_METHOD(jobject, getSourceImageNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_DERIVED_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItem(env, getJavaHEIF(env, obj), nativeHandle->getSourceImage(index));
    }
}
