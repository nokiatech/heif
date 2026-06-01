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
 */

#include <jni.h>

#include "Helpers.h"
#include "Track.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <cstdint>

#define CLASS_NAME Track

extern "C"
{
    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
            }
        JNI_CATCH_RETURN_VOID("Track.destroyContextNative")
    }

    JNI_METHOD(jint, getSampleCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSampleCount());
            }
        JNI_CATCH_RETURN(0, "Track.getSampleCountNative")
    }

    JNI_METHOD_ARG(jobject, getSampleNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaSample(env, getJavaHEIF(env, self), nativeSelf->getSample(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Track.getSampleNative")
    }

    JNI_METHOD_ARG(void, removeSampleNative, jobject sample)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);
        nativeSelf->removeSample(nativeSample);
            }
        JNI_CATCH_RETURN_VOID("Track.removeSampleNative")
    }

    JNI_METHOD(jint, getTimescaleNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTimescale());
            }
        JNI_CATCH_RETURN(0, "Track.getTimescaleNative")
    }

    JNI_METHOD_ARG(void, setTimescaleNative, jint timescale)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setTimescale(static_cast<uint32_t>(timescale));
            }
        JNI_CATCH_RETURN_VOID("Track.setTimescaleNative")
    }

    JNI_METHOD(jint, getThumbnailCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getThumbnailCount());
            }
        JNI_CATCH_RETURN(0, "Track.getThumbnailCountNative")
    }

    JNI_METHOD_ARG(jobject, getThumbnailNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getThumbnail(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Track.getThumbnailNative")
    }

    JNI_METHOD(jint, getAuxiliaryCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getAuxCount());
            }
        JNI_CATCH_RETURN(0, "Track.getAuxiliaryCountNative")
    }

    JNI_METHOD_ARG(jobject, getAuxiliaryNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getAux(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Track.getAuxiliaryNative")
    }

    JNI_METHOD_ARG(void, addAuxiliaryNative, jobject auxiliary)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, auxiliary);
        nativeSelf->addAux(nativeThumbnail);
            }
        JNI_CATCH_RETURN_VOID("Track.addAuxiliaryNative")
    }

    JNI_METHOD_ARG(void, removeAuxiliaryNative, jobject auxiliary)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeAuxiliary, auxiliary);
        nativeSelf->removeAux(nativeAuxiliary);
            }
        JNI_CATCH_RETURN_VOID("Track.removeAuxiliaryNative")
    }

    JNI_METHOD_ARG(void, addThumbnailNative, jobject thumbnail)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, thumbnail);
        nativeSelf->addThumbnail(nativeThumbnail);
            }
        JNI_CATCH_RETURN_VOID("Track.addThumbnailNative")
    }

    JNI_METHOD_ARG(void, removeThumbnailNative, jobject thumbnail)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, thumbnail);
        nativeSelf->removeThumbnail(nativeThumbnail);
            }
        JNI_CATCH_RETURN_VOID("Track.removeThumbnailNative")
    }

    JNI_METHOD(jobject, getAlternativeTrackGroupNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaAlternativeTrackGroup(env, getJavaHEIF(env, self), nativeSelf->getAlternativeTrackGroup());
            }
        JNI_CATCH_RETURN(nullptr, "Track.getAlternativeTrackGroupNative")
    }

    JNI_METHOD(jint, getGroupCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getGroupCount());
            }
        JNI_CATCH_RETURN(0, "Track.getGroupCountNative")
    }

    JNI_METHOD_ARG(jobject, getGroupNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaEntityGroup(env, getJavaHEIF(env, self), nativeSelf->getGroup(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Track.getGroupNative")
    }

    JNI_METHOD(jboolean, hasInfiniteLoopPlaybackNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->hasInfiniteLoopPlayback());
            }
        JNI_CATCH_RETURN(JNI_FALSE, "Track.hasInfiniteLoopPlaybackNative")
    }

    JNI_METHOD(jdouble, getDurationNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return nativeSelf->getDuration();
            }
        JNI_CATCH_RETURN(0, "Track.getDurationNative")
    }

    JNI_METHOD(jint, getTimestampCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTimestampCount());
            }
        JNI_CATCH_RETURN(0, "Track.getTimestampCountNative")
    }

    JNI_METHOD_ARG(jobject, getTimestampNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        std::uint32_t sampleId;
        std::int64_t timeStamp;

        nativeSelf->getTimestamp(static_cast<uint32_t>(index), sampleId, timeStamp);

        jclass c = env->FindClass("com/nokia/heif/Track$TimeStamp");
        if (c == nullptr)
        {
            return nullptr;
        }
        jmethodID constructor   = env->GetMethodID(c, "<init>", "()V");
        jobject obj             = env->NewObject(c, constructor);
        jfieldID indexField     = env->GetFieldID(c, "sampleIndex", "I");
        jfieldID timeStampField = env->GetFieldID(c, "timeStamp", "J");

        env->SetIntField(obj, indexField, static_cast<jint>(sampleId));
        env->SetLongField(obj, timeStampField, timeStamp);

        return obj;
            }
        JNI_CATCH_RETURN(nullptr, "Track.getTimestampNative")
    }

    JNI_METHOD_ARG(void, setEditListLoopingNative, jboolean isLooping)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setEditListLooping(isLooping);
            }
        JNI_CATCH_RETURN_VOID("Track.setEditListLoopingNative")
    }

    JNI_METHOD_ARG(void, setEditListRepetitionsNative, jdouble repetitions)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setEditListRepetitions(repetitions);
            }
        JNI_CATCH_RETURN_VOID("Track.setEditListRepetitionsNative")
    }

    JNI_METHOD_ARG(void, addEditListUnitNative, jobject editUnit)
    {
        JNI_TRY
        {
        NATIVE_SELF;

        const jclass c = env->GetObjectClass(editUnit);
        if (c == nullptr)
        {
            return;
        }

        jfieldID fieldId               = env->GetFieldID(c, "mediaTimeInTrackTS", "J");
        const jlong mediaTimeInTrackTS = env->GetLongField(editUnit, fieldId);

        fieldId                 = env->GetFieldID(c, "durationInMovieTS", "J");
        jlong durationInMovieTS = env->GetLongField(editUnit, fieldId);

        fieldId               = env->GetFieldID(c, "mediaRateInteger", "I");
        jint mediaRateInteger = env->GetIntField(editUnit, fieldId);

        fieldId                = env->GetFieldID(c, "mediaRateFraction", "I");
        jint mediaRateFraction = env->GetIntField(editUnit, fieldId);

        fieldId              = env->GetFieldID(c, "editType", "Lcom/nokia/heif/Track$EditType;");
        jobject editTypeJava = env->GetObjectField(editUnit, fieldId);
        jmethodID editTypeGetValueMethod =
            env->GetMethodID(env->FindClass("com/nokia/heif/Track$EditType"), "ordinal", "()I");
        const jint editTypeValue = env->CallIntMethod(editTypeJava, editTypeGetValueMethod);

        HEIF::EditUnit unit;
        unit.durationInMovieTS  = static_cast<uint64_t>(durationInMovieTS);
        unit.mediaTimeInTrackTS = mediaTimeInTrackTS;
        unit.mediaRateFraction  = static_cast<int16_t>(mediaRateFraction);
        unit.mediaRateInteger   = static_cast<int16_t>(mediaRateInteger);
        unit.editType           = HEIF::EditType(editTypeValue);
        nativeSelf->addEditListUnit(unit);
            }
        JNI_CATCH_RETURN_VOID("Track.addEditListUnitNative")
    }
}
