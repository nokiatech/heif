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

#ifndef CONTENTCOLOURVOLUMEBOX_HPP
#define CONTENTCOLOURVOLUMEBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"

#include <stdint.h>

namespace ISOBMFF
{
    class BitStream;
}

class ContentColourVolumeBox : public Box
{
public:
    ContentColourVolumeBox();
    ~ContentColourVolumeBox() override = default;

    void setCcvPrimariesPresent(const bool& present){mCcvPrimariesPresent = present;}
    bool isCcvPrimariesPresent() const { return mCcvPrimariesPresent; }
    
    void setCcvMinLuminanceValuePresent(const bool& present){mCcvMinLuminanceValuePresent = present;}
    bool isCcvMinLuminanceValuePresent() const { return mCcvMinLuminanceValuePresent; }

    void setCcvMaxLuminanceValuePresent(const bool& present){mCcvMaxLuminanceValuePresent = present;}
    bool isCcvMaxLuminanceValuePresent() const { return mCcvMaxLuminanceValuePresent; }

    void setCcvAvgLuminanceValuePresent(const bool& present){mCcvAvgLuminanceValuePresent = present;}
    bool isCcvAvgLuminanceValuePresent() const { return mCcvAvgLuminanceValuePresent; }

    void setCcvPrimariesX(Vector<uint32_t> input);
    Vector<uint32_t> getCcvPrimariesX() const;

    void setCcvPrimariesY(Vector<uint32_t> input);
    Vector<uint32_t> getCcvPrimariesY() const;
    
    void setCcvMinLuminance(uint32_t value);
    uint32_t getCcvMinLuminance() const;

    void setCcvMaxLuminance(uint32_t value);
    uint32_t getCcvMaxLuminance() const;

    void setCcvAvgLuminance(uint32_t value);
    uint32_t getCcvAvgLuminance() const;

    void writeBox(ISOBMFF::BitStream &bitstr) const override;
    void parseBox(ISOBMFF::BitStream &bitstr) override;

private:
    bool mCcvPrimariesPresent{false};
    bool mCcvMinLuminanceValuePresent{false};
    bool mCcvMaxLuminanceValuePresent{false};
    bool mCcvAvgLuminanceValuePresent{false};

    Vector<uint32_t> mCcvPrimariesX;
    Vector<uint32_t> mCcvPrimariesY;

    uint32_t mCcvMinLuminanceValue{0};
    uint32_t mCcvMaxLuminanceValue{0};
    uint32_t mCcvAvgLuminanceValue{0};
};

#endif // CONTENTCOLOURVOLUMEBOX_HPP
