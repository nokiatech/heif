/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef AUDIOSAMPLEENTRYBOX_HPP
#define AUDIOSAMPLEENTRYBOX_HPP

#include "bitstream.hpp"
#include "channellayoutbox.hpp"
#include "customallocator.hpp"
#include "sampleentrybox.hpp"
#include "samplingratebox.hpp"

/** @brief AudioSampleEntryBox (supports also v1) class. Extends from SampleEntryBox.
 *  @details This box contains information related to the audio samples of the track
 *  @details as defined in the ISOBMFF standard. */

class AudioSampleEntryBox : public SampleEntryBox
{
public:
    AudioSampleEntryBox(FourCCInt codingname);
    AudioSampleEntryBox(const AudioSampleEntryBox& box);
    virtual ~AudioSampleEntryBox() = default;

    /** @brief Set version field of the AudioSampleEntryBox (version 0 or 1)
    *  @param [in] version version field of the AudioSampleEntryBox */
    void setVersion(std::uint16_t version);

    /** @brief Get version field of the AudioSampleEntryBox (version 0 or 1)
    *  @returns version field of the AudioSampleEntryBox as unsigned 16 bit integer value */
    std::uint16_t getVersion() const;

    /** @brief Gets sample's sample size as defined in ISOBMFF
     *  @returns Sample's  */
    std::uint16_t getSampleSize() const;

    /** @brief Sets sample's sample size as defined in ISOBMFF
     *  @param [in]  sample's sample size */
    void setSampleSize(std::uint16_t height);

    /** @brief Gets sample's channel count as defined in ISOBMFF
     *  @returns Sample's  */
    std::uint16_t getChannelCount() const;

    /** @brief Sets sample's channel count as defined in ISOBMFF
     *  @param [in]  sample's channel count */
    void setChannelCount(std::uint16_t height);

    /** @brief Gets sample's sample rate as defined in ISOBMFF
     *  @returns Sample's sample rate */
    std::uint32_t getSampleRate() const;

    /** @brief Sets sample's sample rate as defined in ISOBMFF
     *  @param [in]  sample's sample rate */
    void setSampleRate(std::uint32_t height);

    /** @brief whether AudioSampleEntryBox has  ChannelLayoutBox
     *  @returns true if ChannelLayoutBox present, false otherwise */
    bool hasChannelLayoutBox();

    /** @brief Gets sample's ChannelLayoutBox as defined in ISOBMFF
     *  @returns Sample's ChannelLayoutBox */
    ChannelLayoutBox& getChannelLayoutBox();

    /** @brief Sets sample's ChannelLayoutBox as defined in ISOBMFF
     *   @param [in] channelLayoutBox Sample's ChannelLayoutBox */
    void setChannelLayoutBox(ChannelLayoutBox& channelLayoutBox);

    /** @brief whether AudioSampleEntryv1Box has SamplingRateBox
    *  @returns true if SamplingRateBox present, false otherwise */
    bool hasSamplingRateBox();

    /** @brief Gets sample's SamplingRateBox as defined in ISOBMFF
    *  @returns Sample's Sampling rate */
    SamplingRateBox& getSamplingRateBox();

    /** @brief Sets sample's SamplingRateBox as defined in ISOBMFF
    *   @param [in] samplingRateBox Sample's ChannelLayoutBox */
    void setSamplingRateBox(SamplingRateBox& samplingRateBox);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a AudioSampleEntryBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr) override;

    /** @brief Not implemented and returns a null pointer, but the other functionality of
     * this class is used by the tests. Usually one is expected to clone the objects that inherit
     * from this class, but if this changes, feel free to dig the old version of this function
     * from the version history.
     */
    virtual AudioSampleEntryBox* clone() const override;

    /** @brief Check if this sample entry is a visual sample
    *  @return FALSE */
    virtual bool isVisual() const override;
private:
    std::uint16_t mVersion;       ///< Version of box, either 0 or 1
    std::uint16_t mChannelCount;  ///< Number of channels 1 (mono) or 2 (stereo)
    std::uint16_t mSampleSize;    ///< In bits and takes default value of 16
    std::uint32_t mSampleRate;    ///< 32-bit field expressed as 16.16 fixed-point number (hi.lo)
    bool mHasChannelLayoutBox;
    bool mHasSamplingRateBox;
    ChannelLayoutBox mChannelLayoutBox;  // optional
    SamplingRateBox mSamplingRateBox;    // optional
};

#endif  // AUDIOSAMPLEENTRYBOX_HPP
