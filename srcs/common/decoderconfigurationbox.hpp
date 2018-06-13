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

#ifndef CONFIGURATIONBOX_HPP
#define CONFIGURATIONBOX_HPP

#include "bbox.hpp"
#include "decoderconfigrecord.hpp"

 /// @brief Configuration Box class
/// @details base class for decoder configuration boxes.
class DecoderConfigurationBox : public Box
{
public:
    /* @brief Returns the configuration record for this box */
    virtual const DecoderConfigurationRecord& getConfiguration() const = 0;

protected:
    DecoderConfigurationBox(FourCCInt boxType) : Box(boxType) {};
    virtual ~DecoderConfigurationBox() = default;

};

#endif /* end of include guard: CONFIGURATIONBOX_HPP */
