/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef MEDIADATABOX_HPP
#define MEDIADATABOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"

#include <cstdint>

/** @brief Media Data Box class. Extends from Box.
 *  @details 'mdat' box contains media data as defined in the ISOBMFF and HEIF standards. */
class MediaDataBox : public Box
{
public:
    MediaDataBox();
    ~MediaDataBox() = default;

    /** @brief Add data to the media data container.
     *  @details the data is inserted to the mData private member but not serialized until writeBox() is called.
     *  @param [in] srcData Media data to be inserted into the media data box.
     *  @return Byte offset of the  start location of the media data with respect to the media data box. */
    std::uint64_t addData(const Vector<std::uint8_t>& srcData);

    /** @brief Add data to the media data container.
     *  @details the data is inserted to the mData private member but not serialized until writeBox() is called.
     *  @param [in] char* buffer  Media data to be inserted into the media data box.
     *  @param [in] uint64_t bufferSize Size of Media data to be inserted into the media data box.
     *  @return Byte offset of the  start location of the media data with respect to the media data box. */
    std::uint64_t addData(const uint8_t* buffer, const uint64_t bufferSize);

    /** @brief Add a vector of NAL data to the media data container.
     *  @details Multiple NAL units can be written to the media data box at once by using this method.
     *           The data is inserted to the mData private member but not serialized until writeBox() is called.
     *  @param [in] srcData vector of NAL unit data */
    void addNalData(const Vector<Vector<std::uint8_t>>& srcData);

    /** @brief Add a single NAL unit data to the media data container.
     *  @details the data is inserted to the mData private member but not serialized until writeBox() is called.
     *  @param [in] srcData NAL unit data*/
    void addNalData(const Vector<std::uint8_t>& srcData);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] output Bitstream that contains the box data. */
    void writeBox(std::ofstream& output) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a MediaDataBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Update box size information in header data */
    void updateSize(ISOBMFF::BitStream& bitstr);

private:
    ISOBMFF::BitStream mHeaderData;         // header container
    std::list<Vector<uint8_t>> mMediaData;  // media data container
    uint64_t mTotalDataSize;                // total size of mMediaData vectors


    Vector<std::uint64_t> mDataOffsetArray;  // offsets relative to the beginning of the media data box
    Vector<std::uint64_t> mDataLengthArray;  // vector of data lengths which are inserted to the media box

    /** Returns the number of bytes in start code
     * start code consists of any number of zero bytes (0x00) followed by a one (0x01) byte */
    std::uint64_t findStartCode(const Vector<std::uint8_t>& srcData,
                                std::uint64_t initPos,
                                std::uint64_t& startCodePos);
};

#endif /* end of include guard: MEDIADATABOX_HPP */
