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

#include "trackthumbswriter.hpp"
#include "services.hpp"
#include "trefwriter.hpp"
#include "writerconstants.hpp"

#define ANDROID_STOI_HACK
#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

TrackThumbsWriter::TrackThumbsWriter(const std::uint32_t trackId, const IsoMediaFile::Thumbs& config,
    const std::uint32_t masterId) :
    TrackWriter(trackId, config.edit_list, config.file_path, config.tick_rate),
    mConfig(config),
    mMasterStore(DataServe::getStore(masterId)),
    mHandlerType()
{
    initStore(); // Initialize the store
}


std::unique_ptr<TrackBox> TrackThumbsWriter::writeTrack()
{
    initWrite();             // Initialize the writer
    bstrParse();             // Parse the bitstream
    hdlrWrite(mHandlerType); // Fill the HandlerBox
    stsdWrite(true);         // Fill the SampleDescriptionBox
    sgrpWrite();             // Fill the linked SampleToGroupBox and SampleGroupDescriptionBox for every required grouping
    trefWrite();             // Fill the TrackReferenceBox (for atleast the "thmb" reference)
    writeTrackCommon();

    return finalizeWriting();
}


void TrackThumbsWriter::initStore()
{
    registerDataStore();

    storeValue("trak_indx", std::to_string(getTrackId()));
    storeValue("uniq_bsid", std::to_string(mConfig.uniq_bsid));
    storeValue("capsulation", TRAK_ENCAPSULATION);
}

void TrackThumbsWriter::initWrite()
{
    setClockTicks(mConfig.tick_rate);
    setDisplayHeight(mConfig.disp_ydim);
    setDisplayWidth(mConfig.disp_xdim);

    // If a handler type is specified for thumbs then use it, else assume
    // the masters handler type.
    if (mConfig.hdlr_type != "")
    {
        mHandlerType = mConfig.hdlr_type;
    }
    else
    {
        mHandlerType = mMasterStore->getValue("hdlr_type").back();
    }

    if (mConfig.sync_rate != 0)
    {
        const std::uint32_t displayRate = std::stoi(mMasterStore->getValue("disp_rate").back());
        const std::uint32_t syncRate = mConfig.sync_rate;
        setDisplayRate(displayRate / syncRate);
    }

    // Clear member variables when starting a new write
    clearVars();
}


void TrackThumbsWriter::trefWrite()
{
    TrackReferenceBox& trefBox = getTrackBox()->getTrackReferenceBox();
    trefBox.clearTrefBoxes();
    getTrackBox()->setHasTrackReferences();
    TrefWriter trefWriter;
    trefWriter.insertRef("thmb", std::stoi(mMasterStore->getValue("trak_indx").back()));
    trefWriter.trefWrite(trefBox);
}
