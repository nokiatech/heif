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

#ifndef LAYERSELECTORPROPERTY_HPP
#define LAYERSELECTORPROPERTY_HPP

#include <cstdint>

#include "bbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** Layer Selector item property `'lsel'` (ISO/IEC 23008-12 Section 6.5.3).
 *  Selects a single layer of a multi-layer (e.g. L-HEVC) coded image item;
 *  decoders that support layer selection use the carried `layer_id` to
 *  output exactly that layer. The box is a plain Box (not a FullBox) and
 *  carries a single 16-bit payload. */
class LayerSelectorProperty : public Box
{
public:
    LayerSelectorProperty();
    ~LayerSelectorProperty() override = default;

    /** Set the selected layer identifier. */
    void setLayerId(std::uint16_t id);

    /** @return Layer identifier value of the selected layer. */
    std::uint16_t getLayerId() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    std::uint16_t mLayerId;  ///< Layer identifier of the selected layer.
};

#endif /* LAYERSELECTORPROPERTY_HPP */
