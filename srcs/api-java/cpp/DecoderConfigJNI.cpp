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

#include <jni.h>

#include <cstring>

#include "DecoderConfiguration.h"
#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME DecoderConfig

extern "C"
{
    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_DECODER_CONFIG(nativeItem, self);
        setNativeHandle(env, self, 0);
        delete nativeItem;
            }
        JNI_CATCH_RETURN_VOID("DecoderConfig.destroyContextNative")
    }

    JNI_METHOD_ARG(void, setConfigNative, jbyteArray configdata)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData = env->GetByteArrayElements(configdata, nullptr);
        nativeSelf->setConfig((uint8_t*) nativeData, static_cast<uint32_t>(env->GetArrayLength(configdata)));
        env->ReleaseByteArrayElements(configdata, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("DecoderConfig.setConfigNative")
    }

    JNI_METHOD(jobject, getConfigNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        const HEIF::Array<HEIF::DecoderSpecificInfo>& codecInfo = nativeSelf->getConfig();
        size_t totalSize                                        = 0;
        for (size_t index = 0; index < codecInfo.size; index++)
        {
            totalSize += codecInfo[index].decSpecInfoData.size;
        }
        auto* data        = new jbyte[totalSize];
        size_t writeIndex = 0;
        for (size_t index = 0; index < codecInfo.size; index++)
        {
            std::memcpy(data + writeIndex, codecInfo[index].decSpecInfoData.elements,
                        codecInfo[index].decSpecInfoData.size);
            writeIndex += codecInfo[index].decSpecInfoData.size;
        }
        jbyteArray output = env->NewByteArray(static_cast<jsize>(totalSize));
        env->SetByteArrayRegion(output, 0, static_cast<jsize>(totalSize), data);
        delete[] data;
        return output;
            }
        JNI_CATCH_RETURN(nullptr, "DecoderConfig.getConfigNative")
    }
}
