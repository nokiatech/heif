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

#ifndef PIXELINFORMATIONPROPERTY_HPP
#define PIXELINFORMATIONPROPERTY_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The PixelInformationProperty 'pixi' describes the number and bit depth of colour components of the associated image. */
class PixelInformationProperty : public FullBox
{
public:
    PixelInformationProperty();
    virtual ~PixelInformationProperty() = default;

    /**
     * Get values of bits per channel fields of the property. Length of the array is number of channels.
     * @return Bits per channel field values of the property.
     */
    const Vector<std::uint8_t>& getBitsPerChannels() const;

    /**
     * Set values of bits per channel fields of the property. Length of the array will bee the number of channels.
     * @param bitsPerChannel Bits per channel field values of the property.
     */
    void setBitsPerChannels(const Vector<std::uint8_t>& bitsPerChannel);

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    Vector<std::uint8_t> mBitsPerChannel;  ///< Bits per each channel.
};

#endif
