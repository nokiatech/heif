/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef MASTERINGDISPLAYCOLOURVOLUMEBOX_HPP
#define MASTERINGDISPLAYCOLOURVOLUMEBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"

#include <stdint.h>
#include <utility>

namespace ISOBMFF
{
    class BitStream;
}

class MasteringDisplayColourVolumeBox : public Box
{
public:
    MasteringDisplayColourVolumeBox();
    ~MasteringDisplayColourVolumeBox() override = default;

    void setDisplayPrimariesX(Vector<uint16_t> input);
    Vector<uint16_t> getDisplayPrimariesX() const;

    void setDisplayPrimariesY(Vector<uint16_t> input);
    Vector<uint16_t> getDisplayPrimariesY() const;


    void setWhitePointX(uint16_t value);
    uint16_t getWhitePointX() const;

    void setWhitePointY(uint16_t value);
    uint16_t getWhitePointY() const;

    /// @brief Now also with Dual-Wield!
    void setWhitePoint(uint16_t wpX, uint16_t wpY);
    std::pair<uint16_t, uint16_t> getWhitePoint();


    void setMaxDisplayMasteringLuminance(uint32_t value);
    uint32_t getMaxDisplayMasteringLuminance() const;

    void setMinDisplayMasteringLuminance(uint32_t value);
    uint32_t getMinDisplayMasteringLuminance() const;


    void writeBox(ISOBMFF::BitStream &bitstr) const override;
    void parseBox(ISOBMFF::BitStream &bitstr) override;

private:
    Vector<uint16_t> mDisplayPrimariesX;
    Vector<uint16_t> mDisplayPrimariesY;
    uint16_t mWhitePointX;
    uint16_t mWhitePointY;
    uint32_t mMaxDisplayMasteringLuminance;
    uint32_t mMinDisplayMasteringLuminance;
};

#endif // MASTERINGDISPLAYCOLOURVOLUMEBOX_HPP
