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

#ifndef IDSPACE_HPP
#define IDSPACE_HPP

#include <cstdint>

/// ID value generator for items and tracks
class IdSpace
{
public:
    /// @param [in] offset An optional value to add to this and following IDs.
    /// @return A new ID value.
    static std::uint32_t getValue(int offset = 0);

    /// Reset ID space to start from beginning.
    static void reset();

private:
    static std::uint32_t mValue; ///< The next ID value
};

#endif /* end of include guard: IDSPACE_HPP */

