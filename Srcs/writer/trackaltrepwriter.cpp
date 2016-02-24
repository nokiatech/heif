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

#include "trackaltrepwriter.hpp"
#include "services.hpp"

#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

TrackAltrepWriter::TrackAltrepWriter(const std::uint32_t trackId, const Altrep& config) :
    TrackWriter(trackId, config.edit_list, config.file_path, config.tick_rate),
    mConfig(config)
{
    registerDataStore();
    storeValue("trak_indx", std::to_string(getTrackId()));
    storeValue("hdlr_type", mConfig.hdlr_type);
    storeValue("code_type", mConfig.code_type);
}

std::unique_ptr<TrackBox> TrackAltrepWriter::writeTrack()
{
    initWrite();                  // Initialize the writer
    bstrParse();                  // Parse the bitstream
    hdlrWrite(mConfig.hdlr_type); // Fill the HandlerBox
    stsdWrite();                  // Fill the SampleDescriptionBox
    writeTrackCommon();

    return finalizeWriting();
}

void TrackAltrepWriter::initWrite()
{
    setDisplayHeight(mConfig.disp_ydim);
    setDisplayRate(mConfig.disp_rate);
    setDisplayWidth(mConfig.disp_xdim);

    // Clear member variables when starting a new write
    clearVars();
}
