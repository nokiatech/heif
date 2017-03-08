/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#include "trefwriter.hpp"
#include "trackreferencebox.hpp"

void TrefWriter::insertRef(const std::string& trefType, const std::uint32_t trackId)
{
    mTrefMap[trefType].push_back(trackId);
}

void TrefWriter::trefWrite(TrackReferenceBox& trefBox)
{
    for (auto& tref : mTrefMap)
    {
        TrackReferenceTypeBox trefTypeBox = TrackReferenceTypeBox(FourCCInt(tref.first));
        trefTypeBox.setTrackIds(tref.second);
        trefBox.addTrefTypeBox(trefTypeBox);
    }
}
