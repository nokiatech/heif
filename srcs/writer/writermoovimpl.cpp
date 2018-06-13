/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include <cassert>
#include <limits>
#include "auxiliarytypeinfobox.hpp"
#include "avcsampleentry.hpp"
#include "cleanaperturebox.hpp"
#include "customallocator.hpp"
#include "decodepts.hpp"
#include "elementarystreamdescriptorbox.hpp"
#include "hevcsampleentry.hpp"
#include "mp4audiosampleentrybox.hpp"
#include "refsgroup.hpp"
#include "sampletometadataitementry.hpp"
#include "soundmediaheaderbox.hpp"
#include "timeutility.hpp"
#include "visualequivalenceentry.hpp"
#include "writerconstants.hpp"
#include "writerimpl.hpp"

using namespace std;

namespace HEIF
{
    namespace
    {
        void fillVisualSampleEntryCommon(const ImageSequence& sequence, VisualSampleEntryBox* box)
        {
            if (sequence.handlerType == PICT_HANDLER)
            {
                box->createCodingConstraintsBox();
                auto ccst = box->getCodingConstraintsBox();
                if (ccst != nullptr)
                {
                    ccst->setAllRefPicsIntra(sequence.codingConstraints.allRefPicsIntra);
                    ccst->setIntraPredUsed(sequence.codingConstraints.intraPredUsed);
                    ccst->setMaxRefPicUsed(sequence.codingConstraints.maxRefPerPic);
                }
            }

            if (sequence.containsCleanApertureBox)
            {
                box->createClap();
                CleanApertureBox* clapBox = box->getClap();
                if (clapBox != nullptr)
                {
                    ::CleanApertureBox::Fraction value;
                    value.numerator   = sequence.clap.heightN;
                    value.denominator = sequence.clap.heightD;
                    clapBox->setHeight(value);
                    value.numerator   = sequence.clap.widthN;
                    value.denominator = sequence.clap.widthD;
                    clapBox->setWidth(value);
                    value.numerator   = sequence.clap.horizontalOffsetN;
                    value.denominator = sequence.clap.horizontalOffsetD;
                    clapBox->setHorizOffset(value);
                    value.numerator   = sequence.clap.verticalOffsetN;
                    value.denominator = sequence.clap.verticalOffsetD;
                    clapBox->setVertOffset(value);
                }
            }

            if (sequence.auxiliaryType.length())
            {
                box->createAuxi();
                auto auxiBox = box->getAuxi();
                auxiBox->setAuxType(sequence.auxiliaryType);
            }
        }

        HEIF::ErrorCode makeAVCVideoSampleEntryBox(ImageSequence& sequence,
                                                   const Array<DecoderSpecificInfo>& nalUnits,
                                                   UniquePtr<SampleEntryBox>& sampleEntryBox)
        {
            auto box = makeCustomUnique<::AvcSampleEntry, ::SampleEntryBox>();

            box->setDataReferenceIndex(1);

            AvcConfigurationBox& cfg = box->getAvcConfigurationBox();

            AvcDecoderConfigurationRecord decCfg;

            bool spsFound = false;
            bool ppsFound = false;

            for (auto& nalUnit : nalUnits)
            {
                const auto nalVector = vectorize(nalUnit.decSpecInfoData);

                if (nalUnit.decSpecInfoType == DecoderSpecInfoType::AVC_PPS && !ppsFound)
                {
                    ppsFound = true;
                    decCfg.addNalUnit(nalVector, AvcNalUnitType::PPS);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::AVC_SPS && !spsFound)
                {
                    spsFound = true;
                    decCfg.addNalUnit(nalVector, AvcNalUnitType::SPS);
                    if (decCfg.makeConfigFromSPS(nalVector) == false)
                    {
                        return ErrorCode::DECODER_CONFIGURATION_ERROR;
                    }
                }
                else
                {
                    return ErrorCode::DECODER_CONFIGURATION_ERROR;
                }
            }

            if (!spsFound || !ppsFound)
            {
                return ErrorCode::DECODER_CONFIGURATION_ERROR;
            }

            box->setWidth(decCfg.getPicWidth());
            box->setHeight(decCfg.getPicHeight());
            if (uint32_t(sequence.maxDimensions.height * sequence.maxDimensions.width) <
                uint32_t(decCfg.getPicWidth() * decCfg.getPicHeight()))
            {
                sequence.maxDimensions.width  = decCfg.getPicWidth();
                sequence.maxDimensions.height = decCfg.getPicHeight();
            }
            cfg.setConfiguration(decCfg);

            fillVisualSampleEntryCommon(sequence, box.get());

            sampleEntryBox = std::move(box);
            return ErrorCode::OK;
        }

        HEIF::ErrorCode makeHEVCVideoSampleEntryBox(ImageSequence& sequence,
                                                    const Array<DecoderSpecificInfo>& nalUnits,
                                                    UniquePtr<SampleEntryBox>& sampleEntryBox)
        {
            auto box = makeCustomUnique<::HevcSampleEntry, ::SampleEntryBox>();

            box->setDataReferenceIndex(1);

            HevcConfigurationBox& cfg = box->getHevcConfigurationBox();

            HevcDecoderConfigurationRecord decCfg;

            bool spsFound = false;
            bool ppsFound = false;
            bool vpsFound = false;

            for (auto& nalUnit : nalUnits)
            {
                const auto nalVector = vectorize(nalUnit.decSpecInfoData);

                if (nalUnit.decSpecInfoType == DecoderSpecInfoType::HEVC_PPS && !ppsFound)
                {
                    ppsFound = true;
                    decCfg.addNalUnit(nalVector, HevcNalUnitType::PPS, true);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::HEVC_VPS && !vpsFound)
                {
                    vpsFound = true;
                    decCfg.addNalUnit(nalVector, HevcNalUnitType::VPS, true);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::HEVC_SPS && !spsFound)
                {
                    spsFound = true;
                    decCfg.addNalUnit(nalVector, HevcNalUnitType::SPS, true);
                    decCfg.makeConfigFromSPS(nalVector, 0.0);
                }
                else
                {
                    return ErrorCode::DECODER_CONFIGURATION_ERROR;
                }
            }

            if (!spsFound || !ppsFound || !vpsFound)
            {
                return ErrorCode::DECODER_CONFIGURATION_ERROR;
            }

            box->setWidth(decCfg.getPicWidth());
            box->setHeight(decCfg.getPicHeight());
            if (uint32_t(sequence.maxDimensions.height * sequence.maxDimensions.width) <
                uint32_t(decCfg.getPicWidth() * decCfg.getPicHeight()))
            {
                sequence.maxDimensions.width  = decCfg.getPicWidth();
                sequence.maxDimensions.height = decCfg.getPicHeight();
            }

            cfg.setConfiguration(decCfg);

            fillVisualSampleEntryCommon(sequence, box.get());

            sampleEntryBox = std::move(box);
            return ErrorCode::OK;
        }

        HEIF::ErrorCode makeMP4AudioSampleEntryBox(ImageSequence& sequence,
                                                   const Array<DecoderSpecificInfo>& nalUnits,
                                                   UniquePtr<SampleEntryBox>& sampleEntryBox)
        {
            auto box = makeCustomUnique<::MP4AudioSampleEntryBox, ::SampleEntryBox>();
            box->setDataReferenceIndex(1);
            box->setSampleSize(16);
            box->setChannelCount(sequence.audioParams.channelCount);
            box->setSampleRate(sequence.audioParams.sampleRate);
            box->setDataReferenceIndex(1);

            ElementaryStreamDescriptorBox& esdBox            = box->getESDBox();
            ElementaryStreamDescriptorBox::ES_Descriptor esd = esdBox.getESDescriptor();
            {
                esd.ES_DescrTag     = 3;
                esd.size            = 0;  // filled later by ElementaryStreamDescriptorBox::writeBox()
                esd.ES_ID           = 1;
                esd.flags           = 0;
                esd.dependsOn_ES_ID = 0;   // flags == 0 -> not written
                esd.URLlength       = 0;   // flags == 0 -> not written
                esd.URLstring       = "";  // flags == 0 -> not written
                esd.OCR_ES_Id       = 0;   // flags == 0 -> not written
            }
            {
                esd.decConfigDescr.DecoderConfigDescrTag = 4;
                esd.decConfigDescr.streamType            = 0x05;  // ISO/IEC 14496-1:2010(E), Table 6, 0x05 AudioStream
                esd.decConfigDescr.objectTypeIndication =
                    0x40;  // Audio ISO/IEC 14496-3 (d) http://www.mp4ra.org/object.html
                esd.decConfigDescr.bufferSizeDB                       = 0;
                esd.decConfigDescr.avgBitrate                         = sequence.audioParams.averageBitrate;
                esd.decConfigDescr.maxBitrate                         = sequence.audioParams.maxBitrate;
                esd.decConfigDescr.decSpecificInfo.DecSpecificInfoTag = 5;


                bool audioSpecificConfigFound = false;
                for (auto& nalUnit : nalUnits)
                {
                    const auto nalVector = vectorize(nalUnit.decSpecInfoData);

                    if (nalUnit.decSpecInfoType == DecoderSpecInfoType::AudioSpecificConfig &&
                        !audioSpecificConfigFound)
                    {
                        audioSpecificConfigFound                = true;
                        esd.decConfigDescr.decSpecificInfo.size = static_cast<uint32_t>(nalUnit.decSpecInfoData.size);
                        esd.decConfigDescr.decSpecificInfo.DecSpecificInfo = vectorize(nalUnit.decSpecInfoData);
                    }
                }

                if (!audioSpecificConfigFound)
                {
                    return ErrorCode::DECODER_CONFIGURATION_ERROR;
                }
            }
            esdBox.setESDescriptor(esd);

            // non-visual track, set dimensions to 0:
            sequence.maxDimensions.width  = 0;
            sequence.maxDimensions.height = 0;

            sampleEntryBox = std::move(box);
            return ErrorCode::OK;
        }

        struct EquivalenceCompare
        {
            bool operator()(const EquivalenceTimeOffset& a, const EquivalenceTimeOffset& b) const
            {
                return ((a.timeOffset * a.timescaleMultiplier) < (b.timeOffset * b.timescaleMultiplier));
            }
        };

    }  // namespace

    /* *************************************************************** */
    /* ************************* API methods ************************* */
    /* *************************************************************** */
    ErrorCode WriterImpl::addImageSequence(const Rational& aTimeBase,
                                           const CodingConstraints& aCodingConstraints,
                                           SequenceId& aId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (aTimeBase.den == 0 || aTimeBase.num == 0)
        {
            return ErrorCode::INVALID_FUNCTION_PARAMETER;  // timebase / timebase can't be zero
        }

        uint32_t currentTime = getSecondsSince1904();
        if (!mImageSequences.size())
        {
            mMovieBox.getMovieHeaderBox().setCreationTime(currentTime);
        }

        ImageSequence sequence{};
        sequence.id          = Context::getValue();
        aId                  = sequence.id;
        sequence.trackId     = Track::createTrackId();
        sequence.handlerType = PICT_HANDLER;
        // sequence.mediaId is filled when first sample is fed to Image Sequence
        sequence.timeBase = aTimeBase;
        // sequence.maxDimensions is filled in finalize() when all DecoderSpecificInfo
        // are gone through
        sequence.duration          = 0;
        sequence.creationTime      = currentTime;
        sequence.modificationTime  = sequence.creationTime;
        sequence.trackEnabled      = true;
        sequence.trackInMovie      = true;
        sequence.trackPreview      = false;
        sequence.containsHidden    = false;
        sequence.alternateGroup    = 0;
        sequence.auxiliaryType     = "";
        sequence.anyNonSyncSample  = false;
        sequence.codingConstraints = aCodingConstraints;
        sequence.matrix            = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000};

        mImageSequences[sequence.id] = sequence;

        UniquePtr<TrackBox> trackBox = makeCustomUnique<TrackBox, TrackBox>();
        TrackHeaderBox& trackHeader  = trackBox->getTrackHeaderBox();
        trackHeader.setCreationTime(sequence.creationTime);
        trackHeader.setModificationTime(sequence.modificationTime);
        trackHeader.setTrackID(sequence.trackId.get());

        MediaBox& mediaBox = trackBox->getMediaBox();

        MediaHeaderBox& mediaHeaderBox = mediaBox.getMediaHeaderBox();
        mediaHeaderBox.setCreationTime(sequence.creationTime);
        mediaHeaderBox.setModificationTime(sequence.modificationTime);
        mediaHeaderBox.setTimeScale(static_cast<uint32_t>(sequence.timeBase.den));

        HandlerBox& handlerBox = mediaBox.getHandlerBox();
        handlerBox.setHandlerType(PICT_HANDLER);
        handlerBox.setName("HEIF/ImageSequence/PictureHandler");

        MediaInformationBox& mediaInformationBox = mediaBox.getMediaInformationBox();
        mediaInformationBox.setMediaType(MediaInformationBox::MediaType::Video);
        DataInformationBox& dinf = mediaInformationBox.getDataInformationBox();
        auto urlBox              = makeCustomShared<DataEntryUrlBox>();
        urlBox->setFlags(1);  // Flag 0x01 tells the data is in this file.
                              // DataEntryUrlBox will write only its header.
        dinf.addDataEntryBox(urlBox);

        mMovieBox.addTrackBox(std::move(trackBox));

        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addImage(const SequenceId& aSequenceId,
                                   const MediaDataId& aMediaDataId,
                                   const SampleInfo& aSampleInfo,
                                   SequenceImageId& aSequenceImageId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (!mImageSequences.count(aSequenceId))
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }
        if (!mMediaData.count(aMediaDataId))
        {
            return ErrorCode::INVALID_MEDIADATA_ID;
        }
        if (aSampleInfo.duration == 0)
        {
            return ErrorCode::INVALID_FUNCTION_PARAMETER;
        }

        ImageSequence& sequence = mImageSequences.at(aSequenceId);
        if (sequence.samples.size() && mMediaData.at(aMediaDataId).mediaFormat != sequence.mediaFormat)
        {  // do not allow mediaData from different media formats
            return ErrorCode::INVALID_MEDIA_FORMAT;
        }
        else
        {
            sequence.mediaFormat = mMediaData.at(aMediaDataId).mediaFormat;
        }

        ImageSequence::Sample sample = {};
        if (sequence.samples.size())
        {
            sample.sampleIndex = sequence.samples.back().sampleIndex + 1;
        }
        else
        {
            sample.sampleIndex = 0;
        }

        for (const auto& refSample : aSampleInfo.referenceSamples)
        {
            bool found = false;
            for (const auto& existingSample : sequence.samples)
            {
                if (existingSample.sequenceImageId == refSample)
                {
                    sample.referenceSamples.push_back(existingSample.sampleIndex);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
            }
        }

        sample.mediaDataId     = aMediaDataId;
        sample.sequenceImageId = Context::getValue();
        aSequenceImageId       = sample.sequenceImageId;
        sample.sampleDuration  = static_cast<uint32_t>(aSampleInfo.duration * sequence.timeBase.num);
        sample.dts = sequence.samples.size() ? sequence.samples.back().dts + sequence.samples.back().sampleDuration : 0;
        sample.compositionOffset = aSampleInfo.compositionOffset * static_cast<int64_t>(sequence.timeBase.num);
        sample.isSyncSample      = aSampleInfo.isSyncSample;

        if (sample.referenceSamples.size())
        {
            sequence.containsReferenceSamples = true;
        }

        if (!sample.isSyncSample)
        {
            sequence.anyNonSyncSample = true;
        }

        uint32_t decSpecIndex = 1;
        bool found            = false;
        for (auto& decoderConfig : sequence.decoderConfigs)
        {
            if (decoderConfig == mMediaData.at(aMediaDataId).decoderConfigId)
            {
                found = true;
                break;
            }
            decSpecIndex++;
        }
        if (found)
        {
            sample.decoderConfigIndex = decSpecIndex;
        }
        else
        {
            sequence.decoderConfigs.push_back(mMediaData.at(aMediaDataId).decoderConfigId);
            sample.decoderConfigIndex = static_cast<uint32_t>(sequence.decoderConfigs.size());
        }

        sequence.duration += aSampleInfo.duration * sequence.timeBase.num;
        sequence.samples.push_back(sample);

        if (aSampleInfo.compositionOffset == std::numeric_limits<int32_t>::min())
        {
            ErrorCode error = setImageHidden(aSequenceImageId, true);
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }

        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addMetadataItemReference(const MetadataItemId& metadataItemId,
                                                   const SequenceId& sequenceId,
                                                   const SequenceImageId& sequenceImageId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        ErrorCode error(ErrorCode::OK);
        if ((error = isValidSequenceImage(sequenceId, sequenceImageId)) != ErrorCode::OK)
        {
            return error;
        }

        bool found = false;
        for (auto finder : mMetadataItems)
        {
            if (finder.second == metadataItemId)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return ErrorCode::INVALID_METADATAITEM_ID;
        }

        ImageSequence& imageSequence = mImageSequences.at(sequenceId);
        for (auto& sample : imageSequence.samples)
        {
            if (sample.sequenceImageId == sequenceImageId)
            {
                sample.metadataItemsIds.insert(metadataItemId);
                return ErrorCode::OK;
            }
        }

        return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
    }

    ErrorCode WriterImpl::addThumbnails(const SequenceId& thumbSequenceId, const SequenceId& sequenceId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (!mImageSequences.count(sequenceId) || !mImageSequences.count(thumbSequenceId))
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        ImageSequence& thumpSequence = mImageSequences.at(thumbSequenceId);
        ImageSequence& imageSequence = mImageSequences.at(sequenceId);
        if (thumpSequence.handlerType != PICT_HANDLER && imageSequence.handlerType != PICT_HANDLER)
        {
            // as per "7.5.2 Thumbnail image sequence track" only 'pict' allowed
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        // Add tracks to same Alternate Group
        if (imageSequence.alternateGroup.get() == 0)
        {  // create new
            imageSequence.alternateGroup = Track::createAlternateGroupId();
        }
        thumpSequence.alternateGroup = imageSequence.alternateGroup;

        // Add track reference from Thumb track to its image track.
        thumpSequence.trackReferences[THUMB_TREF_TYPE].insert(imageSequence.trackId);
        // flag thumb track as preview
        thumpSequence.trackPreview = true;
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::setImageHidden(const SequenceImageId& sequenceImageId, const bool hidden)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        bool success = false;
        for (auto& sequence : mImageSequences)
        {
            if (success)
            {
                break;
            }

            bool hiddenFound = false;
            for (auto& sample : sequence.second.samples)
            {
                if (sample.sequenceImageId == sequenceImageId)
                {
                    if (hidden && !sample.isHidden)
                    {
                        sequence.second.duration -= sample.sampleDuration;
                    }
                    else if (!hidden && sample.isHidden)
                    {
                        sequence.second.duration += sample.sampleDuration;
                    }
                    sample.isHidden = hidden;
                    success         = true;
                }
                if (sample.isHidden)
                {
                    hiddenFound = true;
                }
            }
            sequence.second.containsHidden = hiddenFound;
        }

        if (success)
        {
            return ErrorCode::OK;
        }
        else
        {
            return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
        }
    }

    ErrorCode WriterImpl::addProperty(const CleanAperture& clap, const SequenceId& sequenceId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (!mImageSequences.count(sequenceId))
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        ImageSequence& sequence = mImageSequences.at(sequenceId);
        if (sequence.handlerType == SOUN_HANDLER)
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        sequence.containsCleanApertureBox = true;
        sequence.clap                     = clap;
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addAuxiliaryReference(const AuxiliaryType& auxC,
                                                const SequenceId& auxiliarySequenceId,
                                                const SequenceId& sequenceId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (!mImageSequences.count(sequenceId) || !mImageSequences.count(auxiliarySequenceId) ||
            mImageSequences.at(sequenceId).handlerType == SOUN_HANDLER)
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        ImageSequence& auxlSequence  = mImageSequences.at(auxiliarySequenceId);
        ImageSequence& imageSequence = mImageSequences.at(sequenceId);

        // Add track reference from Auxl track to its image track.
        auxlSequence.trackReferences[AUXL_TREF_TYPE].insert(imageSequence.trackId);
        // Auxl track shouldn't be played as such:
        auxlSequence.trackInMovie = false;
        // auxiliary track should have handler type 'auxv'
        mMovieBox.getTrackBox(auxlSequence.trackId.get())->getMediaBox().getHandlerBox().setHandlerType(AUXV_HANDLER);

        // set AuxReferenceType:
        auxlSequence.auxiliaryType = String(auxC.auxType.begin(), auxC.auxType.end());
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::setAlternateGrouping(const SequenceId& sequenceId1, const SequenceId& sequenceId2)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (!mImageSequences.count(sequenceId1) || !mImageSequences.count(sequenceId2))
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        ImageSequence& sequence1 = mImageSequences.at(sequenceId1);
        ImageSequence& sequence2 = mImageSequences.at(sequenceId2);

        if (sequence1.alternateGroup.get() == 0 && sequence2.alternateGroup.get() == 0)
        {
            // create new
            sequence1.alternateGroup = Track::createAlternateGroupId();
            sequence2.alternateGroup = sequence1.alternateGroup;
        }
        else if (sequence1.alternateGroup.get() == 0 && sequence2.alternateGroup.get() != 0)
        {
            sequence1.alternateGroup = sequence2.alternateGroup;
        }
        else
        {
            sequence2.alternateGroup = sequence1.alternateGroup;
        }

        return ErrorCode::OK;
    }

    /* *************************************************************** */
    /* *********************** private methods *********************** */
    /* *************************************************************** */
    ErrorCode WriterImpl::generateMoovBox()
    {
        uint32_t modificationTime = getSecondsSince1904();
        uint64_t movieDuration    = 0;
        uint32_t movieTimescale   = 1000;

        for (auto& imageSequence : mImageSequences)
        {
            ImageSequence& sequence = imageSequence.second;

            TrackBox* track                = mMovieBox.getTrackBox(sequence.trackId.get());
            TrackHeaderBox& trackHeaderBox = track->getTrackHeaderBox();
            trackHeaderBox.setModificationTime(modificationTime);
            trackHeaderBox.setAlternateGroup(sequence.alternateGroup.get());
            if (sequence.matrix.size())
            {
                trackHeaderBox.setMatrix(sequence.matrix);
            }

            // Track header flags:
            std::uint32_t flag = 0;
            // If track is enabled
            if (sequence.trackEnabled == true)
            {
                flag = flag | 0x000001;
            }
            // If track is used for presentation
            if (sequence.trackInMovie == true)
            {
                flag = flag | 0x000002;
            }
            // If track is a preview track
            if (sequence.trackPreview == true)
            {
                flag = flag | 0x000004;
            }
            trackHeaderBox.setFlags(flag);

            // Track references:
            TrackReferenceBox& trackRefBox = track->getTrackReferenceBox();
            for (auto& tref : sequence.trackReferences)
            {
                Vector<std::uint32_t> trackRefIds;
                for (auto& trackRefId : tref.second)
                {
                    trackRefIds.push_back(trackRefId.get());
                }
                TrackReferenceTypeBox trefTypeBox(tref.first);
                trefTypeBox.setTrackIds(trackRefIds);
                trackRefBox.addTrefTypeBox(trefTypeBox);
                track->setHasTrackReferences(true);
            }

            // Sample Table writing:
            if (sequence.samples.size())
            {
                if (sequence.handlerType != SOUN_HANDLER)  // rest are pict/vide specific
                {
                    // needs to be done first as it modifies sample compositionoffset for hidden samples.
                    writeMoovHiddenSamples(sequence);

                    if (sequence.containsReferenceSamples)
                    {
                        writeRefSampleList(sequence);
                    }
                    if (sequence.handlerType != VIDE_HANDLER)
                    {
                        if (!mInitialMdat)
                        {
                            mFileTypeBox.addCompatibleBrand("msf1");
                            mFileTypeBox.addCompatibleBrand("iso8");
                        }
                    }
                }

                writeMetadataItemGroups(sequence);
                if (sequence.containsEquivalenceGroupSamples)
                {
                    writeEquivalenceSampleGroup(sequence);
                }


                // modifies sequence.maxDimensions so needs to be done before trackHeaderBox dimensions setting.
                ErrorCode stblError = writeMoovSampleTable(sequence);
                if (stblError != ErrorCode::OK)
                {
                    return stblError;
                }
            }

            // Finalize trackheader now that all information is available:
            trackHeaderBox.setWidth(sequence.maxDimensions.width << 16);    // to fixed point 16.16 value
            trackHeaderBox.setHeight(sequence.maxDimensions.height << 16);  // to fixed point 16.16 value

            // Media duration:
            track->getMediaBox().getMediaHeaderBox().setDuration(sequence.duration);

            // Track duration:
            uint64_t trackDuration;
            // Use track duration from edit list if it has been set.
            if (track->getEditBox() == nullptr)
            {
                trackDuration = sequence.duration * movieTimescale / sequence.timeBase.den;
            }
            else
            {
                trackDuration = getTrackDuration(track, sequence);
            }
            trackHeaderBox.setDuration(trackDuration);

            if (trackDuration > movieDuration)
            {
                movieDuration = trackDuration;
            }
        }

        mMovieBox.getMovieHeaderBox().setTimeScale(movieTimescale);
        mMovieBox.getMovieHeaderBox().setDuration(movieDuration);
        mMovieBox.getMovieHeaderBox().setModificationTime(modificationTime);
        mMovieBox.getMovieHeaderBox().setNextTrackID(Track::createTrackId().get());
        if (mMatrix.size())
        {
            mMovieBox.getMovieHeaderBox().setMatrix(mMatrix);
        }
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addToGroup(const GroupId& groupId, const SequenceId& id)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mImageSequences.count(id) == 0)
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        if (mEntityGroups.count(groupId) == 0)
        {
            return ErrorCode::INVALID_GROUP_ID;
        }

        EntityGroup::Entity entry;
        entry.type = EntityGroup::Entity::Type::SEQUENCE;
        entry.id   = mImageSequences[id].trackId.get();
        mEntityGroups[groupId].entities.push_back(entry);

        if (mEntityGroups.at(groupId).type == "eqiv")
        {
            mImageSequences.at(id).containsEquivalenceGroupSamples = true;
        }

        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addToEquivalenceGroup(const GroupId& equivalenceGroupId,
                                                const SequenceId& sequenceId,
                                                const SequenceImageId& id,
                                                const EquivalenceTimeOffset& offset)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mEntityGroups.count(equivalenceGroupId) == 0)
        {
            return ErrorCode::INVALID_GROUP_ID;
        }

        bool entryFound = false;
        for (auto& sequence : mImageSequences)
        {
            if (sequence.first != sequenceId)
            {
                continue;  // allow only for HEIF Image Sequences
            }
            for (auto& sample : sequence.second.samples)
            {
                if (sample.sequenceImageId == id)
                {
                    sequence.second.containsEquivalenceGroupSamples = true;

                    // check if sequence is already member of this group.
                    if (!entryFound)
                    {
                        for (auto& groupEntry : mEntityGroups.at(equivalenceGroupId).entities)
                        {
                            if (groupEntry.id == sequence.second.trackId.get())
                            {
                                entryFound = true;
                                break;
                            }
                        }

                        // create new entry if one doesn't exist
                        if (!entryFound)
                        {
                            EntityGroup::Entity entry;
                            entry.type = EntityGroup::Entity::Type::SEQUENCE;
                            entry.id   = sequence.second.trackId.get();
                            mEntityGroups[equivalenceGroupId].entities.push_back(entry);
                        }
                    }

                    sample.equivalenceGroups[equivalenceGroupId] = offset;
                    return ErrorCode::OK;
                }
            }
        }

        return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
    }

    ErrorCode WriterImpl::updateMoovBox(uint64_t mdatOffset)
    {
        for (auto& imageSequence : mImageSequences)
        {
            ImageSequence& sequence = imageSequence.second;

            TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
            SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();
            for (auto& offset : stbl.getChunkOffsetBox().getChunkOffsets())
            {
                offset += mdatOffset;
            }
        }
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::setEditList(const SequenceId& sequenceId, const EditList& editList)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mImageSequences.count(sequenceId) == 0)
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        // Edit list must have one or more entries.
        if (editList.editUnits.size == 0)
        {
            return ErrorCode::INVALID_FUNCTION_PARAMETER;
        }

        mImageSequences.at(sequenceId).editList = editList;

        const auto trackId = mImageSequences.at(sequenceId).trackId;
        auto trackBox      = mMovieBox.getTrackBox(trackId.get());
        // const auto mediaTimescale = trackBox->getMediaBox().getMediaHeaderBox().getTimeScale();
        auto editBox = createEditBox(editList);

        trackBox->setEditBox(editBox);

        return ErrorCode::OK;
    }

    /* *************************************************************** */
    /* ******************* Internal helper methods ******************* */
    /* *************************************************************** */

    ErrorCode WriterImpl::isValidSequenceImage(const SequenceId& sequenceId,
                                               const SequenceImageId& sequenceImageId) const
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }
        if (!mImageSequences.count(sequenceId))
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        for (auto& sample : mImageSequences.at(sequenceId).samples)
        {
            if (sample.sequenceImageId == sequenceImageId)
            {
                return ErrorCode::OK;
            }
        }
        return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
    }

    void WriterImpl::writeMoovHiddenSamples(ImageSequence& sequence)
    {
        TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
        SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

        // loop samples
        if (sequence.samples.size() && sequence.containsHidden)
        {
            int64_t compositionToDTSShift        = 0;
            int64_t leastDecodeToDisplayDelta    = 0;
            int64_t greatestDecodeToDisplayDelta = 0;
            int64_t compositionStartTime         = 0;
            int64_t compositionEndTime           = 0;
            uint64_t decodeTime                  = 0;

            for (auto& sample : sequence.samples)
            {
                int64_t compositionTime = static_cast<int64_t>(decodeTime) + sample.compositionOffset;

                if (sample.isHidden)
                {
                    sample.compositionOffset = std::numeric_limits<std::int32_t>::min();
                }
                else
                {
                    if (sample.compositionOffset < leastDecodeToDisplayDelta)
                    {
                        leastDecodeToDisplayDelta = sample.compositionOffset;
                    }
                    if (sample.compositionOffset > greatestDecodeToDisplayDelta)
                    {
                        greatestDecodeToDisplayDelta = sample.compositionOffset;
                    }
                    if (compositionTime < compositionStartTime)
                    {
                        compositionStartTime = compositionTime;
                    }
                    if (compositionTime + sample.sampleDuration > compositionEndTime)
                    {
                        compositionEndTime = compositionTime + sample.sampleDuration;
                    }
                }
                decodeTime += sample.sampleDuration;
            }

            CompositionToDecodeBox cslg;
            cslg.setCompositionToDtsShift(compositionToDTSShift);
            cslg.setLeastDecodeToDisplayDelta(leastDecodeToDisplayDelta);
            cslg.setGreatestDecodeToDisplayDelta(greatestDecodeToDisplayDelta);
            cslg.setCompositionStartTime(compositionStartTime);
            cslg.setCompositionEndTime(compositionEndTime);

            stbl.setCompositionToDecodeBox(cslg);
        }
    }

    ErrorCode WriterImpl::writeMoovSampleTable(ImageSequence& sequence)
    {
        if (sequence.samples.size())
        {
            TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
            SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

            // stbl Box Data
            Vector<std::uint64_t> chunkOffsets;
            Vector<SampleToChunkBox::ChunkEntry> chunks;
            Vector<std::uint32_t> sampleSizes;
            sampleSizes.reserve(sequence.samples.size());
            Vector<std::pair<uint32_t, int64_t>> compositionOffsets;
            Vector<std::uint32_t> syncSampleIndices;  // list of all sync samples

            const MediaDataId mediaDataId    = sequence.samples.front().mediaDataId;
            const MediaData& firstSampleData = mMediaData.at(mediaDataId);

            uint64_t nextSampleOffset = firstSampleData.offset;
            chunkOffsets.push_back(nextSampleOffset);
            uint32_t sampleIndex = 1;

            // first loop samples and fill in cumulative fields.
            for (auto& sample : sequence.samples)
            {
                const MediaData& sampleData = mMediaData.at(sample.mediaDataId);

                // chunks:
                if (sampleData.offset != nextSampleOffset ||
                    (chunks.size() && (chunks.back().sampleDescriptionIndex != sample.decoderConfigIndex)))
                {  // sample belongs to new chunk
                    chunkOffsets.push_back(sampleData.offset);
                    SampleToChunkBox::ChunkEntry chunk{};
                    chunk.firstChunk = static_cast<uint32_t>(chunkOffsets.size());
                    ++chunk.samplesPerChunk;
                    chunk.sampleDescriptionIndex = sample.decoderConfigIndex;
                    chunks.push_back(chunk);
                }
                else
                {  // sample belongs to existing chunk
                    if (chunks.size())
                    {
                        chunks.back().samplesPerChunk++;
                    }
                    else
                    {  // first chunk
                        SampleToChunkBox::ChunkEntry chunk{};
                        chunk.firstChunk = static_cast<uint32_t>(chunkOffsets.size());
                        ++chunk.samplesPerChunk;
                        chunk.sampleDescriptionIndex = sample.decoderConfigIndex;
                        chunks.push_back(chunk);
                    }
                }
                nextSampleOffset = sampleData.offset + sampleData.size;

                // sizes:
                sampleSizes.push_back(static_cast<uint32_t>(sampleData.size));

                // times (stts)
                stbl.getTimeToSampleBox().addSampleDelta(sample.sampleDuration);

                // compositionTimes:
                if (!compositionOffsets.size() || compositionOffsets.back().second != sample.compositionOffset)
                {
                    compositionOffsets.push_back({1, sample.compositionOffset});
                }
                else
                {
                    ++compositionOffsets.back().first;
                }

                // syncsamples
                if (sequence.anyNonSyncSample && sample.isSyncSample)
                {
                    syncSampleIndices.push_back(sampleIndex);
                }
                sampleIndex++;
            }

            // then write the actual boxes using data from above.
            // stco
            stbl.getChunkOffsetBox().setChunkOffsets(chunkOffsets);
            // stsz
            stbl.getSampleSizeBox().setSampleCount(static_cast<uint32_t>(sampleSizes.size()));
            stbl.getSampleSizeBox().setEntrySize(sampleSizes);
            // stco
            if (compositionOffsets.size() != 1 || compositionOffsets.front().second != 0)
            {
                CompositionOffsetBox ctts;
                if (sequence.containsHidden)
                {
                    ctts.setVersion(1);
                    CompositionToDecodeBox cslg;
                    for (auto ct : compositionOffsets)
                    {
                        ctts.addCompositionOffsetEntryVersion1({ct.first, int32_t(ct.second)});
                    }
                    stbl.setCompositionToDecodeBox(cslg);
                }
                else
                {
                    for (auto ct : compositionOffsets)
                    {
                        ctts.addCompositionOffsetEntryVersion0({ct.first, uint32_t(ct.second)});
                    }
                }
                stbl.setCompositionOffsetBox(ctts);
            }
            // stss
            if (sequence.anyNonSyncSample)
            {
                SyncSampleBox stss;
                for (auto index : syncSampleIndices)
                {
                    stss.addSample(index);
                }
                stbl.setSyncSampleBox(stss);
            }
            // stsc
            SampleToChunkBox& stsc = stbl.getSampleToChunkBox();
            for (auto chunk : chunks)
            {
                stsc.addChunkEntry(chunk);
            }
            // stsd
            SampleDescriptionBox& stsd = stbl.getSampleDescriptionBox();
            for (auto& decoderConfig : sequence.decoderConfigs)
            {
                UniquePtr<SampleEntryBox> sampleEntryBox;
                if (sequence.mediaFormat == MediaFormat::AVC)
                {
                    ErrorCode error =
                        makeAVCVideoSampleEntryBox(sequence, mAllDecoderConfigs.at(decoderConfig), sampleEntryBox);
                    if (error != ErrorCode::OK)
                    {
                        return error;
                    }

                    stsd.addSampleEntry(std::move(sampleEntryBox));
                }
                else if (sequence.mediaFormat == MediaFormat::HEVC)
                {
                    ErrorCode error =
                        makeHEVCVideoSampleEntryBox(sequence, mAllDecoderConfigs.at(decoderConfig), sampleEntryBox);
                    if (error != ErrorCode::OK)
                    {
                        return error;
                    }
                    stsd.addSampleEntry(std::move(sampleEntryBox));
                }
                else if (sequence.mediaFormat == MediaFormat::AAC)
                {
                    ErrorCode error =
                        makeMP4AudioSampleEntryBox(sequence, mAllDecoderConfigs.at(decoderConfig), sampleEntryBox);
                    if (error != ErrorCode::OK)
                    {
                        return error;
                    }
                    stsd.addSampleEntry(std::move(sampleEntryBox));
                }
            }
        }
        return ErrorCode::OK;
    }

    void WriterImpl::writeEquivalenceSampleGroup(ImageSequence& sequence)
    {
        if (sequence.samples.size() && sequence.containsEquivalenceGroupSamples)
        {
            TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
            SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

            Map<GroupId, Vector<uint32_t>> entityGroupSamples;
            auto equivalenceOffsets = Set<EquivalenceTimeOffset, EquivalenceCompare>();

            for (const auto& sample : sequence.samples)
            {
                for (const auto& entitygroup : sample.equivalenceGroups)
                {
                    entityGroupSamples[entitygroup.first].push_back(
                        sample.sampleIndex);  // sampleIndexes added for debug purposes.
                    equivalenceOffsets.insert(entitygroup.second);
                }
            }

            if (equivalenceOffsets.size())
            {
                UniquePtr<SampleGroupDescriptionBox> sgpd(CUSTOM_NEW(SampleGroupDescriptionBox, ()));
                sgpd->FullBox::setVersion(1);
                sgpd->setGroupingType("eqiv");

                for (const auto& offset : equivalenceOffsets)
                {
                    UniquePtr<VisualEquivalenceEntry, SampleGroupDescriptionEntry> eqivEntry(
                        CUSTOM_NEW(VisualEquivalenceEntry, ()));
                    eqivEntry->setTimeOffset(offset.timeOffset);
                    eqivEntry->setTimescaleMultiplier(offset.timescaleMultiplier);
                    sgpd->setDefaultLength(eqivEntry->getSize());
                    sgpd->addEntry(std::move(eqivEntry));
                }
                stbl.addSampleGroupDescriptionBox(std::move(sgpd));
            }

            for (const auto& group : entityGroupSamples)
            {
                SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();  // creates new SampleToGroupBox for group
                sbgp.setVersion(1);
                sbgp.setGroupingType("eqiv");
                // there can be several group boxes of same type.. this separates them based on GroupId
                sbgp.setGroupingTypeParameter(group.first.get());

                uint32_t runSampleCount           = 0;
                uint32_t runGroupDescriptionIndex = 0;
                for (const auto& sample : sequence.samples)
                {
                    uint32_t sampleGroupDescriptionIndex = 0;  // 0 index = not member of group
                    // is this sample member of this group?
                    if (sample.equivalenceGroups.count(group.first))
                    {
                        uint32_t i = 1;  // index start at 1 for SampleGroupDescriptionBox Entry
                        // find index of SampleGroupDescriptionBox Entry
                        for (const auto& offset : equivalenceOffsets)
                        {
                            if (sample.equivalenceGroups.at(group.first).timeOffset == offset.timeOffset &&
                                sample.equivalenceGroups.at(group.first).timescaleMultiplier ==
                                    offset.timescaleMultiplier)
                            {
                                sampleGroupDescriptionIndex = i;
                                break;
                            }
                            i++;
                        }
                    }

                    // new run?
                    if (runGroupDescriptionIndex != sampleGroupDescriptionIndex)
                    {
                        // end old run if any
                        if (runSampleCount)
                        {
                            sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);
                        }
                        runSampleCount           = 1;
                        runGroupDescriptionIndex = sampleGroupDescriptionIndex;
                    }
                    else
                    {
                        runSampleCount++;
                    }
                }
                // handle last sample if there is run still going
                if (runSampleCount)
                {
                    sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);
                }
            }
        }
    }

    void WriterImpl::writeRefSampleList(ImageSequence& sequence)
    {
        if (sequence.samples.size() && sequence.containsReferenceSamples)
        {
            TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
            SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

            Vector<Vector<std::uint32_t>> refsList;
            for (auto& sample : sequence.samples)
            {
                refsList.push_back(sample.referenceSamples);
            }

            UniquePtr<SampleGroupDescriptionBox> sgpd(CUSTOM_NEW(SampleGroupDescriptionBox, ()));
            SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();

            RefsGroup refsGroup;
            refsGroup.loadRefs(refsList);
            refsGroup.fillSgpd(sgpd.get());
            refsGroup.fillSbgp(sbgp);

            stbl.addSampleGroupDescriptionBox(std::move(sgpd));
        }
    }

    void WriterImpl::writeMetadataItemGroups(ImageSequence& sequence)
    {
        if (sequence.samples.size())
        {
            TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
            SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

            // create unique sample group descriptor entries
            Vector<Set<MetadataItemId>> entries;
            for (const auto& sample : sequence.samples)
            {
                bool found = false;
                for (const auto& existingEntry : entries)
                {
                    if (existingEntry == sample.metadataItemsIds)
                    {  // there is existing entry
                        found = true;
                        break;
                    }
                }

                // new unique entry?
                if (!found && sample.metadataItemsIds.size())
                {
                    entries.push_back(sample.metadataItemsIds);
                }
            }

            // add unique entries to SampleGroupDescriptionBox
            if (entries.size())
            {
                UniquePtr<SampleGroupDescriptionBox> sgpd(CUSTOM_NEW(SampleGroupDescriptionBox, ()));
                sgpd->FullBox::setVersion(1);  // version 1 for DefaultLength support
                sgpd->setGroupingType(STMI_GROUP_TYPE);

                bool canUseDefaultSize = true;
                size_t defaultSize     = 0;
                for (const auto& entry : entries)
                {
                    UniquePtr<SampleToMetadataItemEntry, SampleGroupDescriptionEntry> stmiEntry(
                        CUSTOM_NEW(SampleToMetadataItemEntry, ()));
                    if (canUseDefaultSize && (entry.size() != entries.begin()->size()))
                    {
                        canUseDefaultSize = false;
                        sgpd->setDefaultLength(0);
                    }
                    stmiEntry->setMetaBoxHandlerType(PICT_HANDLER);
                    Vector<uint32_t>& itemIds = stmiEntry->getItemIds();
                    for (const auto& metadataitemId : entry)
                    {
                        itemIds.push_back(metadataitemId.get());
                    }
                    defaultSize = stmiEntry->getSize();
                    sgpd->addEntry(std::move(stmiEntry));
                }

                if (canUseDefaultSize)
                {
                    sgpd->setDefaultLength(static_cast<uint32_t>(defaultSize));
                }
                stbl.addSampleGroupDescriptionBox(std::move(sgpd));


                SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();  // creates new SampleToGroupBox for group
                sbgp.setVersion(0);
                sbgp.setGroupingType("stmi");

                uint32_t runSampleCount           = 0;
                uint32_t runGroupDescriptionIndex = 0;
                for (const auto& sample : sequence.samples)
                {
                    uint32_t sampleGroupDescriptionIndex = 0;  // 0 index = not member of group
                    if (sample.metadataItemsIds.size())        // is this sample member of any group?
                    {
                        uint32_t i = 1;  // index start at 1 for SampleGroupDescriptionBox Entry
                        // find index of SampleGroupDescriptionBox Entry
                        for (const auto& entry : entries)
                        {
                            if (entry == sample.metadataItemsIds)
                            {
                                sampleGroupDescriptionIndex = i;
                                break;
                            }
                            i++;
                        }
                    }

                    // new run?
                    if (runGroupDescriptionIndex != sampleGroupDescriptionIndex)
                    {
                        // end old run if any
                        if (runSampleCount)
                        {
                            sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);
                        }
                        runSampleCount           = 1;
                        runGroupDescriptionIndex = sampleGroupDescriptionIndex;
                    }
                    else
                    {
                        runSampleCount++;
                    }
                }
                // close last sample run
                if (runSampleCount)
                {
                    sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);
                }
            }
        }
    }

    uint64_t WriterImpl::getTrackDuration(TrackBox* track, const ImageSequence& sequence) const
    {
        uint64_t duration;
        DecodePts decodePts;
        const SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const auto editBox         = track->getEditBox();
        const EditList& editList   = sequence.editList;
        const auto movieTimeScale  = 1000;
        const auto mediaTimeScale  = static_cast<uint32_t>(sequence.timeBase.den);

        if (editList.looping && editList.repetitions == 0.0)
        {
            return numeric_limits<uint64_t>::max();
        }

        const EditListBox* editListBox = editBox->getEditListBox();
        assert(editListBox != nullptr);
        decodePts.loadBox(editListBox, movieTimeScale, mediaTimeScale);
        decodePts.loadBox(&stbl.getTimeToSampleBox());
        decodePts.loadBox(stbl.getCompositionOffsetBox().get());
        decodePts.unravel();

        duration = decodePts.getSpan();

        // Set total duration based on repetitions
        if (editList.looping && editList.repetitions > 0.0)
        {
            duration = static_cast<uint64_t>((double) duration * editList.repetitions);
        }

        return duration * movieTimeScale / sequence.timeBase.den;
    }

    EditBox WriterImpl::createEditBox(const EditList& editList) const
    {
        EditBox editBox;

        auto editListBox         = makeCustomShared<EditListBox>();
        EditListBox* editListPtr = editListBox.get();
        editListBox->setVersion(0);

        // (Flags & 1) equal to 0 specifies that the edit list is not repeated,
        // while (flags & 1) equal to 1 specifies that the edit list is repeated.
        if (editList.looping)
        {
            editListBox->setFlags(1);
        }

        for (const auto& editUnit : editList.editUnits)
        {
            if (editUnit.editType == EditType::EMPTY)
            {
                addEmptyEdit(editListPtr, editUnit);
            }
            else if (editUnit.editType == EditType::DWELL)
            {
                addDwellEdit(editListPtr, editUnit);
            }
            else if (editUnit.editType == EditType::SHIFT)
            {
                addShiftEdit(editListPtr, editUnit);
            }
        }
        editBox.setEditListBox(editListBox);

        return editBox;
    }

    void WriterImpl::addEmptyEdit(EditListBox* editListBox, const EditUnit& editUnit) const
    {
        EditListBox::EntryVersion0 editEntry;

        editEntry.mMediaTime         = -1;
        editEntry.mSegmentDuration   = static_cast<uint32_t>(editUnit.durationInMovieTS);  // Assume mvhd timescale 1000
        editEntry.mMediaRateInteger  = 1;
        editEntry.mMediaRateFraction = 0;

        editListBox->addEntry(editEntry);
    }

    void WriterImpl::addDwellEdit(EditListBox* editListBox, const EditUnit& editUnit) const
    {
        EditListBox::EntryVersion0 editEntry;

        editEntry.mMediaTime         = static_cast<int32_t>(editUnit.mediaTimeInTrackTS);
        editEntry.mSegmentDuration   = static_cast<uint32_t>(editUnit.durationInMovieTS);  // Assume mvhd timescale 1000
        editEntry.mMediaRateInteger  = 0;
        editEntry.mMediaRateFraction = 0;

        editListBox->addEntry(editEntry);
    }

    void WriterImpl::addShiftEdit(EditListBox* editListBox, const EditUnit& editUnit) const
    {
        EditListBox::EntryVersion0 editEntry;

        editEntry.mMediaTime         = static_cast<int32_t>(editUnit.mediaTimeInTrackTS);
        editEntry.mSegmentDuration   = static_cast<uint32_t>(editUnit.durationInMovieTS);  // Assume mvhd timescale 1000
        editEntry.mMediaRateInteger  = 1;
        editEntry.mMediaRateFraction = 0;

        editListBox->addEntry(editEntry);
    }


    ErrorCode WriterImpl::setMatrix(const Array<int32_t>& matrix)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (matrix.size == 9)
        {
            mMatrix = vectorize(matrix);
            return ErrorCode::OK;
        }
        return ErrorCode::INVALID_FUNCTION_PARAMETER;
    }

    ErrorCode WriterImpl::setMatrix(const SequenceId& aSequenceId, const Array<int32_t>& matrix)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (!mImageSequences.count(aSequenceId))
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        if (matrix.size == 9)
        {
            mImageSequences.at(aSequenceId).matrix = vectorize(matrix);
            return ErrorCode::OK;
        }

        return ErrorCode::INVALID_FUNCTION_PARAMETER;
    }

    ErrorCode WriterImpl::addVideoTrack(const Rational& aTimeBase, SequenceId& aId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (aTimeBase.den == 0 || aTimeBase.num == 0)
        {
            return ErrorCode::INVALID_FUNCTION_PARAMETER;  // timebase / timebase can't be zero
        }

        uint32_t currentTime = getSecondsSince1904();
        if (!mImageSequences.size())
        {
            mMovieBox.getMovieHeaderBox().setCreationTime(currentTime);
        }

        ImageSequence sequence{};
        sequence.id          = Context::getValue();
        aId                  = sequence.id;
        sequence.trackId     = Track::createTrackId();
        sequence.handlerType = VIDE_HANDLER;
        // sequence.mediaId is filled when first sample is fed to Image Sequence
        sequence.timeBase = aTimeBase;
        // sequence.maxDimensions is filled in finalize() when all DecoderSpecificInfo
        // are gone through
        sequence.duration          = 0;
        sequence.creationTime      = currentTime;
        sequence.modificationTime  = sequence.creationTime;
        sequence.trackEnabled      = true;
        sequence.trackInMovie      = true;
        sequence.trackPreview      = false;
        sequence.containsHidden    = false;
        sequence.alternateGroup    = 0;
        sequence.auxiliaryType     = "";
        sequence.anyNonSyncSample  = false;
        sequence.codingConstraints = {};
        sequence.matrix            = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000};

        mImageSequences[sequence.id] = sequence;

        UniquePtr<TrackBox> trackBox = makeCustomUnique<TrackBox, TrackBox>();
        TrackHeaderBox& trackHeader  = trackBox->getTrackHeaderBox();
        trackHeader.setCreationTime(sequence.creationTime);
        trackHeader.setModificationTime(sequence.modificationTime);
        trackHeader.setTrackID(sequence.trackId.get());

        MediaBox& mediaBox = trackBox->getMediaBox();

        MediaHeaderBox& mediaHeaderBox = mediaBox.getMediaHeaderBox();
        mediaHeaderBox.setCreationTime(sequence.creationTime);
        mediaHeaderBox.setModificationTime(sequence.modificationTime);
        mediaHeaderBox.setTimeScale(static_cast<uint32_t>(sequence.timeBase.den));

        HandlerBox& handlerBox = mediaBox.getHandlerBox();
        handlerBox.setHandlerType(VIDE_HANDLER);
        handlerBox.setName("HEIF/VideoTrack/VideoHandler");

        MediaInformationBox& mediaInformationBox = mediaBox.getMediaInformationBox();
        mediaInformationBox.setMediaType(MediaInformationBox::MediaType::Video);
        DataInformationBox& dinf = mediaInformationBox.getDataInformationBox();
        auto urlBox              = makeCustomShared<DataEntryUrlBox>();
        urlBox->setFlags(1);  // Flag 0x01 tells the data is in this file.
                              // DataEntryUrlBox will write only its header.
        dinf.addDataEntryBox(urlBox);

        mMovieBox.addTrackBox(std::move(trackBox));
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addAudioTrack(const Rational& aTimeBase, const AudioParams& aConfig, SequenceId& aId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (aTimeBase.den == 0 || aTimeBase.num == 0)
        {
            return ErrorCode::INVALID_FUNCTION_PARAMETER;  // timebase / timebase can't be zero
        }

        uint32_t currentTime = getSecondsSince1904();
        if (!mImageSequences.size())
        {
            mMovieBox.getMovieHeaderBox().setCreationTime(currentTime);
        }

        ImageSequence sequence{};
        sequence.id          = Context::getValue();
        aId                  = sequence.id;
        sequence.trackId     = Track::createTrackId();
        sequence.handlerType = SOUN_HANDLER;
        // sequence.mediaId is filled when first sample is fed to Image Sequence
        sequence.timeBase = aTimeBase;
        // sequence.maxDimensions is filled in finalize() when all DecoderSpecificInfo
        // are gone through
        sequence.duration          = 0;
        sequence.creationTime      = currentTime;
        sequence.modificationTime  = sequence.creationTime;
        sequence.trackEnabled      = true;
        sequence.trackInMovie      = true;
        sequence.trackPreview      = false;
        sequence.containsHidden    = false;
        sequence.alternateGroup    = 0;
        sequence.auxiliaryType     = "";
        sequence.anyNonSyncSample  = false;
        sequence.codingConstraints = {};
        sequence.matrix            = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000};
        sequence.audioParams       = aConfig;

        mImageSequences[sequence.id] = sequence;

        UniquePtr<TrackBox> trackBox = makeCustomUnique<TrackBox, TrackBox>();
        TrackHeaderBox& trackHeader  = trackBox->getTrackHeaderBox();
        trackHeader.setCreationTime(sequence.creationTime);
        trackHeader.setModificationTime(sequence.modificationTime);
        trackHeader.setTrackID(sequence.trackId.get());
        trackHeader.setVolume(0x0100);  // = default value 1.0 / full volume

        MediaBox& mediaBox = trackBox->getMediaBox();

        MediaHeaderBox& mediaHeaderBox = mediaBox.getMediaHeaderBox();
        mediaHeaderBox.setCreationTime(sequence.creationTime);
        mediaHeaderBox.setModificationTime(sequence.modificationTime);
        mediaHeaderBox.setTimeScale(static_cast<uint32_t>(sequence.timeBase.den));

        HandlerBox& handlerBox = mediaBox.getHandlerBox();
        handlerBox.setHandlerType(SOUN_HANDLER);
        handlerBox.setName("HEIF/aac-lc/SoundHandler");

        MediaInformationBox& mediaInformationBox = mediaBox.getMediaInformationBox();
        mediaInformationBox.setMediaType(MediaInformationBox::MediaType::Sound);
        DataInformationBox& dinf = mediaInformationBox.getDataInformationBox();
        auto urlBox              = makeCustomShared<DataEntryUrlBox>();
        urlBox->setFlags(1);  // Flag 0x01 tells the data is in this file.
                              // DataEntryUrlBox will write only its header.
        dinf.addDataEntryBox(urlBox);

        mMovieBox.addTrackBox(std::move(trackBox));
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addVideo(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid)
    {
        return addImage(sequenceId, mediaDataId, sampleInfo,
                        sampleid);  // use addImage() as internal functionality for samples is the same.
    }

    ErrorCode WriterImpl::addAudio(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid)
    {
        return addImage(sequenceId, mediaDataId, sampleInfo,
                        sampleid);  // use addImage() as internal functionality for samples is the same.
    }

}  // namespace HEIF
