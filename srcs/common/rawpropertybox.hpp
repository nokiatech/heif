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

#ifndef RAWPROPERTYBOX_HPP
#define RAWPROPERTYBOX_HPP

#include <cstdint>
#include "bbox.hpp"
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** A class for creating and accessing custom type properties.
 *  The class inherits Box, but the actual property type decides whether
 *  it is a Property or Full Property.
 *  Access box content only as binary blob by using setData() and getData() methods.
 */
class RawPropertyBox : public Box
{
public:
    RawPropertyBox();
    ~RawPropertyBox() = default;

    /** @brief Get box data
     *  @return All data in the box, including headers. */
    const Vector<std::uint8_t>& getData() const;

    /** @brief Set raw box data
     *  @param [in] data Data in the box. All box data including header must be included. */
    void setData(const Vector<std::uint8_t>& data);

    /** Write box data to ISOBMFF::BitStream. */
    void writeBox(ISOBMFF::BitStream& bitstream) const;

    /** Read box data from ISOBMFF::BitStream. */
    void parseBox(ISOBMFF::BitStream& bitstream);

private:
    Vector<std::uint8_t> mData;  ///< Content of this box, including box headers
};

#endif /* RAWPROPERTYBOX_HPP */
