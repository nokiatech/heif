/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef TRACKFRAGMENTBASEMEDIADECODETIMEBOX_HPP
#define TRACKFRAGMENTBASEMEDIADECODETIMEBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief  Track Fragment Base Media Decode Time Box class
 * @details 'tfdt' box implementation as specified in the ISOBMFF specification.
 */
class TrackFragmentBaseMediaDecodeTimeBox : public FullBox
{
public:
    TrackFragmentBaseMediaDecodeTimeBox();
    ~TrackFragmentBaseMediaDecodeTimeBox() override = default;

    /** @brief Set Base Decode Time of the TrackFragmentBaseMediaDecodeTimeBox.
     *  @param [in] uint64_t baseMediaDecodeTime **/
    void setBaseMediaDecodeTime(const uint64_t baseMediaDecodeTime);

    /** @brief Get Base Decode Time of the TrackFragmentBaseMediaDecodeTimeBox.
     *  @return uint64_t Base Media Decode Time as specified in 8.8.12.1 of ISO/IEC 14496-12:2015(E)**/
    uint64_t getBaseMediaDecodeTime() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    uint64_t mBaseMediaDecodeTime;
};

#endif /* end of include guard: TRACKFRAGMENTBASEMEDIADECODETIMEBOX_HPP */
