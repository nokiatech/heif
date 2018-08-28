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
#define CLASS_NAME GridImageItem

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::GridImageItem *nativeObject = new HEIFPP::GridImageItem(nativeHeif);
        nativeObject->setContext(static_cast<void*>(env->NewGlobalRef(self)));
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD_ARG(void, resizeNative, jint width, jint height)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, self);
        nativeHandle->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }

    JNI_METHOD(jint, getColumnCountNative)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->columns());
    }

    JNI_METHOD(jint, getRowCountNative)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->rows());
    }

    JNI_METHOD_ARG(jobject, getImageNative, jint column, jint row)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, self);

        HEIFPP::ImageItem *imageItem = nullptr;
        CHECK_ERROR(nativeHandle->getImage(static_cast<uint32_t>(column),
                                           static_cast<uint32_t>(row),
                                           imageItem),
                    "getImage failed");
        return imageItem ? GET_JAVA_ITEM(imageItem) : nullptr;
    }

    JNI_METHOD_ARG(void, setImageNative, jint column, jint row, jobject image)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        CHECK_ERROR(nativeHandle->setImage(static_cast<uint32_t>(column),
                                           static_cast<uint32_t>(row),
                                           nativeImage), "setImage failed");
    }

    JNI_METHOD_ARG(void, removeImageNative, jobject image)
    {
        NATIVE_GRID_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->removeImage(nativeImage);
    }
}
