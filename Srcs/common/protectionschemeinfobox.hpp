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

#ifndef PROTECTIONSCHEMEINFOBOX_HPP
#define PROTECTIONSCHEMEINFOBOX_HPP

#include <cstdint>
#include <vector>

class BitStream;

/** Protection Scheme Information Box class
 *  @details 'sinf' box dummy implementation. Box structure is specified in the ISOBMFF specification.
 *  Structures of 'sinf' and contained boxes are not implemented. It is possible to set and get content as a binary blob. */
class ProtectionSchemeInfoBox
{
public:
    /** @brief Get box data
     *  @return All data in the box, including headers. */
    std::vector<std::uint8_t> getData() const;

    /** @brief Set box data
     *  @param [in] data Data in the 'sinf' ox. All box data including header must be included. */
    void setData(const std::vector<std::uint8_t>& data);

    /** Write box data to BitStream. */
    void writeBox(BitStream& bitstream);

    /** Read box data from BitStream. */
    void parseBox(BitStream& bitstream);

private:
    std::vector<std::uint8_t> mData; ///< Content of this box, including box headers
};

#endif /* PROTECTIONSCHEMEINFOBOX_HPP */
