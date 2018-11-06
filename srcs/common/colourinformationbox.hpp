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

#ifndef COLOURINFORMATIONBOX_HPP
#define COLOURINFORMATIONBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The ColourInformationBox 'colr' describes the type of colour information used. */
class ColourInformationBox : public Box
{
public:
    ColourInformationBox();
    virtual ~ColourInformationBox() = default;

    /**
     * Set the type of the colour information.
     * When the type is "nclx" ICC Profile, ICC profile
     * values are not used. If the type is "rICC" or "prof",
     * only ICC profile values are used.
     * @param type Type of the colour information.
     */
    void setColourType(const FourCCInt& type);

    /**
     * Get the type of the colour information.
     * When the type is "nclx" ICC Profile, ICC profile
     * values are not used. If the type is "rICC" or "prof",
     * only ICC profile values are used.
     * @return Type of the colour information.
     */
    FourCCInt getColourType() const;

    // Methods for "nclx" colour type
    void setColourPrimaries(uint16_t colourPrimaries);
    uint16_t getColourPrimaries() const;
    void setTransferCharacteristics(uint16_t transferCharacteristics);
    uint16_t getTransferCharacteristics() const;
    void setMatrixCoefficients(uint16_t matrixCoefficients);
    uint16_t getMatrixCoefficients() const;
    void setFullRangeFlag(bool fullRange);
    bool getFullRangeFlag() const;

    // Methods for "rICC" and "prof" colour types
    const Vector<std::uint8_t>& getIccProfile() const;
    void setIccProfile(const Vector<std::uint8_t>& iccProfile);

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    FourCCInt mColourType;
    uint16_t mColourPrimaries;
    uint16_t mTransferCharasteristics;
    uint16_t mMatrixCoefficients;
    bool mFullRangeFlag;
    Vector<uint8_t> mIccProfile;
};

#endif
