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
 *
 *
 */
#include <jni.h>

#include "Helpers.h"
#include "VideoSample.h"

#define CLASS_NAME VideoSample

extern "C"
{
    JNI_METHOD(jint, getWidthNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getWidth());
    }

    JNI_METHOD(jint, getHeightNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getHeight());
    }
}
