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
 */

#include <jni.h>
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_ItemProperty_##method_name
#include "Helpers.h"
#include "ItemProperty.h"
extern "C"
{
    JNI_METHOD(void, destroyContextNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_ITEM_PROPERTY(nativeItem, obj);
        jobject javaHandle = (jobject) nativeItem->getContext();
        env->DeleteGlobalRef(javaHandle);
        setNativeHandle(env, obj, 0);
        delete nativeItem;
    }
}