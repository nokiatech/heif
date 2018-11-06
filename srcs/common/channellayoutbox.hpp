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

#ifndef CHANNELLAYOUTBOX_HPP
#define CHANNELLAYOUTBOX_HPP

#include <vector>
#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Channel Layout Box class
 * @details 'chnl' box implementation as specified in the ISOBMFF specification.
 */
class ChannelLayoutBox : public FullBox
{
public:
    ChannelLayoutBox();
    ChannelLayoutBox(const ChannelLayoutBox& box);
    virtual ~ChannelLayoutBox() = default;

    /// A helper for getting and setting class data channel layout
    struct ChannelLayout
    {
        std::uint8_t speakerPosition = 127;  // undefined / unknown
        std::int16_t azimuth         = 0;
        std::int8_t elevation        = 0;
    };

    /// @return vector of channel layouts as ChannelLayout structs
    const Vector<ChannelLayout>& getChannelLayouts() const;

    /** @brief Adds channel layout into box.
     *  @param [in] channelLayout ChannelLayoutBox::ChannelLayout struct */
    void addChannelLayout(ChannelLayout& channelLayout);

    /// @return structure of stream: Channels (1) or Objects (2)
    std::uint8_t getStreamStructure() const;

    /// @return defined layout (1) or speaker positions (0)
    std::uint8_t getDefinedLayout() const;

    /** @brief Sets Definedlayout into box.
     *  @param [in] definedLayout std::uint8_t as defined as ChannelConfiguration from ISO/IEC 23001-8 */
    void setDefinedLayout(std::uint8_t definedLayout);

    /// @return std::uint64_t omitted channels map
    std::uint64_t getOmittedChannelsMap() const;

    /** @brief Sets Omitted Channels Map into box.
     *  @param [in] omittedChannelsMap std::uint64_t Omitted channels map */
    void setOmittedChannelsMap(std::uint64_t omittedChannelsMap);

    /// @return std::uint8_t object count
    std::uint8_t getObjectCount() const;

    /** @brief Sets object count into box.
     *  @param [in] objectCount std::uint8_t Object count */
    void setObjectCount(std::uint8_t objectCount);

    /// @return std::uint16_t channel count
    std::uint16_t getChannelCount() const;

    /** @brief Sets channel count from sample entry for channel layout reading
     *  @param [in] audio channel count for channel layout */
    void setChannelCount(std::uint16_t channelCount);

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint8_t mStreamStructure;
    std::uint8_t mDefinedLayout;
    std::uint64_t mOmittedChannelsMap;
    std::uint8_t mObjectCount;
    std::uint16_t mChannelCount;
    Vector<ChannelLayoutBox::ChannelLayout> mChannelLayouts;
};

#endif /* end of include guard: CHANNELLAYOUTBOX_HPP */
