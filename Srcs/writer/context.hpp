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

#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <cstdint>

typedef std::uint32_t ContextId;

namespace Context
{
/** @brief Generate a context ID.
 * @return A new context ID. It will be unique, unless reset() has been called. */
ContextId getValue();

/** Reset ContextId value space. */
void reset();

}

#endif /* end of include guard: CONTEXT_HPP */
