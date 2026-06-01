/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef PIXELINFORMATIONPROPERTY_HPP
#define PIXELINFORMATIONPROPERTY_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <utility>

namespace ISOBMFF
{
    class BitStream;
}

/** The PixelInformationProperty 'pixi' describes the number and bit depth of colour components of the associated image.
 */
class PixelInformationProperty : public FullBox
{
public:

    struct ChannelInfo{
        uint8_t BitsPerChannel;
        uint8_t ComponentFormat;
        bool SubSamplingFlag;
        bool ChannelLabelFlag;
        uint8_t SubSamplingType;
        uint8_t SubSamplingLocation;
        std::string ChannelLabel;
    };


    PixelInformationProperty();
    ~PixelInformationProperty() override = default;

    /**
     * Get values of bits per channel fields of the property. Length of the array is number of channels.
     * @return Bits per channel field values of the property.
     */
    const Vector<std::uint8_t>& getBitsPerChannels() const;

    /**
     * Set values of bits per channel fields of the property. Length of the array will bee the number of channels.
     * @param bitsPerChannel Bits per channel field values of the property.
     */
    void setBitsPerChannels(const Vector<std::uint8_t>& bitsPerChannel);

    /**
     * @brief Set number of channels
     * @param The number of channels
     */
    void setNumChannels(uint8_t);

    /**
     * @brief Get mNumChannels property
     * @return The number of channels as uint8_t
     */
    const uint8_t& getNumChannels() const;

    /**
     * @brief Sets the datatype indication of the channel
     * @param Vector containing a component format of a channel per index
     */
    void setComponentFormat(const uint8_t& idc, uint8_t);

    /**
     * @brief get mComponentFormat value
     * @return Vector containing component format of channel per index
     */
    uint8_t getComponentFormat(const uint8_t& idc) const;

    /**
     * @brief Sets the flag indicating that the channel has subsampling
     * @param Vector containing a bool for a channel per index
     */
    void setSubSamplingFlag(const uint8_t& idc, bool);

    /**
     * @brief get mSubSamplingFlag value
     * @return Vector containing flag value for subsampling per channel per index
     */
    bool getSubSamplingFlag(const uint8_t& idc) const;

    /**
     * @brief Sets the channel label flags
     * @param Vector containing a bool for a channel per index
     */
    void setChannelLabelFlag(const uint8_t& idc, bool);

    /**
     * @brief get mChannelLabelFlag value
     * @return Vector containing a bool indicating existence of a label
     *  per channel per index
     */
    bool getChannelLabelFlag(const uint8_t& idc) const;

    /**
     * @brief Sets the subsampling type for channels
     * @param Vector containing channels subsampling type per index
     */
    void setSubSamplingType(const uint8_t& idc, uint8_t);

    /**
     * @brief get mSubSamplingType value
     * @return Vector containing subsampling type for channel per index
     */
    uint8_t getSubSamplingType(const uint8_t& idc) const;

    /**
     * @brief Sets the indicator of position for subsampling
     *        Subsampling pixel aspect ratio setting (4:2:0..)
     * @param A vector containing an indicator value (0-5) for subsampling
     *          location per channel per index
     */
    void setSubSamplingLocation(const uint8_t& idc,uint8_t);

    /**
     * @brief get mSubSamplingLocation value
     * @return Vector containing value to indicate subsampling location
     */
    uint8_t getSubSamplingLocation(const uint8_t& idc) const;

    /**
     * @brief Sets the human readable descriptions for channels
     * @param Vector containing the description as a string per channel
     */
    void setChannelLabel(const uint8_t& idc, std::string);

    /**
     * @brief get mChannelLabel value
     * @return Vector containing string type label containing channels info
     */
    std::string getChannelLabel(const uint8_t& idc) const;

    /**
     * @brief Insert an entire channelInfoStructure to mChannelData
     */
    void setChannelData(const std::map<uint8_t,PixelInformationProperty::ChannelInfo>& chaninfo);

    const std::map<uint8_t,PixelInformationProperty::ChannelInfo> getChannelData() const;

    /** @brief Get information on chroma offset (subsampling) on vertical and horizontal
     *  @note Returns true per axis if chroma IS centered. Mini bitstream requires value if IS NOT centered
     *          -> Values need to be inverted!
     *  @return pair of bools, pair.first is Horizontal centered value, pair.second is Vertical centered value
     */
    const std::pair<bool,bool> getChromaCenterdness(const uint8_t& idc) const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;



private:
    uint8_t mNumChannels;
    Vector<std::uint8_t> mBitsPerChannel;  ///< Bits per each channel.


    /// @brief The new PIXI holds data per pre-defined channel
    /// Channel IDC hold the channel identifier, e.g. alpha or luma or chroma
    /// and that functions as the key value in this map
    ///@note 0: Unused, 1: Unspecified, 2: First color channel (e.g. monochrome, Y, R, C) 
    ///      3: Second colour channel (e.g. U, Cb, G, M), 4: Third colour channel (e.g. V, Cr, B, Y) 
    ///      5: Alpha, 6: Depth, 7: Fourth colour channel (e.g. K)
    std::map<uint8_t, PixelInformationProperty::ChannelInfo> mChannelData;
};

#endif
