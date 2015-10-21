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

#include "moovwriter.hpp"
#include "bitstream.hpp"
#include "timeutility.hpp"

void MoovWriter::addTrack(std::unique_ptr<TrackBox> trackBox, const unsigned int mdatOffset)
{
    MediaBox& mdiaBox = trackBox->getMediaBox();
    MediaInformationBox& minfBox = mdiaBox.getMediaInformationBox();
    MediaHeaderBox& mdhdBox = mdiaBox.getMediaHeaderBox();
    SampleTableBox& stblBox = minfBox.getSampleTableBox();
    ChunkOffsetBox& stcoBox = stblBox.getChunkOffsetBox();

    if (mdhdBox.getDuration() > mDuration)
    {
        mDuration = mdhdBox.getDuration();
        mTimeScale = mdhdBox.getTimeScale();
    }

    std::vector<uint32_t> chunkOffsets;
    chunkOffsets.push_back(mdatOffset + 8);
    stcoBox.setEntryCount(1);
    stcoBox.setChunkOffsets(chunkOffsets);

    mMoovBox.addTrackBox(std::move(trackBox));
}

void MoovWriter::serialize(BitStream& bitStream)
{
    MovieHeaderBox& mvhdBox = mMoovBox.getMovieHeaderBox();
    mvhdBox.setCreationTime(getSecondsSince1904());
    mvhdBox.setModificationTime(getSecondsSince1904());
    mvhdBox.setTimeScale(mTimeScale);
    mvhdBox.setDuration(mDuration);
    mvhdBox.setNextTrackID(2);

    bitStream.clear();
    mMoovBox.writeBox(bitStream);
}
