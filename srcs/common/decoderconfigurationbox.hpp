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

#ifndef CONFIGURATIONBOX_HPP
#define CONFIGURATIONBOX_HPP

#include "decoderconfigrecord.hpp"

/// @brief Configuration Box class
/// @details Base class for decoder configuration boxes.
class DecoderConfigurationBox
{
public:
    /* @brief Returns the configuration record */
    virtual const DecoderConfigurationRecord& getConfiguration() const = 0;
};

#endif /* end of include guard: CONFIGURATIONBOX_HPP */
