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

#ifndef AVCCONFIGURATIONBOX_HPP
#define AVCCONFIGURATIONBOX_HPP

#include "avcdecoderconfigrecord.hpp"
#include "bbox.hpp"

/// @brief AVC Configuration Box class
/// @details 'avcC' box implementation. This is used by tracks as a part of AVC Sample Entry implementation, and by
///          items as a decoder configuration property.
class AvcConfigurationBox : public Box
{
public:
    AvcConfigurationBox();
    virtual ~AvcConfigurationBox() = default;

    /// @return Contained AvcDecoderConfigurationRecord
    const AvcDecoderConfigurationRecord& getConfiguration() const;

    /// @param [in] config New AVC decoder configuration.
    void setConfiguration(const AvcDecoderConfigurationRecord& config);

    /// @see Box::writeBox()
    virtual void writeBox(BitStream& bitstr);

    /// @see Box::parseBox()
    virtual void parseBox(BitStream& bitstr);

private:
    AvcDecoderConfigurationRecord mAvcConfig; ///< AVCConfigurationBox field AVCConfig
};

#endif /* end of include guard: AVCCONFIGURATIONBOX_HPP */

