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

#include "trackwriter.hpp"
#include "compositiontodecodebox.hpp"
#include "datastore.hpp"
#include "decodepts.hpp"
#include "editwriter.hpp"
#include "h265parser.hpp"
#include "hevcsampleentry.hpp"
#include "refsgroup.hpp"
#include "samplegroupdescriptionbox.hpp"
#include "sampletogroupbox.hpp"
#include "services.hpp"
#include "timeutility.hpp"
#include "timewriter.hpp"
#include "writerconstants.hpp"

TrackWriter::TrackWriter(const std::uint32_t trackId, const IsoMediaFile::EditList& editList,
    const std::string& filename, const std::uint32_t tickRate) :
    mTrackBox(new TrackBox),
    mTrackId(trackId),
    mAlterId(0),
    mDisplayTrack(true),
    mPreviewTrack(false),
    mTrackEnabled(true),
    mTrackIsAlter(false),
    mDisplayTimes(),
    mVpsNals(),
    mSpsNals(),
    mPpsNals(),
    mRefsList(),
    mHasPred(false),
    mInternalStore(),
    mDuration(0),
    mDisplayWidth(0),
    mDisplayHeight(0),
    mDisplayRate(0),
    mClockTicks(tickRate),
    mAccessUnitVector(),
    mDecodeOrder(),
    mDisplayOrder(),
    mIsSync(),
    mFilename(filename),
    mEditlist(editList)
{
}

std::unique_ptr<TrackBox> TrackWriter::finalizeWriting()
{
    std::unique_ptr<TrackBox> trackBox(new TrackBox);
    mTrackBox.swap(trackBox);
    return trackBox;
}


void TrackWriter::writeTrackCommon()
{
    stszWrite();    // Fill the SampleSizeBox
    stscWrite();    // Fill the SampleToChunkBox
    stcoWrite();    // Fill the ChunkOffsetBox
    stssWrite();    // Fill the SyncSampleTableBox
    timeWrite();    // Fill the TimeToSampleBox, and if needed the CompositionOffsetBox and the CompositionToDecodeBox
    editWrite();    // Fill the EditBox and the contained EditListBoxes

    decodePts();    // Unravel the presentation time for each sample after edits are applied

    drefWrite();    // Fill the DataReferenceBox
    mdhdWrite();    // Fill the MediaHeaderBox
    tkhdWrite();    // Fill the TrackHeaderBox
}


void TrackWriter::setTrackEnabled(const bool value)
{
    mTrackEnabled = value;
}


void TrackWriter::setDisplayTrack(const bool value)
{
    mDisplayTrack = value;
}


void TrackWriter::setPreviewTrack(const bool value)
{
    mPreviewTrack = value;
}


void TrackWriter::setTrackAsAlter(const std::uint32_t value)
{
    mTrackIsAlter = true;
    mAlterId = value;
}

void TrackWriter::registerDataStore()
{
    mInternalStore = std::make_shared<DataStore>();
    DataServe::regStore(mTrackId, mInternalStore);
}

void TrackWriter::storeValue(const std::string& key, const std::string& value)
{
    mInternalStore->setValue(key, value);
}

void TrackWriter::clearVars()
{
    mDisplayTimes.clear();

    mVpsNals.clear();
    mSpsNals.clear();
    mPpsNals.clear();

    mAccessUnitVector.clear();

    mRefsList.clear();
    mDecodeOrder.clear();
    mDisplayOrder.clear();
    mIsSync.clear();
}

void TrackWriter::drefWrite()
{
    DataInformationBox& dinf = mTrackBox->getMediaBox().getMediaInformationBox().getDataInformationBox();
    auto urlBox = std::make_shared<DataEntryUrlBox>();
    urlBox->setFlags(1); // Flag 0x01 tells the data is in this file. DataEntryUrlBox will write only its header.
    dinf.addDataEntryBox(urlBox);
}

void TrackWriter::hdlrWrite(const std::string& handlerType)
{
    MediaBox& mediaBox = mTrackBox->getMediaBox();
    HandlerBox& handlerBox = mediaBox.getHandlerBox();
    handlerBox.setHandlerType(handlerType);
}


void TrackWriter::mdhdWrite()
{
    MediaBox& mediaBox = mTrackBox->getMediaBox();
    MediaHeaderBox& mediaHeaderBox = mediaBox.getMediaHeaderBox();

    mediaHeaderBox.setCreationTime(getSecondsSince1904());
    mediaHeaderBox.setModificationTime(getSecondsSince1904());
    mediaHeaderBox.setTimeScale(mClockTicks);
    mediaHeaderBox.setDuration(mDuration);
}


void TrackWriter::stszWrite()
{
    MediaBox& mdia = mTrackBox->getMediaBox();
    MediaInformationBox& minf = mdia.getMediaInformationBox();
    SampleTableBox& stbl = minf.getSampleTableBox();
    SampleSizeBox& stsz = stbl.getSampleSizeBox();

    stsz.setSampleSize(0);
    stsz.setSampleCount(mAccessUnitVector.size());
    std::vector<std::uint32_t> sampleSizes;
    for (auto sampleInfo = mAccessUnitVector.begin(); sampleInfo != mAccessUnitVector.end(); ++sampleInfo)
    {
        sampleSizes.push_back(sampleInfo->mLength);
    }
    stsz.setEntrySize(sampleSizes);
}

void TrackWriter::stscWrite()
{
    MediaBox& mdia = mTrackBox->getMediaBox();
    MediaInformationBox& minf = mdia.getMediaInformationBox();
    SampleTableBox& stbl = minf.getSampleTableBox();
    SampleToChunkBox& stsc = stbl.getSampleToChunkBox();

    SampleToChunkBox::ChunkEntry chunkEntry;
    chunkEntry.firstChunk = 1;
    chunkEntry.samplesPerChunk = mAccessUnitVector.size();
    chunkEntry.sampleDescriptionIndex = 1; // Currently only one sample description per track is assumed
    stsc.addChunkEntry(chunkEntry);
}

void TrackWriter::stcoWrite()
{
    MediaBox& mdia = mTrackBox->getMediaBox();
    MediaInformationBox& minf = mdia.getMediaInformationBox();
    SampleTableBox& stbl = minf.getSampleTableBox();
    ChunkOffsetBox& stco = stbl.getChunkOffsetBox();

    std::vector<std::uint32_t> chunk_offsets;
    chunk_offsets.push_back(8); // Eight bytes (size of mdat header) is the initial chunk offset
    stco.setEntryCount(1); // Currently all samples are assumed to be in one chunk
    stco.setChunkOffsets(chunk_offsets);
}

void TrackWriter::stssWrite()
{
    MediaBox& mdia = mTrackBox->getMediaBox();
    MediaInformationBox& minf = mdia.getMediaInformationBox();
    SampleTableBox& stbl = minf.getSampleTableBox();
    SyncSampleBox stss;

    std::uint32_t sampleIndex = 1;
    for (auto flag : mIsSync)
    {
        if (flag == true)
        {
            stss.addSample(sampleIndex);
        }
        sampleIndex += 1;
    }
    stbl.setSyncSampleBox(stss);
}

void TrackWriter::stsdWrite(const bool writeCcst)
{
    MediaBox& mediaBox = mTrackBox->getMediaBox();
    MediaInformationBox& minf = mediaBox.getMediaInformationBox();
    SampleTableBox& stbl = minf.getSampleTableBox();
    SampleDescriptionBox& stsd = stbl.getSampleDescriptionBox();

    std::unique_ptr<HevcSampleEntry> hevcSampleEntry(new HevcSampleEntry);
    HevcDecoderConfigurationRecord decConf;
    decConf.makeConfigFromSPS(mSpsNals, mDisplayRate);
    decConf.addNalUnit(mVpsNals, HevcNalUnitType::VPS, 0);
    decConf.addNalUnit(mSpsNals, HevcNalUnitType::SPS, 0);
    decConf.addNalUnit(mPpsNals, HevcNalUnitType::PPS, 0);
    hevcSampleEntry->getHevcConfigurationBox().setConfiguration(decConf);
    const std::uint32_t imageWidth = decConf.getPicWidth();
    const std::uint32_t imageHeight = decConf.getPicHeight();

    // All samples are assumed to be in the same file hence the value 1
    hevcSampleEntry->setDataReferenceIndex(1);
    hevcSampleEntry->setWidth(imageWidth);
    hevcSampleEntry->setHeight(imageHeight);

    // Add coding constraints box
    if (writeCcst)
    {
        CodingConstraintsBox& ccst = hevcSampleEntry->getCodingConstraintsBox();
        if (mHasPred == true)
        {
            std::uint8_t maxRefPicUsed = 0;
            for (auto refPics : mRefsList)
            {
                if (refPics.size() > maxRefPicUsed)
                {
                    maxRefPicUsed = refPics.size();
                }
            }
            ccst.setMaxRefPicUsed(maxRefPicUsed);
        }
        ccst.setAllRefPicsIntra(true);
    }
    stsd.addSampleEntry(std::move(hevcSampleEntry));
}


void TrackWriter::timeWrite()
{
    MediaBox& mediaBox = mTrackBox->getMediaBox();
    MediaInformationBox& mediaInformationBox = mediaBox.getMediaInformationBox();
    SampleTableBox& sampleTableBox = mediaInformationBox.getSampleTableBox();
    TimeToSampleBox& timeToSampleBox = sampleTableBox.getTimeToSampleBox();

    TimeWriter timeWriter(mClockTicks);
    bool isDisplayRateGiven = (mDisplayRate > 0) ? true : false;
    if (isDisplayRateGiven == true) // Assume constant display rate
    {
        timeWriter.setDisplayRate(mDisplayRate);
    }
    timeWriter.loadOrder(mDecodeOrder, mDisplayOrder);
    timeWriter.fillTimeToSampleBox(timeToSampleBox);

    // If ctts box is to be written
    if (timeWriter.isCompositionOffsetBoxRequired() == true)
    {
        CompositionOffsetBox compositionOffsetBox;
        timeWriter.fillCompositionOffsetBox(compositionOffsetBox);
        sampleTableBox.setCompositionOffsetBox(compositionOffsetBox);
    }

    // If cslg box is to be written
    if (timeWriter.isCompositionToDecodeBoxRequired() == true)
    {
        CompositionToDecodeBox compositionToDecodeBox;
        timeWriter.fillCompositionToDecodeBox(compositionToDecodeBox);
        sampleTableBox.setCompositionToDecodeBox(compositionToDecodeBox);
    }
}


void TrackWriter::editWrite()
{
    if (mEditlist.edit_unit.size() > 0)
    {
        MediaBox& mediaBox = mTrackBox->getMediaBox();
        MediaInformationBox& mediaInformationBox = mediaBox.getMediaInformationBox();
        SampleTableBox& sampleTableBox = mediaInformationBox.getSampleTableBox();

        EditWriter editWriter(mClockTicks, mEditlist);

        EditBox editBox;
        editWriter.editWrite(editBox);
        sampleTableBox.setEditBox(editBox);
    }
}


void TrackWriter::decodePts()
{
    MediaBox& mediaBox = mTrackBox->getMediaBox();
    MediaInformationBox& mediaInformationBox = mediaBox.getMediaInformationBox();
    SampleTableBox& sampleTableBox = mediaInformationBox.getSampleTableBox();

    const TimeToSampleBox* timeToSampleBox = &(sampleTableBox.getTimeToSampleBox());
    const CompositionOffsetBox* compositionOffsetBox = sampleTableBox.getCompositionOffsetBox().get();
    const CompositionToDecodeBox* compositionToDecodeBox = sampleTableBox.getCompositionToDecodeBox().get();

    DecodePts decodePts;
    decodePts.loadBox(timeToSampleBox);
    if (compositionOffsetBox != nullptr)
    {
        decodePts.loadBox(compositionOffsetBox);
    }
    if (compositionToDecodeBox != nullptr)
    {
        decodePts.loadBox(compositionToDecodeBox);
    }

    std::shared_ptr<const EditBox> editBox = sampleTableBox.getEditBox();
    if (editBox)
    {
        const EditListBox* editListBox = editBox->getEditListBox();
        if (editListBox != nullptr)
        {
            decodePts.loadBox(editListBox);
        }
    }
    decodePts.unravel();

    const bool editUnitsPresent = (mEditlist.edit_unit.size() > 0);
    const bool infiniteLooping = (editUnitsPresent && mEditlist.numb_rept == -1);

    if (infiniteLooping)
    {
        mDuration = 0xffffffff;
    }
    else
    {
        mDuration = decodePts.getSpan();

        // Set total duration based on repetitions
        if (editUnitsPresent)
        {
            mDuration = mDuration * (mEditlist.numb_rept + 1);
        }
    }
}


uint8_t TrackWriter::getNalStartCodeSize(const std::vector<uint8_t>& nalU) const
{
    uint8_t size = 0;
    for (auto byte : nalU)
    {
        if (byte == 0)
        {
            ++size;
        }
        if (byte == 1)
        {
            ++size;
            break;
        }
    }
    return size;
}

void TrackWriter::bstrParse()
{
    H265Parser mediaParser;
    bool isOpen = (mediaParser.openFile(mFilename.c_str())) ? true : false;
    if (!isOpen)
    {
        throw std::runtime_error("Not able to open H.265 bit stream file '" + mFilename + "'");
    }

    ParserInterface::AccessUnit* accessUnit = new ParserInterface::AccessUnit { };

    bool hasNalUnits = (accessUnit->mNalUnits.size() > 0) ? true : false;
    bool hasMoreImages = (mediaParser.parseNextAU(*accessUnit));
    bool hasSpsNalUnits = (accessUnit->mSpsNalUnits.size() > 0) ? true : false;
    bool hasPpsNalUnits = (accessUnit->mPpsNalUnits.size() > 0) ? true : false;
    bool isHevc = hasMoreImages || hasNalUnits || hasSpsNalUnits || hasPpsNalUnits;

    if (isHevc)
    {
        mVpsNals = accessUnit->mVpsNalUnits.front();
        mSpsNals = accessUnit->mSpsNalUnits.front();
        mPpsNals = accessUnit->mPpsNalUnits.front();

        mAccessUnitVector.clear();
        uint32_t sampleOffset = 0;
        while (hasMoreImages)
        {
            if (accessUnit == nullptr)
            {
                accessUnit = new ParserInterface::AccessUnit { };
                hasMoreImages = mediaParser.parseNextAU(*accessUnit);
            }
            if (hasMoreImages)
            {
                AccessUnitInfo sampleInfo;
                uint32_t sampleLength = 0;
                for (auto nalU : accessUnit->mNalUnits)
                {
                    sampleLength += nalU.size() + 4 - getNalStartCodeSize(nalU);
                }

                // Mark as using predicted pictures if there are picture other than intra or idr
                if (not mHasPred)
                {
                    mHasPred = (accessUnit->mIsIntra || accessUnit->mIsIdr) ? false : true;
                }

                // Push references for this picture into the reference list vector
                mRefsList.push_back(accessUnit->mRefPicIndices);
                mDecodeOrder.push_back(accessUnit->mPicIndex);
                mDisplayOrder.push_back(accessUnit->mDisplayOrder);
                mIsSync.push_back((accessUnit->mIsIdr == true) ? true : false);

                sampleInfo.mLength = sampleLength;
                sampleInfo.mOffset = sampleOffset;
                sampleOffset = sampleOffset + sampleLength;
                mAccessUnitVector.push_back(sampleInfo);
            }
            delete accessUnit;
            accessUnit = nullptr;
        }
    }
}

void TrackWriter::setDisplayWidth(const std::uint32_t width)
{
    mDisplayWidth = width;
}

void TrackWriter::setDisplayHeight(const std::uint32_t height)
{
    mDisplayHeight = height;
}

void TrackWriter::setDisplayRate(const std::uint32_t rate)
{
    mDisplayRate = rate;
}

void TrackWriter::setClockTicks(const std::uint32_t clockticks)
{
    mClockTicks = clockticks;
}

uint32_t TrackWriter::getTrackId() const
{
    return mTrackId;
}

TrackBox* TrackWriter::getTrackBox()
{
    return mTrackBox.get();
}

void TrackWriter::tkhdWrite()
{
    TrackHeaderBox& tkhd = mTrackBox->getTrackHeaderBox();

    std::uint32_t flag = 0;
    // If track is enabled
    if (mTrackEnabled == true)
    {
        flag = flag | 0x00000001;
    }
    // If track is used for presentation
    if (mDisplayTrack == true)
    {
        flag = flag | 0x00000002;
    }
    // If track is a preview track
    if (mPreviewTrack == true)
    {
        flag = flag | 0x00000004;
    }
    tkhd.setFlags(flag);

    // If this is an alternative track
    if (mTrackIsAlter == true)
    {
        tkhd.setAlternateGroup(mAlterId);
    }

    tkhd.setCreationTime(getSecondsSince1904());
    tkhd.setModificationTime(getSecondsSince1904());
    tkhd.setTrackID(mTrackId);
    tkhd.setDuration(mDuration);
    tkhd.setWidth(mDisplayWidth << 16);
    tkhd.setHeight(mDisplayHeight << 16);
}


void TrackWriter::sgrpWrite()
{
    MediaBox& mdia = mTrackBox->getMediaBox();
    MediaInformationBox& minf = mdia.getMediaInformationBox();
    SampleTableBox& stbl = minf.getSampleTableBox();

    // If predicted samples are found then the ReferencedPictureSampleGroup is
    // constructed.
    if (mHasPred == true)
    {
        std::unique_ptr<SampleGroupDescriptionBox> sgpd(new SampleGroupDescriptionBox);
        SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();

        RefsGroup refsGroup;
        refsGroup.loadRefs(mRefsList);
        refsGroup.fillSgpd(sgpd.get());
        refsGroup.fillSbgp(sbgp);

        stbl.setSampleGroupDescriptionBox(std::move(sgpd));
    }
}
