/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include <limits>

#include "auxiliarytypeinfobox.hpp"
#include "avcsampleentry.hpp"
#include "cleanaperturebox.hpp"
#include "customallocator.hpp"
#include "decodepts.hpp"
#include "elementarystreamdescriptorbox.hpp"
#include "hevcsampleentry.hpp"
#include "mp4audiosampleentrybox.hpp"
#include "rectangularregiongroupentry.hpp"
#include "refsgroup.hpp"
#include "sampletometadataitementry.hpp"
#include "timeutility.hpp"
#include "visualequivalenceentry.hpp"
#include "vvcmixednalunittypepicentry.hpp"

#include "vvcparser.hpp"
#include "vvcsampleentry.hpp"
#include "vvcsubpicidentry.hpp"
#include "vvcsubpicorderentry.hpp"
#include "vvcsubpicsampleentry.hpp"
#include "vvcsubpicturelayoutmapentry.hpp"
#include "writerconstants.hpp"
#include "writerimpl.hpp"

#include "heiferrorinfo.h"
#include "avccommondefs.hpp"
#include "avcconfigurationbox.hpp"
#include "avcdecoderconfigrecord.hpp"
#include "chunkoffsetbox.hpp"
#include "codingconstraintsbox.hpp"
#include "compositionoffsetbox.hpp"
#include "compositiontodecodebox.hpp"
#include "datainformationbox.hpp"
#include "datareferencebox.hpp"
#include "editbox.hpp"
#include "filetypebox.hpp"
#include "fourccint.hpp"
#include "handlerbox.hpp"
#include "heifcommondatatypes.h"
#include "heifwriterdatatypes.h"
#include "hevccommondefs.hpp"
#include "hevcconfigurationbox.hpp"
#include "hevcdecoderconfigrecord.hpp"
#include "idgenerators.hpp"
#include "mediabox.hpp"
#include "mediaheaderbox.hpp"
#include "mediainformationbox.hpp"
#include "moviebox.hpp"
#include "movieheaderbox.hpp"
#include "sampledescriptionbox.hpp"
#include "sampleentrybox.hpp"
#include "samplegroupdescriptionbox.hpp"
#include "samplegroupdescriptionentry.hpp"
#include "samplesizebox.hpp"
#include "sampletablebox.hpp"
#include "sampletochunkbox.hpp"
#include "sampletogroupbox.hpp"
#include "syncsamplebox.hpp"
#include "timetosamplebox.hpp"
#include "trackbox.hpp"
#include "trackgroupbox.hpp"
#include "trackgrouptypebox.hpp"
#include "trackheaderbox.hpp"
#include "trackreferencebox.hpp"
#include "trackreferencetypebox.hpp"
#include "visualsampleentrybox.hpp"
#include "vvccommondefs.hpp"
#include "vvcconfigurationbox.hpp"
#include "vvcdecoderconfigrecord.hpp"
#include "vvcnaluconfigbox.hpp"
#include "writerdatatypesinternal.hpp"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <stddef.h>
#include <stdexcept>
#include <utility>
#include <vector>

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
                    value.numerator   = static_cast<uint32_t>(sequence.clap.horizontalOffsetN);
                    value.denominator = sequence.clap.horizontalOffsetD;
                    clapBox->setHorizOffset(value);
                    value.numerator   = static_cast<uint32_t>(sequence.clap.verticalOffsetN);
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
                        return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "makeAVCVideoSampleEntryBox: makeConfigFromSPS rejected SPS NAL");
                    }
                }
                else
                {
                    return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "makeAVCVideoSampleEntryBox: unexpected/duplicate NAL unit type for AVC track");
                }
            }

            if (!spsFound || !ppsFound)
            {
                return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "makeAVCVideoSampleEntryBox: missing required SPS or PPS NAL unit");
            }

            box->setWidth(decCfg.getPicWidth());
            box->setHeight(decCfg.getPicHeight());
            if (sequence.maxDimensions.height * sequence.maxDimensions.width <
                static_cast<uint32_t>(decCfg.getPicWidth()) * decCfg.getPicHeight())
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
                    if (decCfg.makeConfigFromSPS(nalVector) == false)
                    {
                        return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "makeHEVCVideoSampleEntryBox: makeConfigFromSPS rejected SPS NAL");
                    }
                }
                else
                {
                    return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "makeHEVCVideoSampleEntryBox: unexpected/duplicate NAL unit type for HEVC track");
                }
            }

            if (!spsFound || !ppsFound || !vpsFound)
            {
                return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "makeHEVCVideoSampleEntryBox: missing required SPS, PPS or VPS NAL unit");
            }

            box->setWidth(decCfg.getPicWidth());
            box->setHeight(decCfg.getPicHeight());
            if (sequence.maxDimensions.height * sequence.maxDimensions.width <
                static_cast<uint32_t>(decCfg.getPicWidth()) * decCfg.getPicHeight())
            {
                sequence.maxDimensions.width  = decCfg.getPicWidth();
                sequence.maxDimensions.height = decCfg.getPicHeight();
            }

            cfg.setConfiguration(decCfg);

            fillVisualSampleEntryCommon(sequence, box.get());

            sampleEntryBox = std::move(box);
            return ErrorCode::OK;
        }

// C6262: MSVC /analyze reports ~60 KB stack.  The VvcSampleEntry box
// contains a VvcDecoderConfigurationRecord whose addNalUnit() path
// triggers VPS parsing with spec-mandated fixed-size arrays.  The
// object itself is heap-allocated (makeCustomUnique); the analyser
// counts the transient stack of the called parse chain.  60 KB is
// well within the 1 MB default stack.
#if defined(_MSC_VER)
#pragma warning(suppress : 6262)
#endif
        HEIF::ErrorCode makeVVCVideoSampleEntryBox(ImageSequence& sequence,
                                                           const Array<DecoderSpecificInfo>& nalUnits,
                                                           UniquePtr<SampleEntryBox>& sampleEntryBox)
        {
            auto box = makeCustomUnique<::VvcSampleEntry, ::SampleEntryBox>();

            box->setDataReferenceIndex(1);

            VvcConfigurationBox& cfg = box->getVvcConfigurationBox();

            VvcDecoderConfigurationRecord decCfg;

            bool spsFound = false;
            bool ppsFound = false;

            for (auto& nalUnit : nalUnits)
            {
                const auto nalVector = vectorize(nalUnit.decSpecInfoData);

                if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_PPS && !ppsFound)
                {
                    ppsFound = true;
                    decCfg.addNalUnit(nalVector, VvcNalUnitType::PPS_NUT, true);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_SPS && !spsFound)
                {
                    spsFound = true;
                    decCfg.addNalUnit(nalVector, VvcNalUnitType::SPS_NUT, true);
                    if (decCfg.makeConfigFromSPS(nalVector) == false)
                    {
                        return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "makeVVCVideoSampleEntryBox: makeConfigFromSPS rejected SPS NAL");
                    }
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_VPS)
                {
                    decCfg.addNalUnit(nalVector, VvcNalUnitType::VPS_NUT, true);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_APS_PREFIX)
                {
                    // VVC allows multiple APS NAL units
                    decCfg.addNalUnit(nalVector, VvcNalUnitType::PREFIX_APS_NUT, true);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_APS_SUFFIX)
                {
                    // VVC allows multiple APS NAL units
                    decCfg.addNalUnit(nalVector, VvcNalUnitType::SUFFIX_APS_NUT, true);
                }
                else
                {
                    return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "makeVVCVideoSampleEntryBox: unexpected/duplicate NAL unit type for VVC track");
                }
            }

            if (!spsFound || !ppsFound)
            {
                return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "makeVVCVideoSampleEntryBox: missing required SPS or PPS NAL unit");
            }

            box->setWidth(decCfg.getMaxPictureWidth());
            box->setHeight(decCfg.getMaxPictureHeight());
            if (sequence.maxDimensions.height * sequence.maxDimensions.width <
                static_cast<uint32_t>(decCfg.getMaxPictureWidth()) * decCfg.getMaxPictureHeight())
            {
                sequence.maxDimensions.width  = decCfg.getMaxPictureWidth();
                sequence.maxDimensions.height = decCfg.getMaxPictureHeight();
            }

            cfg.setConfiguration(decCfg);

            fillVisualSampleEntryCommon(sequence, box.get());

            sampleEntryBox = std::move(box);
            return ErrorCode::OK;
        }


// C6262: MSVC /analyze reports ~60 KB stack.  Same root cause as
// makeVVCVideoSampleEntryBox above — VVC decoder config parsing
// with spec-mandated fixed-size OLS/layer arrays in the call chain.
#if defined(_MSC_VER)
#pragma warning(suppress : 6262)
#endif
        HEIF::ErrorCode makeVvcSubpicVideoSampleEntryBox(ImageSequence& sequence,
                                                            const Array<DecoderSpecificInfo>& nalUnits,
                                                            UniquePtr<SampleEntryBox>& sampleEntryBox)
        {
            auto box = makeCustomUnique<::VvcSubpicSampleEntry, ::SampleEntryBox>();

            box->setDataReferenceIndex(1);

            VvcNaluConfigBox& cfg = box->getVvcNaluConfigBox();
            cfg.setLengthSizeMinusOne(3);
            // substitute subpicture track is signaled by 'vvnC' flags=1
            if (sequence.isSubstituteSubpictureTrack)
            {
                cfg.setFlags(1);
            }

            VvcDecoderConfigurationRecord decCfg;

            bool spsFound = false;
            for (auto& nalUnit : nalUnits)
            {
                const auto nalVector = vectorize(nalUnit.decSpecInfoData);

                if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_SPS && !spsFound)
                {
                    spsFound = true;
                    decCfg.addNalUnit(nalVector, VvcNalUnitType::SPS_NUT, true);
                    if (decCfg.makeConfigFromSPS(nalVector) == false)
                    {
                        return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "makeVvcSubpicVideoSampleEntryBox: makeConfigFromSPS rejected SPS NAL");
                    }
                }
            }
            if (!spsFound)
            {
                return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "makeVvcSubpicVideoSampleEntryBox: missing required SPS NAL unit");
            }

            box->setWidth(decCfg.getMaxPictureWidth());
            box->setHeight(decCfg.getMaxPictureHeight());
            if (sequence.maxDimensions.height * sequence.maxDimensions.width <
                static_cast<uint32_t>(decCfg.getMaxPictureWidth()) * decCfg.getMaxPictureHeight())
            {
                sequence.maxDimensions.width  = decCfg.getMaxPictureWidth();
                sequence.maxDimensions.height = decCfg.getMaxPictureHeight();
            }

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
                    return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "makeMP4AudioSampleEntryBox: missing AudioSpecificConfig");
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
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addImageSequence: called before initialize()");
        }

        if (aTimeBase.den == 0 || aTimeBase.num == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addImageSequence: invalid argument");
        }

        uint32_t currentTime = static_cast<uint32_t>(TimeUtility::getSecondsSince1904());
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
        sequence.subpicId          = 0;

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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addImageSequence: exception escaped firewall")
    }

    ErrorCode WriterImpl::addImage(const SequenceId& aSequenceId,
                                   const MediaDataId& aMediaDataId,
                                   const SampleInfo& aSampleInfo,
                                   SequenceImageId& aSequenceImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addImage(SampleInfo): called before initialize()");
        }

        if (!mImageSequences.count(aSequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addImage(SampleInfo): sequence (track) id not found");
        }
        if (!mMediaData.count(aMediaDataId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_MEDIADATA_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addImage(SampleInfo): media-data id not found");
        }
        if (aSampleInfo.duration == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addImage(SampleInfo): invalid argument");
        }

        ImageSequence& sequence = mImageSequences.at(aSequenceId);
        if (sequence.samples.size() && mMediaData.at(aMediaDataId).mediaFormat != sequence.mediaFormat)
        {  // do not allow mediaData from different media formats
            return HEIF_FAIL(ErrorCode::INVALID_MEDIA_FORMAT,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addImage(SampleInfo): unrecognised or unsupported media format");
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
                return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                                 ErrorOrigin::Writer_BoxBuild,
                                 "WriterImpl::addImage(SampleInfo): sequence image id not found");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addImage(SampleInfo): exception escaped firewall")
    }

    ErrorCode WriterImpl::addMetadataItemReference(const MetadataItemId& metadataItemId,
                                                   const SequenceId& sequenceId,
                                                   const SequenceImageId& sequenceImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addMetadataItemReference: called before initialize()");
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
            return HEIF_FAIL(ErrorCode::INVALID_METADATAITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addMetadataItemReference: metadata item id not found");
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

        return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                         ErrorOrigin::Writer_BoxBuild,
                         "WriterImpl::addMetadataItemReference: sequence image id not found");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addMetadataItemReference: exception escaped firewall")
    }

    ErrorCode WriterImpl::addThumbnails(const SequenceId& thumbSequenceId, const SequenceId& sequenceId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addThumbnails: called before initialize()");
        }

        if (!mImageSequences.count(sequenceId) || !mImageSequences.count(thumbSequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addThumbnails: sequence (track) id not found");
        }

        ImageSequence& thumpSequence = mImageSequences.at(thumbSequenceId);
        ImageSequence& imageSequence = mImageSequences.at(sequenceId);
        if (thumpSequence.handlerType != PICT_HANDLER && imageSequence.handlerType != PICT_HANDLER)
        {
            // as per "7.5.2 Thumbnail image sequence track" only 'pict' allowed
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addThumbnails: sequence (track) id not found");
        }

        // Add tracks to same Alternate Group
        if (imageSequence.alternateGroup.get() == 0)
        {  // create new
            imageSequence.alternateGroup = Track::createAlternateGroupId();
        }
        thumpSequence.alternateGroup = imageSequence.alternateGroup;

        // Add track reference from Thumb track to its image track.
        thumpSequence.trackReferences[THUMB_TREF_TYPE].push_back(imageSequence.trackId);
        // flag thumb track as preview
        thumpSequence.trackPreview = true;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addThumbnails: exception escaped firewall")
    }

    ErrorCode WriterImpl::setImageHidden(const SequenceImageId& sequenceImageId, const bool hidden)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setImageHidden: called before initialize()");
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
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setImageHidden: sequence image id not found");
        }
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setImageHidden: exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const CleanAperture& clap, const SequenceId& sequenceId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty: called before initialize()");
        }

        if (!mImageSequences.count(sequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addProperty: sequence (track) id not found");
        }

        ImageSequence& sequence = mImageSequences.at(sequenceId);
        if (sequence.handlerType == SOUN_HANDLER)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addProperty: sequence (track) id not found");
        }

        sequence.containsCleanApertureBox = true;
        sequence.clap                     = clap;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty: exception escaped firewall")
    }

    ErrorCode WriterImpl::addAuxiliaryReference(const AuxiliaryType& auxC,
                                                const SequenceId& auxiliarySequenceId,
                                                const SequenceId& sequenceId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addAuxiliaryReference: called before initialize()");
        }

        if (!mImageSequences.count(sequenceId) || !mImageSequences.count(auxiliarySequenceId) ||
            mImageSequences.at(sequenceId).handlerType == SOUN_HANDLER)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addAuxiliaryReference: sequence (track) id not found");
        }

        ImageSequence& auxlSequence  = mImageSequences.at(auxiliarySequenceId);
        ImageSequence& imageSequence = mImageSequences.at(sequenceId);

        // Add track reference from Auxl track to its image track.
        auxlSequence.trackReferences[AUXL_TREF_TYPE].push_back(imageSequence.trackId);
        // Auxl track shouldn't be played as such:
        auxlSequence.trackInMovie = false;
        // auxiliary track should have handler type 'auxv'
        mMovieBox.getTrackBox(auxlSequence.trackId.get())->getMediaBox().getHandlerBox().setHandlerType(AUXV_HANDLER);

        // set AuxReferenceType:
        auxlSequence.auxiliaryType = String(auxC.auxType.begin(), auxC.auxType.end());
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addAuxiliaryReference: exception escaped firewall")
    }

    ErrorCode WriterImpl::setAlternateGrouping(const SequenceId& sequenceId1, const SequenceId& sequenceId2)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setAlternateGrouping: called before initialize()");
        }

        if (!mImageSequences.count(sequenceId1) || !mImageSequences.count(sequenceId2))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setAlternateGrouping: sequence (track) id not found");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setAlternateGrouping: exception escaped firewall")
    }

    /* *************************************************************** */
    /* *********************** private methods *********************** */
    /* *************************************************************** */
    ErrorCode WriterImpl::generateMoovBox()
    {
        HEIF_TRY
        {
        uint32_t modificationTime = static_cast<uint32_t>(TimeUtility::getSecondsSince1904());
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

                if (sequence.mediaFormat ==
                    MediaFormat::VVC_SUBPIC)  // Add VvcSubpicOrderEntry for VVC subpicture track
                {
                    writeVvcSubpicIdEntry(sequence);
                }

                if (sequence.isBaseTrack)
                {
                    writeVvcSubpicOrderEntry(sequence);
                    writeRectangularRegionGroupEntry(sequence);
                    writeVvcSubpictureLayoutMapEntry(sequence);
                    ErrorCode error = writeVvcMixedNalUnitTypePicEntry(sequence);
                    if (error != ErrorCode::OK)
                    {
                        return error;
                    }
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

            writeTrackGroups(sequence);
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::generateMoovBox: exception escaped firewall")
    }

    void WriterImpl::writeTrackGroups(ImageSequence& imageSequence)
    {
        TrackBox* track = mMovieBox.getTrackBox(imageSequence.trackId.get());
        for (const auto& it : mTrackGroups)
        {
            const TrackGroup& trackGroup = it.second;
            if (trackGroup.trackIds.count(imageSequence.id))
            {
                TrackGroupTypeBox typebox(trackGroup.type.value, trackGroup.id.get());
                if (trackGroup.type == "alte")
                {
                    typebox.setFlags(1);
                }
                track->getTrackGroupBox().addTrackGroupTypeBox(typebox);
                track->setHasTrackGroup(true);
            }
        }
    }

    ErrorCode WriterImpl::addToGroup(const GroupId& groupId, const SequenceId& id)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addToGroup: called before initialize()");
        }

        if (mImageSequences.count(id) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addToGroup: sequence (track) id not found");
        }

        if (mEntityGroups.count(groupId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_GROUP_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addToGroup: group id not found");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addToGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::addToEquivalenceGroup(const GroupId& equivalenceGroupId,
                                                const SequenceId& sequenceId,
                                                const SequenceImageId& id,
                                                const EquivalenceTimeOffset& offset)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addToEquivalenceGroup: called before initialize()");
        }

        if (mEntityGroups.count(equivalenceGroupId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_GROUP_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addToEquivalenceGroup: group id not found");
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

        return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                         ErrorOrigin::Writer_BoxBuild,
                         "WriterImpl::addToEquivalenceGroup: sequence image id not found");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addToEquivalenceGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::updateMoovBox(uint64_t mdatOffset)
    {
        HEIF_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::updateMoovBox: exception escaped firewall")
    }

    ErrorCode WriterImpl::setEditList(const SequenceId& sequenceId, const EditList& editList)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setEditList: called before initialize()");
        }

        if (mImageSequences.count(sequenceId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setEditList: sequence (track) id not found");
        }

        // Edit list must have one or more entries.
        if (editList.editUnits.size == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::setEditList: invalid argument");
        }

        mImageSequences.at(sequenceId).editList = editList;

        const auto trackId = mImageSequences.at(sequenceId).trackId;
        auto trackBox      = mMovieBox.getTrackBox(trackId.get());
        // const auto mediaTimescale = trackBox->getMediaBox().getMediaHeaderBox().getTimeScale();
        auto editBox = createEditBox(editList);

        trackBox->setEditBox(editBox);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setEditList: exception escaped firewall")
    }

    /* *************************************************************** */
    /* ******************* Internal helper methods ******************* */
    /* *************************************************************** */

    ErrorCode WriterImpl::isValidSequenceImage(const SequenceId& sequenceId,
                                               const SequenceImageId& sequenceImageId) const
    {
        HEIF_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::isValidSequenceImage: called before initialize()");
        }
        if (!mImageSequences.count(sequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::isValidSequenceImage: sequence (track) id not found");
        }

        for (auto& sample : mImageSequences.at(sequenceId).samples)
        {
            if (sample.sequenceImageId == sequenceImageId)
            {
                return ErrorCode::OK;
            }
        }
        return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                         ErrorOrigin::Writer_BoxBuild,
                         "WriterImpl::isValidSequenceImage: sequence image id not found");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::isValidSequenceImage: exception escaped firewall")
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
        HEIF_TRY
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
            bool isNegativeCompositionOffsetPresent = false;

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
                if (sampleData.offset != nextSampleOffset ||  // if next sample doesn't start where we expect
                    (chunks.size() && (chunks.back().sampleDescriptionIndex != sample.decoderConfigIndex)) ||
                    (chunks.size() && (sequence.anyNonSyncSample == true) && sample.isSyncSample))
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
                    compositionOffsets.emplace_back(1, sample.compositionOffset);
                    if (sample.compositionOffset < 0)
                    {
                        isNegativeCompositionOffsetPresent = true;
                    }
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
                if (sequence.containsHidden || isNegativeCompositionOffsetPresent)
                {
                    ctts.setVersion(1);
                    for (auto ct : compositionOffsets)
                    {
                        if ((ct.second < std::numeric_limits<std::int32_t>::min()) ||
                            (ct.second > std::numeric_limits<std::int32_t>::max()))
                        {
                            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                                             ErrorOrigin::Writer_Validation,
                                             "WriterImpl::writeMoovSampleTable: invalid argument");
                        }
                        ctts.addCompositionOffsetEntryVersion1({ct.first, static_cast<int32_t>(ct.second)});
                    }
                }
                else
                {
                    for (auto ct : compositionOffsets)
                    {
                        if ((ct.second < std::numeric_limits<std::uint32_t>::min()) ||
                            (ct.second > std::numeric_limits<std::uint32_t>::max()))
                        {
                            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                                             ErrorOrigin::Writer_Validation,
                                             "WriterImpl::writeMoovSampleTable: invalid argument");
                        }
                        ctts.addCompositionOffsetEntryVersion0({ct.first, static_cast<uint32_t>(ct.second)});
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
                else if (sequence.mediaFormat == MediaFormat::VVC)
                {
                    ErrorCode error =
                        makeVVCVideoSampleEntryBox(sequence, mAllDecoderConfigs.at(decoderConfig), sampleEntryBox);
                    if (error != ErrorCode::OK)
                    {
                        return error;
                    }
                    stsd.addSampleEntry(std::move(sampleEntryBox));
                }
                else if (sequence.mediaFormat == MediaFormat::VVC_SUBPIC)
                {
                    ErrorCode error = makeVvcSubpicVideoSampleEntryBox(sequence, mAllDecoderConfigs.at(decoderConfig),
                                                                       sampleEntryBox);
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::writeMoovSampleTable: exception escaped firewall")
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
                auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();
                sgpd->setVersion(1);
                sgpd->setGroupingType("eqiv");

                for (const auto& offset : equivalenceOffsets)
                {
                    auto eqivEntry =
                        makeCustomUnique<VisualEquivalenceEntry, SampleGroupDescriptionEntry>();
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

            auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();
            SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();

            RefsGroup refsGroup;
            refsGroup.loadRefs(refsList);
            refsGroup.fillSgpd(sgpd.get());
            refsGroup.fillSbgp(sbgp);

            stbl.addSampleGroupDescriptionBox(std::move(sgpd));
        }
    }

    ErrorCode WriterImpl::writeVvcSubpictureLayoutMapEntry(ImageSequence& sequence)
        {
        HEIF_TRY
        {
        if (sequence.sulmEntries.size() == 0)
        {
            return ErrorCode::OK;
        }

        TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
        SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

        auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();

        const uint32_t runSampleCount     = static_cast<uint32_t>(sequence.samples.size());
        uint32_t runGroupDescriptionIndex = 1;
        for (const auto& sulmInfo : sequence.sulmEntries)
        {
            auto sulm =
                makeCustomUnique<VvcSubpictureLayoutMapEntry, SampleGroupDescriptionEntry>();

            sulm->setGroupIdInfo4CC(sulmInfo.groupIdInfo.value);
            const vector<uint16_t> ids(sulmInfo.groupIds.begin(), sulmInfo.groupIds.end());
            sulm->setGroupIds(ids);

            SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();  // creates new SampleToGroupBox for group
            sbgp.setGroupingType("sulm");
            sbgp.setVersion(1);
            sbgp.setGroupingTypeParameter(
                runGroupDescriptionIndex);  // could be something else too, just need unique parameter here as there can
                                            // be several sulm entries
            sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);

            sgpd->setVersion(1);  // version 1 for DefaultLength support
            sgpd->setDefaultLength(sulm->getSize());
            sgpd->setGroupingType("sulm");
            sgpd->addEntry(std::move(sulm));

            runGroupDescriptionIndex++;
        }

        stbl.addSampleGroupDescriptionBox(std::move(sgpd));

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::writeVvcSubpictureLayoutMapEntry: exception escaped firewall")
    }

    ErrorCode WriterImpl::writeRectangularRegionGroupEntry(ImageSequence& sequence)
    {
        HEIF_TRY
        {
        if (sequence.trifEntries.size() == 0)
        {
            return ErrorCode::OK;
        }

        TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
        SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

        auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();

        const uint32_t runSampleCount     = static_cast<uint32_t>(sequence.samples.size());
        uint32_t runGroupDescriptionIndex = 1;
        for (const auto& trifInfo : sequence.trifEntries)
        {
            auto trif =
                makeCustomUnique<RectangularRegionGroupEntry, SampleGroupDescriptionEntry>();

            trif->setGroupId(trifInfo.groupId);
            trif->setRectRegionFlag(trifInfo.rectRegionFlag);
            trif->setIndependentIdc(static_cast<uint8_t>(trifInfo.independentIdc));
            trif->setFilteringDisabled(trifInfo.filteringDisabled);
            trif->setHasDependencyList(trifInfo.dependencyRectRegionGroupIds.size > 0);
            trif->setHorizontalOffset(trifInfo.horizontalOffset);
            trif->setVerticalOffset(trifInfo.verticalOffset);
            trif->setRegionWidth(trifInfo.regionWidth);
            trif->setRegionHeight(trifInfo.regionHeight);
            const vector<uint16_t> ids(trifInfo.dependencyRectRegionGroupIds.begin(),
                                       trifInfo.dependencyRectRegionGroupIds.end());
            trif->setDependencyRectRegionGroupIds(ids);

            SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();  // creates new SampleToGroupBox for group
            sbgp.setGroupingType("trif");
            sbgp.setVersion(1);
            sbgp.setGroupingTypeParameter(
                runGroupDescriptionIndex);  // could be something else too, just need unique parameter here as there can
                                            // be several trif entries
            sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);

            sgpd->setVersion(1);  // version 1 for DefaultLength support
            sgpd->setDefaultLength(trif->getSize());
            sgpd->setGroupingType("trif");
            sgpd->addEntry(std::move(trif));

            runGroupDescriptionIndex++;
        }


        stbl.addSampleGroupDescriptionBox(std::move(sgpd));

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::writeRectangularRegionGroupEntry: exception escaped firewall")
    }

    ErrorCode WriterImpl::writeVvcSubpicOrderEntry(ImageSequence& sequence)
    {
        HEIF_TRY
        {
        TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
        SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

        auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();
        auto spor = makeCustomUnique<VvcSubpicOrderEntry, SampleGroupDescriptionEntry>();
        spor->setSubpicIdInfoFlag(true);

        // Set subp_track_ref_idx of 'spor'.
        {
            const auto& decodingOrder = sequence.subpictureTrackDecodingOrder;
            const bool allZeros =
                std::all_of(decodingOrder.cbegin(), decodingOrder.cend(), [](unsigned int i) { return i == 0; });
            if (!allZeros)
            {
                const auto subpTrefCount = sequence.trackReferences.at(SUBP_TREF_TYPE).size();
                if (subpTrefCount != decodingOrder.size())
                {
                    return HEIF_FAIL(ErrorCode::NOT_APPLICABLE,
                                     ErrorOrigin::Writer_State,
                                     "WriterImpl::writeVvcSubpicOrderEntry: operation not valid for this file/state");
                }
                std::vector<std::uint16_t> subpTrackRefIdx;
                std::set<std::uint16_t> foundIndices;
                for (const auto subpTrefIndex : decodingOrder)
                {
                    if ((foundIndices.count(static_cast<uint16_t>(subpTrefIndex)) != 0) || (subpTrefIndex == 0) ||
                        (subpTrefIndex > subpTrefCount + 1))
                    {
                        return HEIF_FAIL(ErrorCode::NOT_APPLICABLE,
                                         ErrorOrigin::Writer_State,
                                         "WriterImpl::writeVvcSubpicOrderEntry: operation not valid for this file/state");
                    }
                    foundIndices.insert(static_cast<uint16_t>(subpTrefIndex));
                    subpTrackRefIdx.push_back(static_cast<uint16_t>(subpTrefIndex));
                }
                spor->setSubpicRefIdx(subpTrackRefIdx);
            }
        }

        for (const auto& configId : sequence.decoderConfigs)
        {
            // Prefer the cached SPS/PPS info captured at feedDecoderConfig()
            // time -- this avoids invoking VvcParser::parseSps / parsePps
            // again over bytes we have already parsed once. The legacy parse
            // path is kept as a fallback for entries that failed to cache
            // (e.g. parser threw at feed time).
            const auto cacheIt = mVvcParsedConfigs.find(configId);
            const VvcParsedConfig* cached =
                (cacheIt != mVvcParsedConfigs.end()) ? &cacheIt->second : nullptr;

            for (const auto& decoderInfo : mAllDecoderConfigs.at(configId))
            {
                if (decoderInfo.decSpecInfoType == DecoderSpecInfoType::VVC_PPS)
                {
                    if (cached != nullptr && cached->hasPps)
                    {
                        const auto& info = cached->pps;
                        if (info.isSubpicMappingPresent)
                        {
                            spor->setPpsSpsSubpicIdFlag(true);
                            spor->setPpsId(static_cast<uint8_t>(info.ppsId));
                            spor->setSubpicIdLenMinus1(static_cast<uint16_t>(info.subpicIdLen - 1));
                            spor->setSubpicIdBitPos(static_cast<uint16_t>(info.subpicIdBitPos));
                            spor->setStartCodeEmulFlag(info.startCodeEmulationFlag);
                        }
                    }
                    else
                    {
                        const auto nalVector = vectorize(decoderInfo.decSpecInfoData);
                        const auto info = VvcParser::parsePps(nalVector);
                        if (info.isSubpicMappingPresent)
                        {
                            spor->setPpsSpsSubpicIdFlag(true);
                            spor->setPpsId(static_cast<uint8_t>(info.ppsId));
                            spor->setSubpicIdLenMinus1(static_cast<uint16_t>(info.subpicIdLen - 1));
                            spor->setSubpicIdBitPos(static_cast<uint16_t>(info.subpicIdBitPos));
                            spor->setStartCodeEmulFlag(info.startCodeEmulationFlag);
                        }
                    }
                }
                else if (decoderInfo.decSpecInfoType == DecoderSpecInfoType::VVC_SPS)
                {
                    if (cached != nullptr && cached->hasSps)
                    {
                        const auto& info = cached->sps;
                        spor->setSpsId(static_cast<uint8_t>(info.spsId));
                        spor->setSubpicIdLenMinus1(static_cast<uint16_t>(info.subpicIdLen - 1));
                        spor->setSubpicIdBitPos(static_cast<uint16_t>(info.subpicIdBitPos));
                        spor->setStartCodeEmulFlag(info.startCodeEmulationFlag);
                    }
                    else
                    {
                        const auto nalData = vectorize(decoderInfo.decSpecInfoData);
                        const auto info = VvcParser::parseSps(nalData);
                        spor->setSpsId(static_cast<uint8_t>(info.spsId));
                        spor->setSubpicIdLenMinus1(static_cast<uint16_t>(info.subpicIdLen - 1));
                        spor->setSubpicIdBitPos(static_cast<uint16_t>(info.subpicIdBitPos));
                        spor->setStartCodeEmulFlag(info.startCodeEmulationFlag);
                    }
                }
            }
        }

        sgpd->setVersion(1);  // version 1 for DefaultLength support
        sgpd->setDefaultLength(spor->getSize());
        sgpd->setGroupingType("spor");
        sgpd->addEntry(std::move(spor));
        stbl.addSampleGroupDescriptionBox(std::move(sgpd));

        SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();  // creates new SampleToGroupBox for group
        sbgp.setGroupingType("spor");

        uint32_t runSampleCount           = static_cast<uint32_t>(sequence.samples.size());
        uint32_t runGroupDescriptionIndex = 1;
        sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::writeVvcSubpicOrderEntry: exception escaped firewall")
    }

    ErrorCode WriterImpl::writeVvcMixedNalUnitTypePicEntry(ImageSequence& sequence)
    {
        HEIF_TRY
        {
        if (sequence.minpGroupings.size() == 0)
        {
            return ErrorCode::OK;
        }

        TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
        SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

        for (const auto& mixedNal : sequence.minpGroupings)
        {
            // sgpd / minp must be constructed fresh per iteration: the previous
            // iteration std::move()'s both into stbl / sgpd at the bottom of the
            // body, so reusing the outer-scope objects on a 2nd+ iteration would
            // dereference a moved-from UniquePtr (caught by clang-tidy
            // bugprone-use-after-move). In practice minpGroupings is usually
            // size <= 1 so the bug had never surfaced; this fix is defensive.
            auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();
            auto minp =
                makeCustomUnique<VvcMixedNalUnitTypePicEntry, SampleGroupDescriptionEntry>();

            const auto ppsId = mixedNal.second.ppsId;

            minp->setPpsId(static_cast<uint8_t>(ppsId));

            Vector<VvcMixedNalUnitTypePicEntry::MixSubpTrackIdxs> minpIds;
            minpIds.reserve(mixedNal.second.mixedNalUnitInfos.size());
            for (const auto& i : mixedNal.second.mixedNalUnitInfos)
            {
                minpIds.emplace_back(i.first.get(), i.second.get());
            }
            minp->setMixSubpTrackIds(minpIds);
            // flag position is fixed, as there is no optional fields before it
            minp->setPpsMixNaluTypesInPicBitPos(10);

            sgpd->setVersion(1);  // version 1 for DefaultLength support
            sgpd->setDefaultLength(minp->getSize());
            sgpd->setGroupingType(MINP_GROUP_TYPE);
            sgpd->addEntry(std::move(minp));
            stbl.addSampleGroupDescriptionBox(std::move(sgpd));
        }

        SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();  // creates new SampleToGroupBox for group
        sbgp.setVersion(0);
        sbgp.setGroupingType(MINP_GROUP_TYPE);

        uint32_t runSampleCount           = 0;
        uint32_t runGroupDescriptionIndex = 0;
        for (const auto& sample : sequence.samples)
        {
            uint32_t sampleGroupDescriptionIndex = 0;  // 0 index = not member of group
            if (sample.mixedNalUnitTypeIds.size())     // is this sample member of any group?
            {
                uint32_t i = 1;  // index start at 1 for SampleGroupDescriptionBox Entry
                // find index of SampleGroupDescriptionBox Entry
                for (const auto& entry : sequence.minpGroupings)
                {
                    if (entry.first == *sample.mixedNalUnitTypeIds.begin())
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

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::writeVvcMixedNalUnitTypePicEntry: exception escaped firewall")
    }


    void WriterImpl::writeVvcSubpicIdEntry(ImageSequence& sequence)
    {
        TrackBox* track      = mMovieBox.getTrackBox(sequence.trackId.get());
        SampleTableBox& stbl = track->getMediaBox().getMediaInformationBox().getSampleTableBox();

        auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();
        auto spid = makeCustomUnique<VvcSubpicIdEntry, SampleGroupDescriptionEntry>();
        spid->setSubPicIds({sequence.subpicId});
        sgpd->setVersion(1);  // version 1 for DefaultLength support
        sgpd->setDefaultLength(spid->getSize());
        sgpd->setGroupingType("spid");
        sgpd->addEntry(std::move(spid));
        stbl.addSampleGroupDescriptionBox(std::move(sgpd));

        SampleToGroupBox& sbgp = stbl.getSampleToGroupBox();  // creates new SampleToGroupBox for group

        sbgp.setGroupingType("spid");

        uint32_t runSampleCount           = static_cast<uint32_t>(sequence.samples.size());
        uint32_t runGroupDescriptionIndex = 1;
        sbgp.addSampleRun(runSampleCount, runGroupDescriptionIndex);
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
                auto sgpd = makeCustomUnique<SampleGroupDescriptionBox, SampleGroupDescriptionBox>();
                sgpd->setVersion(1);  // version 1 for DefaultLength support
                sgpd->setGroupingType(STMI_GROUP_TYPE);

                bool canUseDefaultSize = true;
                size_t defaultSize     = 0;
                for (const auto& entry : entries)
                {
                    auto stmiEntry =
                        makeCustomUnique<SampleToMetadataItemEntry, SampleGroupDescriptionEntry>();
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
        if (editListBox == nullptr)
        {
            throw std::runtime_error("getTrackDurationTS: EditBox present but EditListBox is null");
        }
        decodePts.loadBox(editListBox, movieTimeScale, mediaTimeScale);
        decodePts.loadBox(&stbl.getTimeToSampleBox());
        decodePts.loadBox(stbl.getCompositionOffsetBox().get());
        decodePts.unravel();

        duration = decodePts.getSpan();

        // Set total duration based on repetitions
        if (editList.looping && editList.repetitions > 0.0)
        {
            duration = static_cast<uint64_t>(static_cast<double>(duration) * editList.repetitions);
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
            else if (editUnit.editType == EditType::RAW)
            {
                addRawEdit(editListPtr, editUnit);
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

    void WriterImpl::addRawEdit(EditListBox* editListBox, const EditUnit& editUnit) const
    {
        EditListBox::EntryVersion0 editEntry;

        editEntry.mMediaTime         = static_cast<int32_t>(editUnit.mediaTimeInTrackTS);
        editEntry.mSegmentDuration   = static_cast<uint32_t>(editUnit.durationInMovieTS);  // Assume mvhd timescale 1000
        editEntry.mMediaRateInteger  = editUnit.mediaRateInteger;
        editEntry.mMediaRateFraction = editUnit.mediaRateFraction;

        editListBox->addEntry(editEntry);
    }


    ErrorCode WriterImpl::setMatrix(const Array<int32_t>& matrix)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setMatrix: called before initialize()");
        }

        if (matrix.size == 9)
        {
            mMatrix = vectorize(matrix);
            return ErrorCode::OK;
        }
        return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                         ErrorOrigin::Writer_Validation,
                         "WriterImpl::setMatrix: invalid argument");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setMatrix: exception escaped firewall")
    }

    ErrorCode WriterImpl::setMatrix(const SequenceId& aSequenceId, const Array<int32_t>& matrix)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setMatrix: called before initialize()");
        }

        if (!mImageSequences.count(aSequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setMatrix: sequence (track) id not found");
        }

        if (matrix.size == 9)
        {
            mImageSequences.at(aSequenceId).matrix = vectorize(matrix);
            return ErrorCode::OK;
        }

        return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                         ErrorOrigin::Writer_Validation,
                         "WriterImpl::setMatrix: invalid argument");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setMatrix: exception escaped firewall")
    }

    ErrorCode WriterImpl::addSubpictureTrack(const SequenceId& aBaseTrackSequenceId,
                                             const unsigned int decodingOrder,
                                             SequenceId& aId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addSubpictureTrack: called before initialize()");
        }

        if (!mImageSequences.count(aBaseTrackSequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addSubpictureTrack: sequence (track) id not found");
        }

        const uint32_t currentTime = static_cast<uint32_t>(TimeUtility::getSecondsSince1904());
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
        sequence.timeBase = mImageSequences[aBaseTrackSequenceId].timeBase;
        // sequence.maxDimensions is filled in finalize() when all DecoderSpecificInfo
        // are gone through
        sequence.duration          = 0;
        sequence.creationTime      = currentTime;
        sequence.modificationTime  = sequence.creationTime;
        sequence.trackEnabled      = true;
        sequence.trackInMovie      = false;  // must be false for 'vvs1' sample entry VVC subpicture track
        sequence.trackPreview      = false;
        sequence.containsHidden    = false;
        sequence.alternateGroup    = 0;
        sequence.auxiliaryType     = "";
        sequence.anyNonSyncSample  = false;
        sequence.codingConstraints = {};
        sequence.matrix            = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000};
        sequence.subpicId          = mNextSubpicId;
        mNextSubpicId++;

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
        handlerBox.setName("VideoTrack/VideoHandler");

        MediaInformationBox& mediaInformationBox = mediaBox.getMediaInformationBox();
        mediaInformationBox.setMediaType(MediaInformationBox::MediaType::Video);
        DataInformationBox& dinf = mediaInformationBox.getDataInformationBox();
        auto urlBox              = makeCustomShared<DataEntryUrlBox>();
        urlBox->setFlags(1);  // Flag 0x01 tells the data is in this file.
                              // DataEntryUrlBox will write only its header.
        dinf.addDataEntryBox(urlBox);

        mMovieBox.addTrackBox(std::move(trackBox));

        mImageSequences.at(aBaseTrackSequenceId).subpictureTrackDecodingOrder.push_back(decodingOrder);
        mImageSequences.at(aBaseTrackSequenceId).isBaseTrack = true;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addSubpictureTrack: exception escaped firewall")
    }

    ErrorCode WriterImpl::setSubstituteSubpictureTrack(const SequenceId& sequence, bool isSubstituteSubpictureTrack)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setSubstituteSubpictureTrack: called before initialize()");
        }

        if (!mImageSequences.count(sequence))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setSubstituteSubpictureTrack: sequence (track) id not found");
        }

        mImageSequences.at(sequence).isSubstituteSubpictureTrack = isSubstituteSubpictureTrack;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setSubstituteSubpictureTrack: exception escaped firewall")
    }

    ErrorCode WriterImpl::addVideoTrack(const Rational& aTimeBase, SequenceId& aId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addVideoTrack(Rational): called before initialize()");
        }

        if (aTimeBase.den == 0 || aTimeBase.num == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addVideoTrack(Rational): invalid argument");
        }

        uint32_t currentTime = static_cast<uint32_t>(TimeUtility::getSecondsSince1904());
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
                sequence.subpicId          = 0;

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
        handlerBox.setName("VideoTrack/VideoHandler");

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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addVideoTrack(Rational): exception escaped firewall")
    }

    ErrorCode WriterImpl::addAudioTrack(const Rational& aTimeBase, const AudioParams& aConfig, SequenceId& aId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addAudioTrack(AudioParams): called before initialize()");
        }

        if (aTimeBase.den == 0 || aTimeBase.num == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addAudioTrack(AudioParams): invalid argument");
        }

        uint32_t currentTime = static_cast<uint32_t>(TimeUtility::getSecondsSince1904());
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
        sequence.subpicId          = 0;

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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addAudioTrack(AudioParams): exception escaped firewall")
    }

    ErrorCode WriterImpl::addVideo(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid)
    {
        HEIF_ENTRY_TRY
        {
        return addImage(sequenceId, mediaDataId, sampleInfo,
                        sampleid);  // use addImage() as internal functionality for samples is the same.
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addVideo: exception escaped firewall")
    }

    ErrorCode WriterImpl::addAudio(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid)
    {
        HEIF_ENTRY_TRY
        {
        return addImage(sequenceId, mediaDataId, sampleInfo,
                        sampleid);  // use addImage() as internal functionality for samples is the same.
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addAudio: exception escaped firewall")
    }

    ErrorCode WriterImpl::addMixedNalTypeGrouping(const SequenceId& baseTrackId,
                                                  const unsigned int ppsId,
                                                  MixedNalUnitTypeId& minpId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addMixedNalTypeGrouping: called before initialize()");
        }

        if (!mImageSequences.count(baseTrackId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addMixedNalTypeGrouping: sequence (track) id not found");
        }

        minpId = Context::getValue();
        MinpVisualSampleGrouping minp;
        minp.ppsId  = ppsId;
        auto& minps = mImageSequences.at(baseTrackId).minpGroupings;

        minps[minpId] = minp;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addMixedNalTypeGrouping: exception escaped firewall")
    }

    ErrorCode WriterImpl::addMixedNalGrouping(const MixedNalUnitTypeId& minpId, const SequenceImageId& sequenceImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addMixedNalGrouping: called before initialize()");
        }

        auto sequenceIt = std::find_if(
            mImageSequences.begin(), mImageSequences.end(),
            [minpId](const pair<SequenceId, ImageSequence>& i) { return i.second.minpGroupings.count(minpId); });
        if (sequenceIt == mImageSequences.end())
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addMixedNalGrouping: invalid argument");
        }

        auto& samples = sequenceIt->second.samples;
        auto sampleIt = std::find_if(samples.begin(), samples.end(), [sequenceImageId](const ImageSequence::Sample& i) {
            return i.sequenceImageId == sequenceImageId;
        });
        if (sampleIt == samples.end())
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addMixedNalGrouping: sequence image id not found");
        }

        sampleIt->mixedNalUnitTypeIds.insert(minpId);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addMixedNalGrouping: exception escaped firewall")
    }

    ErrorCode WriterImpl::addMixedNalPair(const MixedNalUnitTypeId& minpId,
                                          const std::uint16_t mixSubpTrackIdx1,
                                          const std::uint16_t mixSubpTrackIdx2)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addMixedNalPair: called before initialize()");
        }

        // Values shall not be equal.
        if (mixSubpTrackIdx1 == mixSubpTrackIdx2)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addMixedNalPair: invalid argument");
        }

        auto sequenceIt = std::find_if(
            mImageSequences.begin(), mImageSequences.end(),
            [minpId](const pair<SequenceId, ImageSequence>& i) { return i.second.minpGroupings.count(minpId); });
        if (sequenceIt == mImageSequences.end())
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addMixedNalPair: invalid argument");
        }

        sequenceIt->second.minpGroupings[minpId].mixedNalUnitInfos.emplace_back(mixSubpTrackIdx1, mixSubpTrackIdx2);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addMixedNalPair: exception escaped firewall")
    }

    ErrorCode WriterImpl::addRectangularRegionGrouping(const Trif& trif, const SequenceId& sequenceId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addRectangularRegionGrouping: called before initialize()");
        }
        if (!mImageSequences.count(sequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addRectangularRegionGrouping: sequence (track) id not found");
        }

        mImageSequences.at(sequenceId).trifEntries.push_back(trif);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addRectangularRegionGrouping: exception escaped firewall")
    }

    ErrorCode WriterImpl::addVvcSubpictureLayoutMapEntry(const Sulm& sulm, const SequenceId& sequenceId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addVvcSubpictureLayoutMapEntry: called before initialize()");
        }
        if (!mImageSequences.count(sequenceId))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addVvcSubpictureLayoutMapEntry: sequence (track) id not found");
        }

        mImageSequences.at(sequenceId).sulmEntries.push_back(sulm);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addVvcSubpictureLayoutMapEntry: exception escaped firewall")
    }

    ErrorCode WriterImpl::addTrackReference(const SequenceId& from, const TrackGroupId& to, const FourCC& refType)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addTrackReference: called before initialize()");
        }
        if (!mImageSequences.count(from))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addTrackReference: sequence (track) id not found");
        }
        if (!mTrackGroups.count(to))
        {
            return HEIF_FAIL(ErrorCode::INVALID_GROUP_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addTrackReference: group id not found");
        }

        mImageSequences.at(from).trackReferences[refType.value].emplace_back(to.get());

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addTrackReference: exception escaped firewall")
    }

    ErrorCode WriterImpl::addTrackReference(const SequenceId& from, const SequenceId& to, const FourCC& refType)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addTrackReference: called before initialize()");
        }
        if (!mImageSequences.count(from))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addTrackReference: sequence (track) id not found");
        }
        if (!mImageSequences.count(to))
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addTrackReference: sequence (track) id not found");
        }

        mImageSequences.at(from).trackReferences[refType.value].push_back(mImageSequences.at(to).trackId);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addTrackReference: exception escaped firewall")
    }

    void WriterImpl::resetSubpictureIds()
    {
        mNextSubpicId = 0;
    }

}  // namespace HEIF
