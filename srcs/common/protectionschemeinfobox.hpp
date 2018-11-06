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

#ifndef PROTECTIONSCHEMEINFOBOX_HPP
#define PROTECTIONSCHEMEINFOBOX_HPP

#include <cstdint>
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** Protection Scheme Information Box class
 *  @details 'sinf' box dummy implementation. Box structure is specified in the ISOBMFF specification.
 *  Structures of 'sinf' and contained boxes are not implemented. It is possible to set and get content as a binary blob. */
class ProtectionSchemeInfoBox
{
public:
    /** @brief Get box data
     *  @return All data in the box, including headers. */
    const Vector<std::uint8_t>& getData() const;

    /** @brief Set box data
     *  @param [in] data Data in the 'sinf' ox. All box data including header must be included. */
    void setData(const Vector<std::uint8_t>& data);

    /** Write box data to ISOBMFF::BitStream. */
    void writeBox(ISOBMFF::BitStream& bitstream) const;

    /** Read box data from ISOBMFF::BitStream. */
    void parseBox(ISOBMFF::BitStream& bitstream);

private:
    Vector<std::uint8_t> mData;  ///< Content of this box, including box headers
};

#endif /* PROTECTIONSCHEMEINFOBOX_HPP */
