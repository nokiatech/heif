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
#define CLASS_NAME DerivedImageItem

extern "C"
{
    JNI_METHOD(jint, getSourceImageCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSourceImageCount());
    }

    JNI_METHOD_ARG(jobject, getSourceImageNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeSelf->getSourceImage(static_cast<uint32_t>(index)));
    }
}
