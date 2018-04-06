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
#include "Helpers.h"
#include "IdentityImageItem.h"
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_IdentityImageItem_##method_name

extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::Identity *nativeObject = new HEIFPP::Identity(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(jobject, getImageNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IDENTITY_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItem(env, getJavaHEIF(env, obj), nativeHandle->getImage());
    }

    JNI_METHOD(void, setImageNative)(JNIEnv *env, jobject obj, jobject image)
    {
        NATIVE_IDENTITY_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->setImage(nativeImage);
    }
}