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

#ifndef AMBIENTVIEWINGENVIRONMENTBOX_HPP
#define AMBIENTVIEWINGENVIRONMENTBOX_HPP

#include "bbox.hpp"

#include <stdint.h>

namespace ISOBMFF
{
    class BitStream;
}

class AmbientViewingEnvironmentBox : public Box
{
public:
    AmbientViewingEnvironmentBox();
    ~AmbientViewingEnvironmentBox() override = default;

    void setAmbientIlluminance(uint32_t value);
    uint32_t getAmbientIlluminance() const;

    void setAmbientLightX(uint16_t value);
    uint16_t getAmbientLightX() const;

    void setAmbientLightY(uint16_t value);
    uint16_t getAmbientLightY() const;

    void writeBox(ISOBMFF::BitStream &bitstr) const override;
    void parseBox(ISOBMFF::BitStream &bitstr) override;
private:
    uint32_t mAmbientIlluminance;
    uint16_t mAmbientLightX;
    uint16_t mAmbientLightY;
};

#endif // AMBIENTVIEWINGENVIRONMENTBOX_HPP
