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
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::GridImageItem *nativeObject = new HEIFPP::GridImageItem(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD_ARG(void, resizeNative, jint width, jint height)
    {
        NATIVE_SELF;
        nativeSelf->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }

    JNI_METHOD(jint, getColumnCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->columns());
    }

    JNI_METHOD(jint, getRowCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->rows());
    }

    JNI_METHOD_ARG(jobject, getImageNative, jint column, jint row)
    {
        NATIVE_SELF;

        HEIFPP::ImageItem *imageItem = nullptr;
        CHECK_ERROR(nativeSelf->getImage(static_cast<uint32_t>(column), static_cast<uint32_t>(row), imageItem),
                    "getImage failed");
        return imageItem ? GET_JAVA_ITEM(imageItem) : nullptr;
    }

    JNI_METHOD_ARG(void, setImageNative, jint column, jint row, jobject image)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        CHECK_ERROR(nativeSelf->setImage(static_cast<uint32_t>(column), static_cast<uint32_t>(row), nativeImage),
                    "setImage failed");
    }

    JNI_METHOD_ARG(void, removeImageNative, jobject image)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->removeImage(nativeImage);
    }
}
