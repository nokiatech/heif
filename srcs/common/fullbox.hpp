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

#ifndef FULLBOX_HPP
#define FULLBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"

#include <cstdint>

/** @brief Full Box class. Extends from Box.
 *  @details 'ftyp' box implementation as specified in the ISOBMFF specification
 *  @details Major and Compatible Brands List are present in this box.
 */
class FullBox : public Box
{
public:
    FullBox(FourCCInt boxType, std::uint8_t version, std::uint32_t flags = 0);
    virtual ~FullBox() = default;

    /** @brief Set version field of the Full Box header
     *  @param [in] version version field of the full box header*/
    void setVersion(std::uint8_t version);

    /** @brief Get version field of the Full Box header
     *  @returns version field of the full box as unsigned 8 bit integer value*/
    std::uint8_t getVersion() const;

    /** @brief Set flags of the Full Box Header
     *  @param [in] flags flags field of the full box header as unsigned 32 bit integer*/
    void setFlags(std::uint32_t flags);

    /** @brief Get flags of the Full Box Header
     *  @returns flags field of the full box header as unsigned 32 bit integer*/
    std::uint32_t getFlags() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const = 0;

    /** @brief Parses a Full box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr) = 0;

    /** @brief Parses a bitstream that contains a full Box Header and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the full box header data */
    void parseFullBoxHeader(ISOBMFF::BitStream& bitstr);

protected:
    /** @brief Creates the bitstream that represents the full box header
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeFullBoxHeader(ISOBMFF::BitStream& bitstr) const;

private:
    std::uint8_t mVersion;  // version field of the full box header
    std::uint32_t mFlags;   // Flags field of the full box header. Only 24 bits are used.
};

#endif /* end of include guard: FULLBOX_HPP */
