/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#ifndef MEDIADATABOX_HPP
#define MEDIADATABOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"

#include <cstdint>
#include <list>
#include <vector>

/** @brief Media Data Box class. Extends from Box.
 *  @details 'mdat' box contains media data as defined in the ISOBMFF and HEIF standards. **/
class MediaDataBox : public Box
{
public:
    MediaDataBox();
    ~MediaDataBox() = default;

    /** @brief Add data to the media data container.
     *  @details the data is inserted to the mData private member but not serialized until writeBox() is called.
     *  @param [in] srcData Media data to be inserted into the media data box.
     *  @return Byte offset of the  start location of the media data with respect to the media data box. **/
    size_t addData(const std::vector<std::uint8_t>& srcData);

    /** @brief Add a vector of NAL data to the media data container.
     *  @details Multiple NAL units can be written to the media data box at once by using this method.
     *  @details the data is inserted to the mData private member but not serialized until writeBox() is called.
     *  @param [in] srcData vector of NAL unit data **/
    void addListOfNalData(const std::list<std::vector<std::uint8_t>>& srcData);

    /** @brief Add a single NAL unit data to the media data container.
     *  @details the data is inserted to the mData private member but not serialized until writeBox() is called.
     *  @param [in] srcData NAL unit data**/
    void addNalData(const std::vector<std::uint8_t>& srcData);

    /** @brief Gets the reference to data in the media box.
     *  @details the data is in the form of vector of unsigned bytes.
     *  @return Reference to the vector of media data **/
    const std::vector<std::uint8_t>& getData() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a MediaDataBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    BitStream mData; // media data container
    std::vector<std::uint32_t> mDataOffsetArray;  // offsets relative to the beginning of the media data box
    std::vector<std::uint32_t> mDataLengthArray;  // vector of data lengths which are inserted to the media box

    /** Returns the number of bytes in start code
     * start code consists of any number of zero bytes (0x00) followed by a one (0x01) byte */
    unsigned int findStartCode(const std::vector<std::uint8_t>& srcData, unsigned int initPos, unsigned int& startCodePos);
};

#endif /* end of include guard: MEDIADATABOX_HPP */

