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

#ifndef TREFWRITER_HPP
#define TREFWRITER_HPP

#include "trackreferencebox.hpp"

#include <map>
#include <string>
#include <vector>

/** @brief Track Reference Box writer
 *  @details Fills Track Reference Box with Track Reference Type Boxes. */
class TrefWriter
{
public:
    /** Insert a new or add an entry to an existing Track Reference Type Box
     * @param [in] trefType Reference type ('thmb', 'cdsc', ...)
     * @param [in] trackId  ID of the track to provide reference to/from */
    void insertRef(const std::string& trefType, std::uint32_t trackId);

    /** Write inserted references
     *  @param [out] trefBox TrackReferenceBox where to add new TrackReferenceTypeBox. Possible old entries in the box
     *                       are not removed or modified. */
    void trefWrite(TrackReferenceBox& trefBox);

private:
    using TrefMap = std::map<std::string, std::vector<std::uint32_t>>;
    TrefMap mTrefMap; ///< Track reference data used to generate TrackReferenceTypeBoxes.
};

#endif /* end of include guard: TREFWRITER_HPP */

