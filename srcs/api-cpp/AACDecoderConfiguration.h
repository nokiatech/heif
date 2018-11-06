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

#pragma once

#include <DecoderConfiguration.h>

namespace HEIFPP
{
    class AACDecoderConfiguration : public HEIFPP::DecoderConfig
    {
    public:
        AACDecoderConfiguration(Heif* aHeif);
        AACDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType);
        ~AACDecoderConfiguration() = default;

        /** Returns the whole configuration as a block
         * @param [out] data: Reference to where the data should be copied
         * @param [out] size: The size of the data copied */
        void getConfig(std::uint8_t*& data, std::uint32_t& size) const override;

        /** Sets the decoder data as a block
         * @param [in] data: Pointer to the data
         * @param [in] size: The size of the data */
        HEIF::ErrorCode setConfig(const std::uint8_t* data, std::uint32_t size) override;


        std::uint32_t getSampleRate();
        std::uint8_t getChannels();

    protected:
        HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                         std::uint8_t*& aData,
                                         std::uint32_t& aSize) const override;
        HEIF::ErrorCode convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize) override;

    private:
        uint8_t GetAudioObjectType(BitStream& bs);
        bool program_config_element(BitStream& bs);
        bool GASpecificConfig(BitStream& bs);

        // ProgramConfig
        struct ProgramConfig
        {
            uint8_t element_instance_tag, object_type, sampling_frequency_index, num_front_channel_elements,
                num_side_channel_elements, num_back_channel_elements, num_lfe_channel_elements, num_assoc_data_elements,
                num_valid_cc_elements, mono_mixdown_present;
            uint8_t mono_mixdown_element_number, stereo_mixdown_present, stereo_mixdown_element_number,
                matrix_mixdown_idx_present, matrix_mixdown_idx, pseudo_surround_enable;
            uint8_t front_element_is_cpe[16], front_element_tag_select[16];
            uint8_t side_element_is_cpe[16], side_element_tag_select[16];
            uint8_t back_element_is_cpe[16], back_element_tag_select[16];
            uint8_t lfe_element_tag_select[4];
            uint8_t assoc_data_element_tag_select[8];
            uint8_t cc_element_is_ind_sw[16], valid_cc_element_tag_select[16];
            uint8_t comment_field_bytes;
            uint8_t comment_field_data[256];
        } mProgramConfig;
        // GASpecific
        struct GASpecific
        {
            uint8_t frameLengthFlag, dependsOnCoreCoder, extensionFlag, extensionFlag3;
            uint8_t aacSectionDataResilienceFlag, aacScalefactorDataResilienceFlag, aacSpectralDataResilienceFlag;
            uint8_t numOfSubFrame, layerNr;
            uint32_t layer_length;
            uint32_t coreCoderDelay;
        } mGaSpecific;
        // main.
        struct AudioSpecificConfig
        {
            std::uint8_t audioObjectType, extensionAudioObjectType;
            std::uint8_t samplingFrequencyIndex, extensionSamplingFrequencyIndex;
            std::uint32_t samplingFrequency, extensionSamplingFrequency;
            std::uint8_t channelConfiguration, extensionChannelConfiguration;
            std::uint16_t syncExtensionType;
            bool sbrPresentFlag, psPresentFlag;
        } mAudioSpecificConfig;

        bool parse();
        AACDecoderConfiguration& operator=(const AACDecoderConfiguration&) = delete;
        AACDecoderConfiguration& operator=(AACDecoderConfiguration&&)      = delete;
        AACDecoderConfiguration(const AACDecoderConfiguration&)            = delete;
        AACDecoderConfiguration(AACDecoderConfiguration&&)                 = delete;
        AACDecoderConfiguration()                                          = delete;
    };

}  // namespace HEIFPP
