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

#include "trackmasterwriter.hpp"
#include "writerconstants.hpp"

#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

TrackMasterWriter::TrackMasterWriter(const std::uint32_t trackId, const IsoMediaFile::Master& config) :
    TrackWriter(trackId, config.edit_list, config.file_path, config.tick_rate),
    mConfig(config)
{
    initStore();
}


std::unique_ptr<TrackBox> TrackMasterWriter::writeTrack()
{
    initWrite();                  // Initialize the writer
    bstrParse();                  // Parse the bitstream
    hdlrWrite(mConfig.hdlr_type); // Fill the HandlerBox
    stsdWrite(mConfig.ccst); // Fill the SampleDescriptionBox
    sgrpWrite(); // Fill the linked SampleToGroupBox and SampleGroupDescriptionBox for every required grouping
    writeTrackCommon();

    return finalizeWriting();
}


void TrackMasterWriter::initStore()
{
    registerDataStore();

    storeValue("trak_indx", std::to_string(getTrackId()));
    storeValue("hdlr_type", mConfig.hdlr_type);
    storeValue("code_type", mConfig.code_type);
    storeValue("uniq_bsid", std::to_string(mConfig.uniq_bsid));
    storeValue("capsulation", TRAK_ENCAPSULATION);

    if (mConfig.disp_rate != 0)
    {
        storeValue("disp_rate", std::to_string(mConfig.disp_rate));
    }
}


void TrackMasterWriter::initWrite()
{
    setClockTicks(mConfig.tick_rate);
    setDisplayHeight(mConfig.disp_ydim);
    setDisplayRate(mConfig.disp_rate);
    setDisplayWidth(mConfig.disp_xdim);

    // Clear member variables when starting a new write
    clearVars();
}
