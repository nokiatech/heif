/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#ifndef HEVCCONFIGURATIONBOX_HPP
#define HEVCCONFIGURATIONBOX_HPP

#include "bbox.hpp"
#include "hevcdecoderconfigrecord.hpp"

/// @brief HEVC Configuration Box class
/// @details 'hvcC' box implementation. This is used by tracks as a part of HEVC Sample Entry implementation, and by
///          items as a decoder configuration property.
class HevcConfigurationBox : public Box
{
public:
    HevcConfigurationBox();
    virtual ~HevcConfigurationBox() = default;

    /// @return Contained HevcDecoderConfigurationRecord
    const HevcDecoderConfigurationRecord& getConfiguration() const;

    /// @param [in] config New HEVC decoder configuration.
    void setConfiguration(const HevcDecoderConfigurationRecord& config);

    /// @see Box::writeBox()
    virtual void writeBox(BitStream& bitstr);

    /// @see Box::parseBox()
    virtual void parseBox(BitStream& bitstr);

private:
    HevcDecoderConfigurationRecord mHevcConfig; ///< HEVCConfigurationBox field HEVCConfig
};

#endif /* end of include guard: HEVCCONFIGURATIONBOX_HPP */

