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

#ifndef BBOX_HPP
#define BBOX_HPP

#include "fourccint.hpp"
#include <cstdint>
#include <string>
#include <vector>

class BitStream;

/** @brief Generic Box class.
 *  @details ISOBMFF compliant Box definition. Other boxes may extend from Box class.
 *  @todo userType handling is currently not implemented.
 */
class Box
{
public:
    Box(FourCCInt boxType);
    virtual ~Box() = default;

    /** @brief Sets the size of the Box
     *  @param [in] size Byte size of the Box */
    void setSize(std::uint32_t size);

    /** @return size size of the Box in bytes*/
    std::uint32_t getSize() const;

    /** @brief Sets the 4CC type of the Box
     *  @param [in] type 4CC type code of the Box as a string */
    void setType(FourCCInt type);

    /** @return type 4CC type code of the Box as a string */
    FourCCInt getType() const;

    /** @brief Writes the Box structure as a bitstream.
     * This virtual method should be implemented by each class
     * that extends from Box and based on the relevant data structure
     * defined in ISOBMFF standard.
     * @param [in,out] bitstr After method call serialized the data
     *        structure of the box has been appended to the BitStream. */
    virtual void writeBox(BitStream& bitstr) = 0;

    /** @brief Parses the Box bitstream and fills in the Box data structure.
     * This virtual method should be implemented by each class
     * that extends from Box and based on the relevant data structure
     * defined in ISOBMFF standard.
     * @param [in,out] bitstr A Bitstream object that contains Box data stream.
     *        BitStream location is updated during the call as data is read. */
    virtual void parseBox(BitStream& bitstr) = 0;

protected:
    /** @brief Fills in the Box header data structure as defined in ISOBMFF standard.
     * @param [in,out] bitstr A BitStream object that contains Box data stream, where the header data structure
     *                        is appended. */
    void writeBoxHeader(BitStream& bitstr);

    /** @brief Parses the Box header data structure as defined in ISOBMFF standard.
     * @param [in,out] bitstr A BitStream object that contains Box data stream. BitStream internal pointers
     *                         are updated accordingly. */
    void parseBoxHeader(BitStream& bitstr);

    /** @brief Updates the overall byte size of the BitStream object
     * @param [in] bitstr A BitStream object that contains Box data stream
     * @param [out] bitstr Modified BitStream object. Box size is updated. */
    void updateSize(BitStream& bitstr);

    /** @brief Increases the overall byte size of the BitStream object by the input byte amount.
     * During this operation, largeSize is updated if necessary.
     * @param [in] sizeIncrease the amount of byte-size increase to be applied. */
    void increaseSize(std::uint32_t sizeIncrease);

private:
    std::uint32_t mSize;                   ///< Size of the Box.
    FourCCInt mType;                       ///< Type of the Box.
    std::vector<std::uint32_t> mLargeSize; ///< Large size of the Box

    /// When the box type is "uuid", user defined type to be used as extended Type.
    FourCCInt mUserType;

    /// Internal variable to keep track of the byte position for bitstream operations.
    std::uint32_t mStartLocation;
};

#endif /* end of include guard: BBOX_HPP */
