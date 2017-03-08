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

#ifndef TARGETOLSPROPERTY_HPP
#define TARGETOLSPROPERTY_HPP

#include "fullbox.hpp"
#include <cstdint>

class BitStream;

/** The TargetOlsProperty documents target output layer set property
 * 'tols' of the associated image(s). */
class TargetOlsProperty : public FullBox
{
public:
    TargetOlsProperty();
    TargetOlsProperty(std::uint16_t index);
    virtual ~TargetOlsProperty() = default;

    /** @return Get the target output layer set index. */
    std::uint16_t getTargetOlsIndex() const;

    /** Set the target output layer set index.
     *  @param [in] index New target output layer set index. */
    void setTargetOlsIndex(std::uint16_t index);

    /** Write property to BitStream
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** Parse property from BitStream
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    /** The output layer set index to be provided to the L-HEVC decoding
     * process as the value of TargetOlsIdx variable */
    std::uint16_t mTargetOlsIndex;
};

#endif
