/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef FULLBOX_HPP
#define FULLBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"

#include <cstdint>

/** @brief Full Box class. Extends from Box.
 *  @details 'ftyp' box implementation as specified in the ISOBMFF specification
 *  @details Major and Compatible Brands List are present in this box.
 */
class FullBox: public Box
{
public:
    FullBox(FourCCInt boxType, std::uint8_t version, std::uint32_t flags = 0);
    virtual ~FullBox() = default;

    /** @brief Set version field of the Full Box header
     *  @param [in] version version field of the full box header**/
    void setVersion(std::uint8_t version);

    /** @brief Get version field of the Full Box header
     *  @returns version field of the full box as unsigned 8 bit integer value**/
    std::uint8_t getVersion() const;

    /** @brief Set flags of the Full Box Header
     *  @param [in] flags flags field of the full box header as unsigned 32 bit integer**/
    void setFlags(std::uint32_t flags);


    /** @brief Get flags of the Full Box Header
     *  @returns flags field of the full box header as unsigned 32 bit integer**/
    std::uint32_t getFlags() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr) = 0;

    /** @brief Parses a Full box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr) = 0;

    /** @brief Parses a bitstream that contains a full Box Header and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the full box header data */
    void parseFullBoxHeader(BitStream& bitstr);

protected:
    /** @brief Creates the bitstream that represents the full box header
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeFullBoxHeader(BitStream& bitstr);

private:
    std::uint8_t mVersion; // version field of the full box header
    std::uint32_t mFlags;  // Flags field of the full box header. Only 24 bits are used.
};

#endif /* end of include guard: FULLBOX_HPP */
