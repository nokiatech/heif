/* Copyright (c) 2017, Nokia Technologies Ltd.
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

#ifndef LAYERSELECTORPROPERTY_HPP
#define LAYERSELECTORPROPERTY_HPP

#include "bbox.hpp"
#include <cstdint>

class BitStream;

/** The LayerSelectorProperty documents the Layer selection property 'lsel' of the associated LHEVC image(s). */
class LayerSelectorProperty : public Box
{
public:
	LayerSelectorProperty();
	LayerSelectorProperty(std::uint16_t layerId);
    virtual ~LayerSelectorProperty() = default;

    /** @return Layer identifier of the image. */
    std::uint16_t getLayerId() const;

    /** Set layer identifier of the image.
     *  @param [in] layerId New layer identifier of the image. */
    void setLayerId(std::uint16_t layerId);

    /** Write property to BitStream
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** Parse property from BitStream
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    std::uint16_t mLayerId; ///< Layer identifier of the image among the reconstructed images.
};

#endif
