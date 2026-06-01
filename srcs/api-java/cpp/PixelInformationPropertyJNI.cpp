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
#include <string>

#include "DescriptiveProperty.h"
#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME PixelInformationProperty

namespace
{
    // Helper: find a ChannelEntry by channelIdc, return pointer or nullptr
    HEIF::PixelInformation::ChannelEntry* findChannelEntry(
        HEIF::PixelInformation& pixi, uint8_t idc)
    {
        for (size_t i = 0; i < pixi.ChannelData.size; ++i)
        {
            if (pixi.ChannelData[i].channelIdc == idc)
                return &pixi.ChannelData[i];
        }
        return nullptr;
    }

    // Helper: find or insert a ChannelEntry by channelIdc
    HEIF::PixelInformation::ChannelEntry& getOrInsertChannelEntry(
        HEIF::PixelInformation& pixi, uint8_t idc)
    {
        auto* existing = findChannelEntry(pixi, idc);
        if (existing)
            return *existing;
        // Grow the array by 1
        HEIF::Array<HEIF::PixelInformation::ChannelEntry> newArr(pixi.ChannelData.size + 1);
        for (size_t i = 0; i < pixi.ChannelData.size; ++i)
            newArr[i] = pixi.ChannelData[i];
        newArr[pixi.ChannelData.size].channelIdc = idc;
        newArr[pixi.ChannelData.size].info = {};
        pixi.ChannelData = newArr;
        return pixi.ChannelData[pixi.ChannelData.size - 1];
    }
}  // namespace

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::PixelInformationProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "PixelInformationProperty.createContextNative")
    }

    JNI_METHOD(jint, getNumChannelsNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mPixelInformation.numChannels);
            }
        JNI_CATCH_RETURN(0, "PixelInformationProperty.getNumChannelsNative")
    }

    JNI_METHOD(jobject, getBitsPerChannelNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        if (nativeSelf->mPixelInformation.bitsPerChannel.size == 0)
        {
            return nullptr;
        }
        return env->NewDirectByteBuffer(nativeSelf->mPixelInformation.bitsPerChannel.elements,
                                        static_cast<jlong>(nativeSelf->mPixelInformation.bitsPerChannel.size));
            }
        JNI_CATCH_RETURN(nullptr, "PixelInformationProperty.getBitsPerChannelNative")
    }

    JNI_METHOD_ARG(void, setBitsPerChannelNative, jbyteArray data)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData = env->GetByteArrayElements(data, nullptr);
        auto dataSize     = static_cast<uint32_t>(env->GetArrayLength(data));

        nativeSelf->mPixelInformation.numChannels    = static_cast<uint8_t>(dataSize);
        nativeSelf->mPixelInformation.bitsPerChannel = HEIF::Array<uint8_t>(dataSize);
        std::memcpy(nativeSelf->mPixelInformation.bitsPerChannel.elements, nativeData, dataSize);
        env->ReleaseByteArrayElements(data, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("PixelInformationProperty.setBitsPerChannelNative")
    }

    JNI_METHOD_ARG(jint, getComponentFormatNative, jint channelIndex)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        auto* entry = findChannelEntry(nativeSelf->mPixelInformation, idx);
        if (entry)
        {
            return static_cast<jint>(entry->info.ComponentFormat);
        }
        return 0;
            }
        JNI_CATCH_RETURN(0, "PixelInformationProperty.getComponentFormatNative")
    }

    JNI_METHOD_ARG(void, setComponentFormatNative, jint channelIndex, jint format)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        getOrInsertChannelEntry(nativeSelf->mPixelInformation, idx).info.ComponentFormat = static_cast<uint8_t>(format);
            }
        JNI_CATCH_RETURN_VOID("PixelInformationProperty.setComponentFormatNative")
    }

    JNI_METHOD_ARG(jboolean, getSubSamplingFlagNative, jint channelIndex)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        auto* entry = findChannelEntry(nativeSelf->mPixelInformation, idx);
        if (entry)
        {
            return static_cast<jboolean>(entry->info.SubSamplingFlag);
        }
        return JNI_FALSE;
            }
        JNI_CATCH_RETURN(JNI_FALSE, "PixelInformationProperty.getSubSamplingFlagNative")
    }

    JNI_METHOD_ARG(void, setSubSamplingFlagNative, jint channelIndex, jboolean flag)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        getOrInsertChannelEntry(nativeSelf->mPixelInformation, idx).info.SubSamplingFlag = (flag == JNI_TRUE);
            }
        JNI_CATCH_RETURN_VOID("PixelInformationProperty.setSubSamplingFlagNative")
    }

    JNI_METHOD_ARG(jboolean, getChannelLabelFlagNative, jint channelIndex)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        auto* entry = findChannelEntry(nativeSelf->mPixelInformation, idx);
        if (entry)
        {
            return static_cast<jboolean>(entry->info.ChannelLabelFlag);
        }
        return JNI_FALSE;
            }
        JNI_CATCH_RETURN(JNI_FALSE, "PixelInformationProperty.getChannelLabelFlagNative")
    }

    JNI_METHOD_ARG(void, setChannelLabelFlagNative, jint channelIndex, jboolean flag)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        getOrInsertChannelEntry(nativeSelf->mPixelInformation, idx).info.ChannelLabelFlag = (flag == JNI_TRUE);
            }
        JNI_CATCH_RETURN_VOID("PixelInformationProperty.setChannelLabelFlagNative")
    }

    JNI_METHOD_ARG(jint, getSubSamplingTypeNative, jint channelIndex)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        auto* entry = findChannelEntry(nativeSelf->mPixelInformation, idx);
        if (entry)
        {
            return static_cast<jint>(entry->info.SubSamplingType);
        }
        return 0;
            }
        JNI_CATCH_RETURN(0, "PixelInformationProperty.getSubSamplingTypeNative")
    }

    JNI_METHOD_ARG(void, setSubSamplingTypeNative, jint channelIndex, jint type)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        getOrInsertChannelEntry(nativeSelf->mPixelInformation, idx).info.SubSamplingType = static_cast<uint8_t>(type);
            }
        JNI_CATCH_RETURN_VOID("PixelInformationProperty.setSubSamplingTypeNative")
    }

    JNI_METHOD_ARG(jint, getSubSamplingLocationNative, jint channelIndex)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        auto* entry = findChannelEntry(nativeSelf->mPixelInformation, idx);
        if (entry)
        {
            return static_cast<jint>(entry->info.SubSamplingLocation);
        }
        return 0;
            }
        JNI_CATCH_RETURN(0, "PixelInformationProperty.getSubSamplingLocationNative")
    }

    JNI_METHOD_ARG(void, setSubSamplingLocationNative, jint channelIndex, jint location)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        getOrInsertChannelEntry(nativeSelf->mPixelInformation, idx).info.SubSamplingLocation = static_cast<uint8_t>(location);
            }
        JNI_CATCH_RETURN_VOID("PixelInformationProperty.setSubSamplingLocationNative")
    }

    JNI_METHOD_ARG(jstring, getChannelLabelNative, jint channelIndex)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx = static_cast<uint8_t>(channelIndex);
        auto* entry = findChannelEntry(nativeSelf->mPixelInformation, idx);
        if (entry && entry->info.ChannelLabel.size > 0)
        {
            std::string label(entry->info.ChannelLabel.begin(), entry->info.ChannelLabel.end());
            return env->NewStringUTF(label.c_str());
        }
        return env->NewStringUTF("");
            }
        JNI_CATCH_RETURN(nullptr, "PixelInformationProperty.getChannelLabelNative")
    }

    JNI_METHOD_ARG(void, setChannelLabelNative, jint channelIndex, jstring label)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        auto idx            = static_cast<uint8_t>(channelIndex);
        const char* cLabel  = env->GetStringUTFChars(label, nullptr);
        auto len = std::strlen(cLabel);
        auto& chanEntry = getOrInsertChannelEntry(nativeSelf->mPixelInformation, idx);
        chanEntry.info.ChannelLabel = HEIF::Array<char>(len);
        std::memcpy(chanEntry.info.ChannelLabel.elements, cLabel, len);
        env->ReleaseStringUTFChars(label, cLabel);
            }
        JNI_CATCH_RETURN_VOID("PixelInformationProperty.setChannelLabelNative")
    }
}
