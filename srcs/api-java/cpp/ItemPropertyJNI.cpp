/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include <jni.h>
#include "Helpers.h"
#include "ItemProperty.h"

#define CLASS_NAME ItemProperty

extern "C"
{
    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_ITEM_PROPERTY(nativeItem, self);
        jobject javaHandle = GET_JAVA_OBJECT(nativeItem);
        env->DeleteGlobalRef(javaHandle);
        setNativeHandle(env, self, 0);
        delete nativeItem;
    }
}