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
 */

#include <jni.h>

#include "Helpers.h"
#include "Track.h"

#define CLASS_NAME Track

extern "C"
{
    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
    }

    JNI_METHOD(jint, getSampleCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSampleCount());
    }

    JNI_METHOD_ARG(jobject, getSampleNative, jint index)
    {
        NATIVE_SELF;
        return getJavaSample(env, getJavaHEIF(env, self), nativeSelf->getSample(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, removeSampleNative, jobject sample)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);
        nativeSelf->removeSample(nativeSample);
    }

    JNI_METHOD(jint, getTimescaleNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTimescale());
    }

    JNI_METHOD_ARG(void, setTimescaleNative, jint timescale)
    {
        NATIVE_SELF;
        nativeSelf->setTimescale(static_cast<uint32_t>(timescale));
    }

    JNI_METHOD(jint, getThumbnailCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getThumbnailCount());
    }

    JNI_METHOD_ARG(jobject, getThumbnailNative, jint index)
    {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getThumbnail(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jint, getAuxiliaryCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getAuxCount());
    }

    JNI_METHOD_ARG(jobject, getAuxiliaryNative, jint index)
    {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getAux(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addAuxiliaryNative, jobject auxiliary)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, auxiliary);
        nativeSelf->addAux(nativeThumbnail);
    }

    JNI_METHOD_ARG(void, removeAuxiliaryNative, jobject auxiliary)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeAuxiliary, auxiliary);
        nativeSelf->removeAux(nativeAuxiliary);
    }

    JNI_METHOD_ARG(void, addThumbnailNative, jobject thumbnail)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, thumbnail);
        nativeSelf->addThumbnail(nativeThumbnail);
    }

    JNI_METHOD_ARG(void, removeThumbnailNative, jobject thumbnail)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, thumbnail);
        nativeSelf->removeThumbnail(nativeThumbnail);
    }

    JNI_METHOD(jobject, getAlternativeTrackGroupNative)
    {
        NATIVE_SELF;
        return getJavaAlternativeTrackGroup(env, getJavaHEIF(env, self), nativeSelf->getAlternativeTrackGroup());
    }

    JNI_METHOD(jint, getGroupCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getGroupCount());
    }

    JNI_METHOD_ARG(jobject, getGroupNative, jint index)
    {
        NATIVE_SELF;
        return getJavaEntityGroup(env, getJavaHEIF(env, self), nativeSelf->getGroup(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jboolean, hasInfiniteLoopPlaybackNative)
    {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->hasInfiniteLoopPlayback());
    }

    JNI_METHOD(jdouble, getDurationNative)
    {
        NATIVE_SELF;
        return static_cast<jdouble>(nativeSelf->getDuration());
    }

    JNI_METHOD(jint, getTimestampCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTimestampCount());
    }

    JNI_METHOD_ARG(jobject, getTimestampNative, jint index)
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

    JNI_METHOD_ARG(void, setEditListLoopingNative, jboolean isLooping)
    {
        NATIVE_SELF;
        nativeSelf->setEditListLooping(isLooping);
    }

    JNI_METHOD_ARG(void, setEditListRepetitionsNative, jdouble repetitions)
    {
        NATIVE_SELF;
        nativeSelf->setEditListRepetitions(repetitions);
    }

    JNI_METHOD_ARG(void, addEditListUnitNative, jobject editUnit)
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
        unit.mediaRateFraction  = mediaRateFraction;
        unit.mediaRateInteger   = mediaRateInteger;
        unit.editType           = HEIF::EditType(editTypeValue);
        nativeSelf->addEditListUnit(unit);
    }
}
