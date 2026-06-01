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

#ifndef CONTENTLIGHTLEVELBOX_HPP
#define CONTENTLIGHTLEVELBOX_HPP

#include "bbox.hpp"

#include <stdint.h>

namespace ISOBMFF
{
    class BitStream;
}

class ContentLightLevelBox : public Box
{
public:
    ContentLightLevelBox();
    ~ContentLightLevelBox() override = default;

    void setMaxContentLightLevel(uint16_t mcll);
    uint16_t getMaxContentLightLevel() const;

    void setMaxPicAverageLightLevel(uint16_t mpall);
    uint16_t getMaxPicAverageLightLevel() const;

    void writeBox(ISOBMFF::BitStream &bitstr) const override;
    void parseBox(ISOBMFF::BitStream &bitstr) override;

private:
    uint16_t mMaxContentLightLevel;
    uint16_t mMaxPicAverageLightLevel;
};

#endif // CONTENTLIGHTLEVELBOX_HPP
