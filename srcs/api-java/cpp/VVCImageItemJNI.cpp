/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 *
 *
 */

#include <VvcCodedImageItem.h>
#include <jni.h>

#include "Heif.h"
#include "Helpers.h"
#include "jni_md.h"

#define CLASS_NAME VVCImageItem

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = nativeHeif->create<HEIFPP::VVCCodedImageItem>();
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "VVCImageItem.createContextNative")
    }
}
