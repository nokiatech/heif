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
#include "GridImageItem.h"
#include "Helpers.h"
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_GridImageItem_##method_name

extern "C"
{
    JNI_METHOD(jlong, createContextNative)
    (JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::Grid *nativeObject = new HEIFPP::Grid(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(void, resizeNative)(JNIEnv *env, jobject obj, jint width, jint height)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, obj);
        nativeHandle->resize(width, height);
    }

    JNI_METHOD(jint, getColumnCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->columns();
    }

    JNI_METHOD(jint, getRowCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->rows();
    }

    JNI_METHOD(jobject, getImageNative)(JNIEnv *env, jobject obj, jint column, jint row)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, obj);

        HEIFPP::ImageItem *imageItem = nullptr;
        CHECK_ERROR(nativeHandle->getImage(column, row, imageItem), "getImage failed");
        return imageItem ? GET_JAVA_ITEM(imageItem) : nullptr;
    }

    JNI_METHOD(void, setImageNative)(JNIEnv *env, jobject obj, jint column, jint row, jobject image)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        CHECK_ERROR(nativeHandle->setImage(column, row, nativeImage), "setImage failed");
    }

    JNI_METHOD(void, removeImageNative)(JNIEnv *env, jobject obj, jobject image)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->removeImage(nativeImage);
    }
}
