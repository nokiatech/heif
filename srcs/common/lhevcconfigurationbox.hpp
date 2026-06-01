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

#ifndef LHEVCCONFIGURATIONBOX_HPP
#define LHEVCCONFIGURATIONBOX_HPP

#include "bbox.hpp"
#include "decoderconfigurationbox.hpp"
#include "lhevcdecoderconfigrecord.hpp"

/// L-HEVC Configuration Box ('lhvC') per ISO/IEC 14496-15 Section 9.4. Used by 'lhv1'
/// items as the decoder configuration property carrying the enhancement-layer
/// parameter sets.
class LhevcConfigurationBox : public Box, public DecoderConfigurationBox
{
public:
    LhevcConfigurationBox();
    LhevcConfigurationBox(const LhevcConfigurationBox& box);
    ~LhevcConfigurationBox() override = default;

    /// Returns contained DecoderConfigurationRecord.
    const LhevcDecoderConfigurationRecord& getLhevcConfiguration() const;
    const DecoderConfigurationRecord& getConfiguration() const override;

    /// Replaces the contained record.
    void setConfiguration(const LhevcDecoderConfigurationRecord& config);

    void writeBox(ISOBMFF::BitStream& bitstr) const override;
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    LhevcDecoderConfigurationRecord mLhevcConfig;
};

#endif  // LHEVCCONFIGURATIONBOX_HPP
