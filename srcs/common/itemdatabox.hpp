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

#ifndef ITEMDATABOX_HPP
#define ITEMDATABOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"

/**
 * @brief The ItemDataBox 'idat' box contains item data as defined in the ISOBMFF standard. */
class ItemDataBox : public Box
{
public:
    ItemDataBox();
    virtual ~ItemDataBox() = default;

    /**
     * @brief read        Read data from the box.
     * @param destination Vector where read data is appended. The method will not clear the vector.
     * @param offset      Offset to the read data, bytes from the beginning of data[].
     * @param length      Count of read bytes.
     * @return True if data was copied successfully. False if requested amount of bytes was not available.
     */
    bool read(Vector<std::uint8_t>& destination, std::uint64_t offset, std::uint64_t length) const;

    /**
     * @brief read        Read data from the box.
     * @param destination Data pointer where data is copied
     * @param offset      Offset to the read data, bytes from the beginning of data[].
     * @param length      Count of read bytes.
     * @return True if data was copied successfully. False if requested amount of bytes was not available.
     */
    bool read(uint8_t* destination, const std::uint64_t offset, const std::uint64_t length) const;

    /**
     * @brief addData Add item data to the box.
     * @param data    The data to be added.
     * @return        Offset of the added data in the 'idat' box.
     */
    std::uint64_t addData(const Vector<std::uint8_t>& data);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses an ItemDataBox bitstream and fills in member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<std::uint8_t> mData;  ///< Data of stored items.
};

#endif
