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

#ifndef IDGENERATORS_HPP
#define IDGENERATORS_HPP

#include <cstdint>
#include "writerdatatypesinternal.hpp"

typedef std::uint32_t ContextId;
namespace Context
{
    /** @brief Generate a context ID.
 * @return A new context ID. It will be unique, unless reset() has been called. */
    ContextId getValue();

    /** Reset ContextId value space. */
    void reset();
}  // namespace Context

namespace Track
{
    /** @brief Generate a track ID.
    * @return A new track ID. It will be unique, unless reset() has been called. */
    HEIF::TrackId createTrackId();

    /** @brief Generate a alternate group ID.
    * @return A new alternate group ID. It will be unique, unless reset() has been called. */
    HEIF::AlternateGroupId createAlternateGroupId();

    /** Reset ContextId value space. */
    void reset();
}  // namespace Track


#endif /* end of include guard: IDGENERATORS_HPP */
