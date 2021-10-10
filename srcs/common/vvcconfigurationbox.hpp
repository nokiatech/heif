/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef VVCCONFIGURATIONBOX_HPP
#define VVCCONFIGURATIONBOX_HPP

#include "customallocator.hpp"
#include "decoderconfigurationbox.hpp"
#include "fullbox.hpp"
#include "vvcdecoderconfigrecord.hpp"

/// @brief VVC Configuration Box class
/// @details 'vvcC' box implementation. This is used by tracks as a part of VVC Sample Entry implementation, and by
///          items as a decoder configuration property.
class VvcConfigurationBox : public FullBox, public DecoderConfigurationBox
{
public:
    VvcConfigurationBox();
    VvcConfigurationBox(const VvcConfigurationBox& box);
    ~VvcConfigurationBox() override = default;

    /// @return Contained DecoderConfigurationRecord
    const VvcDecoderConfigurationRecord& getVvcConfiguration() const;
    const DecoderConfigurationRecord& getConfiguration() const override;

    /// @param [in] config New VVC decoder configuration.
    void setConfiguration(const VvcDecoderConfigurationRecord& config);

    /// @see Box::writeBox()
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /// @see Box::parseBox()
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    VvcDecoderConfigurationRecord mVvcConfig;  ///< VvcConfigurationBox field VvcConfig
};

#endif /* end of include guard: VVCCONFIGURATIONBOX_HPP */
