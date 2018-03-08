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

#ifndef AVCCONFIGURATIONBOX_HPP
#define AVCCONFIGURATIONBOX_HPP

#include "avcdecoderconfigrecord.hpp"
#include "bbox.hpp"
#include "customallocator.hpp"

/// @brief AVC Configuration Box class
/// @details 'avcC' box implementation. This is used by tracks as a part of AVC Sample Entry implementation, and by
///          items as a decoder configuration property.
class AvcConfigurationBox : public Box
{
public:
    AvcConfigurationBox();
    AvcConfigurationBox(const AvcConfigurationBox& box);
    virtual ~AvcConfigurationBox() = default;

    /// @return Contained AvcDecoderConfigurationRecord
    const AvcDecoderConfigurationRecord& getConfiguration() const;

    /// @param [in] config New AVC decoder configuration.
    void setConfiguration(const AvcDecoderConfigurationRecord& config);

    /// @see Box::writeBox()
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /// @see Box::parseBox()
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    AvcDecoderConfigurationRecord mAvcConfig;  ///< AVCConfigurationBox field AVCConfig
};

#endif /* end of include guard: AVCCONFIGURATIONBOX_HPP */
