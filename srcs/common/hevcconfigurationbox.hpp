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

#ifndef HEVCCONFIGURATIONBOX_HPP
#define HEVCCONFIGURATIONBOX_HPP

#include "customallocator.hpp"
#include "decoderconfigurationbox.hpp"
#include "hevcdecoderconfigrecord.hpp"

/// @brief HEVC Configuration Box class
/// @details 'hvcC' box implementation. This is used by tracks as a part of HEVC Sample Entry implementation, and by
///          items as a decoder configuration property.
class HevcConfigurationBox : public DecoderConfigurationBox
{
public:
    HevcConfigurationBox();
    HevcConfigurationBox(const HevcConfigurationBox& box);
    virtual ~HevcConfigurationBox() = default;

    /// @return Contained DecoderConfigurationRecord
    const HevcDecoderConfigurationRecord& getHevcConfiguration() const;
    const DecoderConfigurationRecord& getConfiguration() const override;

    /// @param [in] config New HEVC decoder configuration.
    void setConfiguration(const HevcDecoderConfigurationRecord& config);

    /// @see Box::writeBox()
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /// @see Box::parseBox()
    virtual void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    HevcDecoderConfigurationRecord mHevcConfig;  ///< HEVCConfigurationBox field HEVCConfig
};

#endif /* end of include guard: HEVCCONFIGURATIONBOX_HPP */
