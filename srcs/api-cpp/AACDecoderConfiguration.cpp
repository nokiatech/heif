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

#include "AACDecoderConfiguration.h"
#include <heifreader.h>
#include <heifwriter.h>
#include <cstring>

using namespace HEIFPP;

AACDecoderConfiguration::AACDecoderConfiguration(Heif* aHeif)
    : DecoderConfig(aHeif, HEIF::FourCC("mp4a"))
    , mProgramConfig()
    , mGaSpecific()
    , mAudioSpecificConfig()
{
}
AACDecoderConfiguration::AACDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType)
    : DecoderConfig(aHeif, aType)
    , mProgramConfig()
    , mGaSpecific()
    , mAudioSpecificConfig()
{
}
HEIF::ErrorCode AACDecoderConfiguration::convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                                          std::uint8_t*& aData,
                                                          std::uint32_t& aSize) const
{
    // TODO: should verify that only correct & valid packets exist in the aConfig array.
    aSize = 0;
    for (std::size_t i = 0; i < aConfig.size; i++)
    {
        if (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::AudioSpecificConfig)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        aSize += (std::uint32_t) aConfig[i].decSpecInfoData.size;
    }
    std::uint8_t* d = aData = new std::uint8_t[aSize];
    for (std::size_t i = 0; i < aConfig.size; i++)
    {
        std::memcpy(d, aConfig[i].decSpecInfoData.begin(), aConfig[i].decSpecInfoData.size);
        d += aConfig[i].decSpecInfoData.size;
    }
    return HEIF::ErrorCode::OK;
}
HEIF::ErrorCode AACDecoderConfiguration::convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize)
{
    mConfig.decoderSpecificInfo                    = HEIF::Array<HEIF::DecoderSpecificInfo>(1);
    mConfig.decoderSpecificInfo[0].decSpecInfoType = HEIF::DecoderSpecInfoType::AudioSpecificConfig;
    mConfig.decoderSpecificInfo[0].decSpecInfoData = aSize;
    std::memcpy(mConfig.decoderSpecificInfo[0].decSpecInfoData.elements, aData, aSize);
    return HEIF::ErrorCode::OK;
}

HEIF::ErrorCode AACDecoderConfiguration::setConfig(const std::uint8_t* aData, std::uint32_t aSize)
{
    delete[] mBuffer;
    mBuffer     = 0;
    mBufferSize = 0;
    mBuffer     = new std::uint8_t[aSize];
    std::memcpy(mBuffer, aData, aSize);
    mBufferSize = aSize;
    parse();
    return convertFromRawData(mBuffer, mBufferSize);
}
void AACDecoderConfiguration::getConfig(uint8_t*& aData, std::uint32_t& aSize) const
{
    aData = mBuffer;
    aSize = mBufferSize;
}

#define PARSE_UNSUPPORTED_OBJECT_TYPES 0
uint8_t AACDecoderConfiguration::GetAudioObjectType(BitStream& bs)
{
    std::uint8_t audioObjectType = (std::uint8_t) bs.getBits(5);
    if (audioObjectType == 31)
    {
        audioObjectType = 32 + (std::uint8_t) bs.getBits(6);
    }
    return audioObjectType;
}
bool AACDecoderConfiguration::program_config_element(BitStream& bs)
{
    std::uint32_t i;
    mProgramConfig.element_instance_tag = (std::uint8_t) bs.getBits(4);
    mProgramConfig.object_type = (std::uint8_t) bs.getBits(2);  // 0 AAC-Main 1 AAC-LC 2 AAC-SSR 3 AAC-LTP
    if (mProgramConfig.object_type != 2)
    {
        return false;
    }
    mProgramConfig.sampling_frequency_index   = (std::uint8_t) bs.getBits(4);
    mProgramConfig.num_front_channel_elements = (std::uint8_t) bs.getBits(4);
    mProgramConfig.num_side_channel_elements  = (std::uint8_t) bs.getBits(4);
    mProgramConfig.num_back_channel_elements  = (std::uint8_t) bs.getBits(4);
    mProgramConfig.num_lfe_channel_elements   = (std::uint8_t) bs.getBits(2);
    mProgramConfig.num_assoc_data_elements    = (std::uint8_t) bs.getBits(3);
    mProgramConfig.num_valid_cc_elements      = (std::uint8_t) bs.getBits(4);
    mProgramConfig.mono_mixdown_present       = (std::uint8_t) bs.getBits(1);
    if (mProgramConfig.mono_mixdown_present == 1)
    {
        mProgramConfig.mono_mixdown_element_number = (std::uint8_t) bs.getBits(4);
    }
    mProgramConfig.stereo_mixdown_present = (std::uint8_t) bs.getBits(1);
    if (mProgramConfig.stereo_mixdown_present == 1)
    {
        mProgramConfig.stereo_mixdown_element_number = (std::uint8_t) bs.getBits(4);
    }
    mProgramConfig.matrix_mixdown_idx_present = (std::uint8_t) bs.getBits(1);
    if (mProgramConfig.matrix_mixdown_idx_present == 1)
    {
        mProgramConfig.matrix_mixdown_idx     = (std::uint8_t) bs.getBits(2);
        mProgramConfig.pseudo_surround_enable = (std::uint8_t) bs.getBits(1);
    }
    for (i = 0; i < mProgramConfig.num_front_channel_elements; i++)
    {
        mProgramConfig.front_element_is_cpe[i]     = (std::uint8_t) bs.getBits(1);
        mProgramConfig.front_element_tag_select[i] = (std::uint8_t) bs.getBits(4);
    }
    for (i = 0; i < mProgramConfig.num_side_channel_elements; i++)
    {
        mProgramConfig.side_element_is_cpe[i]     = (std::uint8_t) bs.getBits(1);
        mProgramConfig.side_element_tag_select[i] = (std::uint8_t) bs.getBits(4);
    }
    for (i = 0; i < mProgramConfig.num_back_channel_elements; i++)
    {
        mProgramConfig.back_element_is_cpe[i]     = (std::uint8_t) bs.getBits(1);
        mProgramConfig.back_element_tag_select[i] = (std::uint8_t) bs.getBits(4);
    }
    for (i = 0; i < mProgramConfig.num_lfe_channel_elements; i++)
    {
        mProgramConfig.lfe_element_tag_select[i] = (std::uint8_t) bs.getBits(4);
    }
    for (i = 0; i < mProgramConfig.num_assoc_data_elements; i++)
    {
        mProgramConfig.assoc_data_element_tag_select[i] = (std::uint8_t) bs.getBits(4);
    }
    for (i = 0; i < mProgramConfig.num_valid_cc_elements; i++)
    {
        mProgramConfig.cc_element_is_ind_sw[i]        = (std::uint8_t) bs.getBits(1);
        mProgramConfig.valid_cc_element_tag_select[i] = (std::uint8_t) bs.getBits(4);
    }
    // byte_alignment(); //Note 1 : If called from within an AudioSpecificConfig(), this byte_alignment shall be
    // relative to the start of the AudioSpecificConfig().
    while (!bs.isByteAligned())
    {
        bs.getBits(1);
    }

    mProgramConfig.comment_field_bytes = (std::uint8_t) bs.getBits(8);
    for (i = 0; i < mProgramConfig.comment_field_bytes; i++)
    {
        mProgramConfig.comment_field_data[i] = (std::uint8_t) bs.getBits(8);
    }
    return true;
}

bool AACDecoderConfiguration::GASpecificConfig(BitStream& bs)
{
    mGaSpecific.frameLengthFlag    = (std::uint8_t) bs.getBits(1);
    mGaSpecific.dependsOnCoreCoder = (std::uint8_t) bs.getBits(1);
    if (mGaSpecific.dependsOnCoreCoder)
    {
        mGaSpecific.coreCoderDelay = (std::uint8_t) bs.getBits(14);
    }
    mGaSpecific.extensionFlag = (std::uint8_t) bs.getBits(1);
    if (mAudioSpecificConfig.channelConfiguration == 0)
    {
        if (!program_config_element(bs))
            return false;
    }
#if PARSE_UNSUPPORTED_OBJECT_TYPES
    if ((mAudioSpecificConfig.audioObjectType == 6) || (mAudioSpecificConfig.audioObjectType == 20))
    {
        mGaSpecific.layerNr = bs.getBits(3);
    }
#endif
    if (mGaSpecific.extensionFlag)
    {
#if PARSE_UNSUPPORTED_OBJECT_TYPES
        if (mAudioSpecificConfig.audioObjectType == 22)
        {
            mGaSpecific.numOfSubFrame = bs.getBits(5);
            mGaSpecific.layer_length  = bs.getBits(11);
        }
        if (mAudioSpecificConfig.audioObjectType == 17 || mAudioSpecificConfig.audioObjectType == 19 ||
            mAudioSpecificConfig.audioObjectType == 20 || mAudioSpecificConfig.audioObjectType == 23)
        {
            mGaSpecific.aacSectionDataResilienceFlag     = bs.getBits(1);
            mGaSpecific.aacScalefactorDataResilienceFlag = bs.getBits(1);
            mGaSpecific.aacSpectralDataResilienceFlag    = bs.getBits(1);
        }
#endif
        mGaSpecific.extensionFlag3 = (std::uint8_t) bs.getBits(1);
        if (mGaSpecific.extensionFlag3)
        {
            /* tbd in version 3 */
        }
    }
    return true;
}
bool AACDecoderConfiguration::parse()
{
    BitStream bs(mBuffer, mBufferSize);
    mProgramConfig                       = {};
    mGaSpecific                          = {};
    mAudioSpecificConfig                 = {};
    mAudioSpecificConfig.audioObjectType = GetAudioObjectType(bs);
#if PARSE_UNSUPPORTED_OBJECT_TYPES == 0
    if (mAudioSpecificConfig.audioObjectType != 2)  // We only parse/support AAC-LC
    {
        return false;
    }
#endif
    mAudioSpecificConfig.samplingFrequencyIndex = (std::uint8_t) bs.getBits(4);
    if (mAudioSpecificConfig.samplingFrequencyIndex == 15)
    {
        mAudioSpecificConfig.samplingFrequency = bs.getBits(24);
    }
    mAudioSpecificConfig.channelConfiguration = (std::uint8_t) bs.getBits(4);
#if PARSE_UNSUPPORTED_OBJECT_TYPES == 0
    if (mAudioSpecificConfig.channelConfiguration == 0)
    {
        return false;
    }
#endif
    mAudioSpecificConfig.sbrPresentFlag = false;
    mAudioSpecificConfig.psPresentFlag  = false;
#if PARSE_UNSUPPORTED_OBJECT_TYPES
    if ((mAudioSpecificConfig.audioObjectType == 5 || mAudioSpecificConfig.audioObjectType == 29))
    {
        mAudioSpecificConfig.extensionAudioObjectType = 5;
        mAudioSpecificConfig.sbrPresentFlag           = true;
        if (mAudioSpecificConfig.audioObjectType == 29)
        {
            mAudioSpecificConfig.psPresentFlag = true;
        }
        mAudioSpecificConfig.extensionSamplingFrequencyIndex = bs.getBits(4);
        if (mAudioSpecificConfig.extensionSamplingFrequencyIndex == 0xf)
        {
            mAudioSpecificConfig.extensionSamplingFrequency = bs.getBits(24);
        }
        mAudioSpecificConfig.audioObjectType = GetAudioObjectType(bs);
        if (mAudioSpecificConfig.audioObjectType == 22)
        {
            mAudioSpecificConfig.extensionChannelConfiguration = bs.getBits(4);
        }
    }
    else
#endif
    {
        mAudioSpecificConfig.extensionAudioObjectType = 0;
    }
    switch (mAudioSpecificConfig.audioObjectType)
    {
    case 2:
#if PARSE_UNSUPPORTED_OBJECT_TYPES
    case 1:
    case 3:
    case 4:
    case 6:
    case 7:
    case 17:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
#endif
    {
        if (!GASpecificConfig(bs))
        {
            // un-supported.
            return false;
        }
        break;
    }
#if PARSE_UNSUPPORTED_OBJECT_TYPES
    case 8:
        CelpSpecificConfig(bs);
        break;
    case 9:
        HvxcSpecificConfig(bs);
        break;
    case 12:
        TTSSpecificConfig(bs);
        break;
    case 13:
    case 14:
    case 15:
    case 16:
        StructuredAudioSpecificConfig(bs);
        break;
    case 24:
        ErrorResilientCelpSpecificConfig(bs);
        break;
    case 25:
        ErrorResilientHvxcSpecificConfig(bs);
        break;
    case 26:
    case 27:
        ParametricSpecificConfig(bs);
        break;
    case 28:
        SSCSpecificConfig(bs);
        break;
    case 30:
        mAudioSpecificConfig.sacPayloadEmbedding = bs.getBits(1);
        SpatialSpecificConfig(bs);
        break;
    case 32:
    case 33:
    case 34:
        MPEG_1_2_SpecificConfig(bs);
        break;
    case 35:
        DSTSpecificConfig(bs);
        break;
    case 36:
        mAudioSpecificConfig.fillBits = bs.getBits(5);  // bslbf
        ALSSpecificConfig(bs);
        break;
    case 37:
    case 38:
        SLSSpecificConfig(bs);
        break;
    case 39:
        ELDSpecificConfig(bs, channelConfiguration);
        break;
    case 40:
    case 41:
        SymbolicMusicSpecificConfig(bs);
        break;
#endif
    default:
        // unsupported object type.
        return false;
    }
#if PARSE_UNSUPPORTED_OBJECT_TYPES
    switch (audioObjectType)
    {
    case 17:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 39:
        mAudioSpecificConfig.epConfig = bs.getBits(2);  // bslbf
        if ((mAudioSpecificConfig.epConfig == 2 || mAudioSpecificConfig.epConfig == 3))
        {
            ErrorProtectionSpecificConfig(bs);
        }
        if (mAudioSpecificConfig.epConfig == 3)
        {
            mAudioSpecificConfig.directMapping = bs.getBits(1);  // bslbf
            if (!mAudioSpecificConfig.directMapping)
            { /* tbd */  // MISSING FROM SPEC?
            }
        }
    default:
    {
    }
    }
#endif
    if (mAudioSpecificConfig.extensionAudioObjectType != 5 && bs.bits_to_decode() >= 16)
    {
        mAudioSpecificConfig.syncExtensionType = (std::uint16_t) bs.getBits(11);  // bslbf
        if (mAudioSpecificConfig.syncExtensionType == 0x2b7)
        {
            mAudioSpecificConfig.extensionAudioObjectType = GetAudioObjectType(bs);
            if (mAudioSpecificConfig.extensionAudioObjectType == 5)
            {
                mAudioSpecificConfig.sbrPresentFlag = bs.getBits(1) != 0;
                if (mAudioSpecificConfig.sbrPresentFlag == 1)
                {
                    mAudioSpecificConfig.extensionSamplingFrequencyIndex = (std::uint8_t) bs.getBits(4);
                    if (mAudioSpecificConfig.extensionSamplingFrequencyIndex == 0xf)
                    {
                        mAudioSpecificConfig.extensionSamplingFrequency = bs.getBits(24);
                    }
                    if (bs.bits_to_decode() >= 12)
                    {
                        mAudioSpecificConfig.syncExtensionType = (std::uint16_t) bs.getBits(11);  // bslbf
                        if (mAudioSpecificConfig.syncExtensionType == 0x548)
                        {
                            mAudioSpecificConfig.psPresentFlag = bs.getBits(1) != 0;  // uimsbf
                        }
                    }
                }
            }
            if (mAudioSpecificConfig.extensionAudioObjectType == 22)
            {
                mAudioSpecificConfig.sbrPresentFlag = bs.getBits(1) != 0;
                if (mAudioSpecificConfig.sbrPresentFlag == 1)
                {
                    mAudioSpecificConfig.extensionSamplingFrequencyIndex = (std::uint8_t) bs.getBits(4);
                    if (mAudioSpecificConfig.extensionSamplingFrequencyIndex == 0xf)
                    {
                        mAudioSpecificConfig.extensionSamplingFrequency = bs.getBits(24);
                    }
                }
                mAudioSpecificConfig.extensionChannelConfiguration = (std::uint8_t) bs.getBits(4);
            }
        }
    }

    return true;
}

uint32_t AACDecoderConfiguration::getSampleRate()
{
    std::uint32_t sampleRate = 0;
    switch (mAudioSpecificConfig.samplingFrequencyIndex)
    {
    case 0:
        sampleRate = 96000;
        break;
    case 1:
        sampleRate = 88200;
        break;
    case 2:
        sampleRate = 64000;
        break;
    case 3:
        sampleRate = 48000;
        break;
    case 4:
        sampleRate = 44100;
        break;
    case 5:
        sampleRate = 32000;
        break;
    case 6:
        sampleRate = 24000;
        break;
    case 7:
        sampleRate = 22050;
        break;
    case 8:
        sampleRate = 16000;
        break;
    case 9:
        sampleRate = 12000;
        break;
    case 10:
        sampleRate = 11025;
        break;
    case 11:
        sampleRate = 8000;
        break;
    case 12:
        sampleRate = 7350;
        break;
    case 13:
    case 14:
        // Reserved
        break;
    case 15:
    {
        sampleRate = mAudioSpecificConfig.samplingFrequency;
        break;
    }
    }
    return sampleRate;
}
uint8_t AACDecoderConfiguration::getChannels()
{
    std::uint8_t channels = 0;
    switch (mAudioSpecificConfig.channelConfiguration)
    {
    case 0:
        // TODO: AudioObjectTyped specific config..
        // not sure about this.
        return mProgramConfig.num_back_channel_elements + mProgramConfig.num_front_channel_elements +
               mProgramConfig.num_lfe_channel_elements + mProgramConfig.num_side_channel_elements;
        break;
    case 1:
        // SPEAKER_FRONT_CENTER);
        channels = 1;
        break;
    case 2:
        // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT
        channels = 2;
        break;
    case 3:
        // SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT
        channels = 3;
        break;
    case 4:
        // SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER
        channels = 4;
        break;
    case 5:
        // SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT |
        // SPEAKER_BACK_RIGHT
        channels = 5;
        break;
    case 6:
        // SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT |
        // SPEAKER_BACK_RIGHT | SPEAKER_LOW_FREQUENCY
        channels = 6;
        break;
    case 7:
        // SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT |
        // SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_LOW_FREQUENCY;
        channels = 8;
        break;
    default:
        // Reserved
        break;
    }

    return channels;
}
