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

#ifndef BBOX_HPP
#define BBOX_HPP

#include <cstdint>
#include "customallocator.hpp"
#include "fourccint.hpp"

#define MP4VR_ABSOLUTE_MAX_SAMPLE_COUNT (1 << 22)  // 4 194 304  (more than day worth of 48hz samples)

namespace ISOBMFF
{
    class BitStream;
}

/** @brief Generic Box class.
 *  @details ISOBMFF compliant Box definition. Other boxes may extend from Box class.
 *  @todo userType handling is currently not implemented.
 */
class Box
{
public:
    /** @brief Sets the 4CC type of the Box
     *  @param [in] type 4CC type code of the Box as a string */
    void setType(FourCCInt type);

    /** @return type 4CC type code of the Box as a string */
    FourCCInt getType() const;

    /**
     * @brief setLargeSize Use 64-bit size field instead of 32-bit for the box.
     * This must not be called already after starting to serialize the box with
     * writeBoxHeader() or writeFullBoxHeader().
     */
    void setLargeSize();

    /**
    * @brief getLargeSize Whether 64-bit size field (true) instead of 32-bit for the box.
    */
    bool getLargeSize() const;

    /** @brief Writes the Box structure as a bitstream.
     * This virtual method should be implemented by each class
     * that extends from Box and based on the relevant data structure
     * defined in ISOBMFF standard.
     * @param [in,out] bitstr After method call serialized the data
     *        structure of the box has been appended to the ISOBMFF::BitStream. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const = 0;

    /** @brief Parses the Box bitstream and fills in the Box data structure.
     * This virtual method should be implemented by each class
     * that extends from Box and based on the relevant data structure
     * defined in ISOBMFF standard.
     * @param [in,out] bitstr A Bitstream object that contains Box data stream.
     *        ISOBMFF::BitStream location is updated during the call as data is read. */
    virtual void parseBox(ISOBMFF::BitStream& bitstr) = 0;

protected:
    Box(FourCCInt boxType);
    virtual ~Box() = default;

    /** @brief Sets the size of the Box
     *  @param [in] size Byte size of the Box */
    void setSize(std::uint64_t size);

    /** @return size size of the Box in bytes*/
    std::uint64_t getSize() const;

    /** @brief Sets usertype to given value and sets type to 'uuid'
     *  @param [in] Vector containing 16 byte UUID code of usertype.
     * */
    void setUserType(const Vector<uint8_t>& usertype);

    /** @brief Gets usertype of 'uuid' type box.
    *  @return Vector containing 16 byte UUID code of usertype.
    * */
    const Vector<uint8_t>& getUserType() const;

    /** @brief Fills in the Box header data structure as defined in ISOBMFF standard.
     * @param [in,out] bitstr A ISOBMFF::BitStream object that contains Box data stream, where the header data structure
     *                        is appended. */
    void writeBoxHeader(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses the Box header data structure as defined in ISOBMFF standard.
     * @param [in,out] bitstr A ISOBMFF::BitStream object that contains Box data stream. ISOBMFF::BitStream internal pointers
     *                         are updated accordingly. */
    void parseBoxHeader(ISOBMFF::BitStream& bitstr);

    /** @brief Updates the overall byte size of the ISOBMFF::BitStream object
     * @param [in] bitstr A ISOBMFF::BitStream object that contains Box data stream
     * @param [out] bitstr Modified ISOBMFF::BitStream object. Box size is updated. */
    void updateSize(ISOBMFF::BitStream& bitstr) const;

private:
    /// @brief size of the Box
    mutable std::uint64_t mSize;
    /// @brief type of the Box
    FourCCInt mType;
    /// @brief When the box type is "uuid", user defined type to be used as extended Type.
    Vector<std::uint8_t> mUserType;
    /// @brief Internal variable to keep track of the byte position for bitstream operations.
    mutable std::uint64_t mStartLocation;
    /// @brief Use 64-bit size field instead of default 32-bit.
    bool mLargeSize;
};

#endif /* end of include guard: BBOX_HPP */
