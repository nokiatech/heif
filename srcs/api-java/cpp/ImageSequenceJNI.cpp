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

#include <algorithm>
#include <cstring>

#include "Helpers.h"
#include "ImageSequence.h"


#define CLASS_NAME ImageSequence

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::ImageSequence(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jstring, getAuxiliaryTypeInfoNative)
    {
        NATIVE_SELF;
        const auto aux = nativeSelf->aux();
        if (aux == nullptr)
        {
            return nullptr;
        }
        return env->NewStringUTF(aux->auxType.elements);
    }

    JNI_METHOD_ARG(void, setAuxNative, jstring auxType)
    {
        NATIVE_SELF;
        const char* inCStr = env->GetStringUTFChars(auxType, nullptr);
        if (nullptr == inCStr)
        {
            nativeSelf->setAux(nullptr);
            return;
        }

        HEIF::AuxiliaryType aux;
        aux.auxType = HEIF::Array<char>(strlen(inCStr));
        std::copy_n(inCStr, strlen(inCStr), aux.auxType.elements);
        env->ReleaseStringUTFChars(auxType, inCStr);

        nativeSelf->setAux(&aux);
    }
}
