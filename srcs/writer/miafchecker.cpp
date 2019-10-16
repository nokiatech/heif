/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2019 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "miafchecker.hpp"
#include "avcconfigurationbox.hpp"
#include "cleanaperturebox.hpp"
#include "decodepts.hpp"
#include "fourccint.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagegrid.hpp"
#include "imagespatialextentsproperty.hpp"
#include "writerconstants.hpp"
#include "writerimpl.hpp"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdint>

using namespace HEIF;

namespace MIAF
{
    MiafChecker::MiafChecker(WriterImpl* writer)
        : mWriter(writer)
    {
    }

    ErrorCode MiafChecker::runChecks()
    {
        detectMiafBrands();

        if (checkMiafBrand() == ErrorCode::MIAF_MIAF_BRAND_MISSING)
        {
            // This is not declared to be a MIAF file. Skip further checks.
            return ErrorCode::OK;
        }

        typedef ErrorCode (MiafChecker::*MiafCheck)() const;
        Vector<MiafCheck> checkers = {
            &MiafChecker::checkMif1Brand,
            &MiafChecker::checkCodecProfile,
            &MiafChecker::checkMsf1BrandForImageSequence,
            &MiafChecker::checkSingleTrack,
            &MiafChecker::checkPrimaryItemIsMiafMasterImage,
            &MiafChecker::checkIdentityDerivedImage,
            &MiafChecker::checkImageDerivationChains,
            &MiafChecker::checkClapIrotImirPropertyOrder,
            &MiafChecker::checkGridDecoderConfigurations,
            &MiafChecker::checkPixiProperty,
            &MiafChecker::checkDerivedItemColourInformation,
            &MiafChecker::checkThumbnailDimensions,
            &MiafChecker::checkGridTileSize,
            &MiafChecker::checkTrackMatrix,
            &MiafChecker::checkGridChroma,
            &MiafChecker::checkCleanApertureChroma,
            &MiafChecker::checkCodingFormat,
            &MiafChecker::checkProgressiveApplicationBrand,
            &MiafChecker::checkBurstCaptureApplicationBrand,
            &MiafChecker::checkAnimationApplicationBrand,
            &MiafChecker::checkMatchedDuration,
            &MiafChecker::checkAlphaTrackDimensions,
        };

        for (const auto& checker : checkers)
        {
            const auto error = ((this)->*(checker))();
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }

        return ErrorCode::OK;
    }

    HEIF::ErrorCode MiafChecker::checkAlphaTrackDimensions() const
    {
        for (const auto& it : mWriter->mImageSequences)
        {
            const auto& sequence = it.second;

            // Check alpha auxiliary tracks
            if (((sequence.auxiliaryType == "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha") ||
                 (sequence.auxiliaryType == "urn:mpeg:hevc:2015:auxid:1")) &&
                (sequence.trackReferences.count("auxl") > 0))
            {
                // Check that dimensions of all auxl-referenced sequences match.
                const auto& masterTrackIds = sequence.trackReferences.at("auxl");
                for (const auto& masterTrackId : masterTrackIds)
                {
                    const auto& masterSequence =
                        std::find_if(mWriter->mImageSequences.cbegin(), mWriter->mImageSequences.cend(),
                                     [&masterTrackId](std::pair<SequenceId, ImageSequence> const& mapItem) {
                                         return masterTrackId == mapItem.second.trackId;
                                     });
                    if ((masterSequence->second.maxDimensions.width != sequence.maxDimensions.width) ||
                        (masterSequence->second.maxDimensions.height != sequence.maxDimensions.height))
                    {
                        return ErrorCode::MIAF_ALPHA_TRACK_DIMENSIONS;
                    }
                }
            }
        }

        return ErrorCode::OK;
    }

    HEIF::ErrorCode MiafChecker::checkMatchedDuration() const
    {
        bool durationFound           = false;
        double firstSequenceDuration = 0;

        for (const auto& it : mWriter->mImageSequences)
        {
            DecodePts::PMap presentationMap;
            double duration;

            if (!getTrackTimeStamps(it.second, duration, presentationMap))
            {
                return ErrorCode::MIAF_TRACK_DURATION;
            }

            if (durationFound == false)
            {
                firstSequenceDuration = duration;
                durationFound         = true;
            }
            else
            {
                // Because of roundings, allow max 1 millisecond difference in duration.
                if (std::fabs(duration - firstSequenceDuration) > 0.001)
                {
                    return ErrorCode::MIAF_TRACK_DURATION;
                }
            }
        }

        return ErrorCode::OK;
    }

    HEIF::ErrorCode MiafChecker::checkAnimationApplicationBrand() const
    {
        if (!mAnimationApplicationBrand)
        {
            return ErrorCode::OK;
        }

        // Check constraints:
        // -exactly one non-auxiliary video track
        // -at most one auxiliary (alpha plane) video track
        // -at most one audio track
        // -no other media tracks
        // -luma sample rate of each video track shall be 62914560 or less
        bool masterTrackFound = false;
        bool auxTrackFound    = false;
        bool audioTrackFound  = false;

        for (const auto& it : mWriter->mImageSequences)
        {
            const ImageSequence& track = it.second;
            const auto handler         = track.handlerType;

            if (handler == PICT_HANDLER)
            {
                if (masterTrackFound || !checkLumaSampleRate(track))
                {
                    return ErrorCode::MIAF_ANIMATION_APPLICATION;
                }
                masterTrackFound = true;
            }
            else if (handler == AUXV_HANDLER)
            {
                if (auxTrackFound || !checkLumaSampleRate(track))
                {
                    return ErrorCode::MIAF_ANIMATION_APPLICATION;
                }
                auxTrackFound = true;
            }
            else if (handler == SOUN_HANDLER)
            {
                if (audioTrackFound)
                {
                    return ErrorCode::MIAF_ANIMATION_APPLICATION;
                }
                audioTrackFound = true;
            }
            else
            {
                return ErrorCode::MIAF_ANIMATION_APPLICATION;
            }
        }

        if (!masterTrackFound)
        {
            return ErrorCode::MIAF_ANIMATION_APPLICATION;
        }

        // also edit list constraints apply
        ErrorCode error = checkEditLists();

        return error;
    }

    bool MiafChecker::getTrackTimeStamps(const ImageSequence& sequence,
                                         double& duration,
                                         DecodePts::PMap& presentationMap) const
    {
        const uint32_t trackId = sequence.trackId.get();

        for (const auto& trackBox : mWriter->mMovieBox.getTrackBoxes())
        {
            if (trackId != trackBox->getTrackHeaderBox().getTrackID())
            {
                continue;
            }

            const SampleTableBox& stblBox        = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
            const TrackHeaderBox& trackHeaderBox = trackBox->getTrackHeaderBox();
            const TimeToSampleBox& timeToSampleBox                           = stblBox.getTimeToSampleBox();
            std::shared_ptr<const CompositionOffsetBox> compositionOffsetBox = stblBox.getCompositionOffsetBox();

            const uint32_t mediaTimeScale = trackBox->getMediaBox().getMediaHeaderBox().getTimeScale();
            const uint32_t movieTimeScale =
                mWriter->mMovieBox.getMovieHeaderBox().getTimeScale();   // Number of time units that pass in a second
            const uint64_t tkhdDuration = trackHeaderBox.getDuration();  // Duration is in timescale units

            std::shared_ptr<const EditBox> editBox = trackBox->getEditBox();
            DecodePts decodePts;
            decodePts.loadBox(&timeToSampleBox);
            decodePts.loadBox(compositionOffsetBox.get());
            if (editBox != nullptr)
            {
                const EditListBox* editListBox = editBox->getEditListBox();
                decodePts.loadBox(editListBox, movieTimeScale, mediaTimeScale);
            }
            if (!decodePts.unravel())
            {
                return false;
            }

            presentationMap = decodePts.getTime(mediaTimeScale);

            static const uint32_t DURATION_FROM_EDIT_LIST = 0xffffffff;
            if (tkhdDuration == DURATION_FROM_EDIT_LIST)
            {
                duration = static_cast<double>(decodePts.getSpan()) / mediaTimeScale;
            }
            else
            {
                duration = tkhdDuration / static_cast<double>(movieTimeScale);
            }

            if (editBox != nullptr)
            {
                // HEIF defines that when (flags & 1) is equal to 1, the entire edit list is repeated a sufficient
                // number of times to equal the track duration.
                if ((editBox->getEditListBox()->getFlags() & 1) == 1)
                {
                    DecodePts::PMap repeatingPMap;
                    const int64_t trackDuration    = static_cast<int64_t>(duration * 1000u);
                    const int64_t editListDuration = static_cast<int64_t>(decodePts.getSpan() * 1000u / mediaTimeScale);
                    auto iter                      = presentationMap.cbegin();
                    int64_t nextSampleTimestamp    = iter->first;
                    int64_t offset                 = 0;

                    while (nextSampleTimestamp < trackDuration)
                    {
                        repeatingPMap.insert(nextSampleTimestamp, iter->second);
                        ++iter;

                        // Increase timestamp offset and skip to begin if the end was reached.
                        if (iter == presentationMap.cend())
                        {
                            iter = presentationMap.cbegin();
                            offset += editListDuration;
                        }
                        nextSampleTimestamp = iter->first + offset;
                    }

                    presentationMap = repeatingPMap;
                }
            }

            return true;
        }

        return false;
    }

    bool MiafChecker::checkLumaSampleRate(const ImageSequence& sequence) const
    {
        const unsigned int pixels = sequence.maxDimensions.height * sequence.maxDimensions.width;
        double duration           = 0;
        DecodePts::PMap pMap;

        if (!getTrackTimeStamps(sequence, duration, pMap))
        {
            return false;  // error during unravel
        }

        const double fps = pMap.size() / duration;
        if (pixels * fps > 62914560)
        {
            return false;
        }
        return true;
    }

    ErrorCode MiafChecker::checkEditLists() const
    {
        bool editListFound = false;
        bool isLooping     = false;

        for (const auto& it : mWriter->mImageSequences)
        {
            const EditList& editlist = it.second.editList;
            if (editlist.editUnits.size > 0)
            {
                if (editListFound)
                {
                    // Either all edit lists shall indicate looping, or none.
                    if (editlist.looping != isLooping)
                    {
                        return ErrorCode::MIAF_EDIT_LIST;
                    }
                }
                else
                {
                    editListFound = true;
                    isLooping     = editlist.looping;
                }

                // Allowed edit list types:
                // 1. media edit
                // 2. empty edit, media edit
                // 3. media edit, media edit
                // 4. empty edit, media edit (forward), media edit (reverse)
                if (editlist.editUnits.size == 1)
                {
                    if (isValidMediaEdit(editlist.editUnits[0], false, false) == false)
                    {
                        return ErrorCode::MIAF_EDIT_LIST;
                    }
                }
                else if (editlist.editUnits.size == 2)
                {
                    if (((isValidMediaEdit(editlist.editUnits[0], true, false) &&
                          isValidMediaEdit(editlist.editUnits[1], false, true)) ||
                         (isValidEmptyEdit(editlist.editUnits[0]) &&
                          isValidMediaEdit(editlist.editUnits[1], false, false))) == false)
                    {
                        return ErrorCode::MIAF_EDIT_LIST;
                    }

                    // When there is two media edits, played media time range must be identical in both edit units.
                    if (isValidMediaEdit(editlist.editUnits[0], true, false))
                    {
                        if ((editlist.editUnits[0].mediaTimeInTrackTS != editlist.editUnits[1].mediaTimeInTrackTS) ||
                            (editlist.editUnits[0].durationInMovieTS != editlist.editUnits[1].durationInMovieTS))
                        {
                            return ErrorCode::MIAF_EDIT_LIST;
                        }
                    }
                }
                else if (editlist.editUnits.size == 3)
                {
                    if ((isValidEmptyEdit(editlist.editUnits[0]) &&
                         isValidMediaEdit(editlist.editUnits[1], true, false) &&
                         isValidMediaEdit(editlist.editUnits[2], false, true)) == false)
                    {
                        return ErrorCode::MIAF_EDIT_LIST;
                    }

                    // When there is two media edits, played media time range must be identical in both edit units.
                    if ((editlist.editUnits[1].mediaTimeInTrackTS != editlist.editUnits[2].mediaTimeInTrackTS) ||
                        (editlist.editUnits[1].durationInMovieTS != editlist.editUnits[2].durationInMovieTS))
                    {
                        return ErrorCode::MIAF_EDIT_LIST;
                    }
                }
                else
                {
                    return ErrorCode::MIAF_EDIT_LIST;
                }
            }
        }

        return ErrorCode::OK;
    }

    bool MiafChecker::isValidEmptyEdit(const HEIF::EditUnit& editunit) const
    {
        if (editunit.editType == EditType::EMPTY)
        {
            return true;
        }

        if ((editunit.editType == EditType::RAW) && (editunit.mediaTimeInTrackTS == -1))
        {
            return true;
        }

        return false;
    }

    bool MiafChecker::isValidMediaEdit(const HEIF::EditUnit& editUnit,
                                       const bool onlyForward,
                                       const bool onlyReverse) const
    {
        // require 0 < media_rate <= 1(forward play), or minus 1 (reverse play)
        if ((editUnit.editType == EditType::SHIFT) && !onlyReverse)
        {
            // Shift edit implies media_rate 1.
            return true;
        }
        else if (editUnit.editType == EditType::RAW)
        {
            // Check if it is a reverse edit.
            if ((editUnit.mediaRateInteger == -1) && (editUnit.mediaRateFraction == 0))
            {
                if (onlyForward)
                {
                    return false;
                }
                return true;
            }

            if (onlyReverse)
            {
                return false;
            }

            // Forward play, normal speed.
            if ((editUnit.mediaRateInteger == 1) && (editUnit.mediaRateFraction == 0))
            {
                return true;
            }

            // Forward play slow motion.
            if ((editUnit.mediaRateInteger == 0) && (editUnit.mediaRateFraction > 0))
            {
                return true;
            }
        }

        return false;
    }

    HEIF::ErrorCode MiafChecker::checkBurstCaptureApplicationBrand() const
    {
        Set<uint32_t> burstSequenceIds;

        const auto& trackBoxes = mWriter->mMovieBox.getTrackBoxes();
        for (const auto& track : trackBoxes)
        {
            const auto& brands = track->getTrackTypeBox().getCompatibleBrands();
            const auto isFound = std::find(brands.cbegin(), brands.cend(), "MiBu") != brands.cend();
            if (isFound)
            {
                burstSequenceIds.insert(track->getTrackHeaderBox().getTrackID());
            }
        }

        // Brand in FileTypeBox indicates presence of a 'pict' track conforming to
        // the this brand.
        Set<uint32_t> burstSequenceCandidateIds;
        if (mBurstCaptureApplicationBrand)
        {
            for (const auto& track : trackBoxes)
            {
                burstSequenceCandidateIds.insert(track->getTrackHeaderBox().getTrackID());
            }
        }

        bool burstSequenceFound = false;
        // First check all tracks marked as 'MiBu'.
        for (const auto sequenceId : burstSequenceIds)
        {
            if (!isBurstSequence(sequenceId))
            {
                return ErrorCode::MIAF_BURST_CAPTURE_APPLICATION;
            }
            burstSequenceFound = true;
        }

        // If FileTypeBox was indicating 'MiBu', at least one 'MiBu' compatible track should be
        // present, possibly without similar TrackTypeBox brand present.
        if (mBurstCaptureApplicationBrand && !burstSequenceFound)
        {
            for (const auto sequenceId : burstSequenceCandidateIds)
            {
                if (isBurstSequence(sequenceId))
                {
                    return ErrorCode::OK;
                }
            }

            // Burst application brand was set in FileTypeBox, but there was no conforming
            // image sequence present.
            return ErrorCode::MIAF_BURST_CAPTURE_APPLICATION;
        }

        return ErrorCode::OK;
    }

    bool MiafChecker::isBurstSequence(const uint32_t trackId) const
    {
        for (const auto& it : mWriter->mImageSequences)
        {
            const ImageSequence& sequence = it.second;
            if ((sequence.trackId == trackId) && (sequence.handlerType == "pict"))
            {
                for (const ImageSequence::Sample& sample : sequence.samples)
                {
                    if (getDependencyCount(sequence.samples, sample.sampleIndex) > 2)
                    {
                        return false;
                    }
                }
                return true;
            }
        }

        return false;
    }

    unsigned int MiafChecker::getDependencyCount(const Vector<ImageSequence::Sample>& samples,
                                                 const uint32_t sampleIndex) const
    {
        unsigned int referenceCount         = 1;
        const ImageSequence::Sample& sample = samples.at(sampleIndex);
        for (const std::uint32_t referenceSample : sample.referenceSamples)
        {
            referenceCount += getDependencyCount(samples, referenceSample);
        }
        return referenceCount;
    }

    HEIF::ErrorCode MiafChecker::checkProgressiveApplicationBrand() const
    {
        if (!mProgressiveApplicationBrand)
        {
            return ErrorCode::OK;
        }

        // Using initial 'mdat' mode is not compatible with the progressive application brand.
        if (mWriter->mInitialMdat == true)
        {
            return ErrorCode::MIAF_PROGRESSIVE_APPLICATION;
        }

        const auto primaryItemId         = mWriter->mMetaBox.getPrimaryItemBox().getItemId();
        const auto primaryItemDataOffset = getItemDataOffset(primaryItemId);
        const auto thumbnails            = getThumbnails(primaryItemId);
        bool thumbInBeginningFound       = false;
        for (const auto thumbId : thumbnails)
        {
            if (getItemDataOffset(thumbId) > primaryItemDataOffset)
            {
                return ErrorCode::MIAF_PROGRESSIVE_APPLICATION;
            }

            const std::uint64_t THUMB_END_MAX_OFFSET = 128000;
            if (getItemDataLastByteOffset(thumbId) <= THUMB_END_MAX_OFFSET)
            {
                thumbInBeginningFound = true;
            }
        }

        if (!thumbInBeginningFound)
        {
            return ErrorCode::MIAF_PROGRESSIVE_APPLICATION;
        }

        // Perceived progressive refinement of primary item alternative group members is left to
        // user's responsibility.

        return ErrorCode::OK;
    }

    std::uint64_t MiafChecker::getItemDataOffset(const std::uint32_t itemId) const
    {
        const auto& iloc               = mWriter->mMetaBox.getItemLocationBox().getItemLocationForID(itemId);
        const ExtentList& extentList   = iloc.getExtentList();
        const std::uint64_t baseOffset = iloc.getBaseOffset();
        return baseOffset + extentList.at(0).mExtentOffset;
    }

    std::uint64_t MiafChecker::getItemDataLastByteOffset(const std::uint32_t itemId) const
    {
        const auto& iloc               = mWriter->mMetaBox.getItemLocationBox().getItemLocationForID(itemId);
        const ExtentList& extentList   = iloc.getExtentList();
        const std::uint64_t baseOffset = iloc.getBaseOffset();
        /// @todo Add support for several extents.
        return baseOffset + extentList.at(0).mExtentOffset + extentList.at(0).mExtentLength;
    }

    bool MiafChecker::isHevcConfigurationBoxConforming(const HevcConfigurationBox* config,
                                                       const bool isVideoTrack) const
    {
        const Vector<std::uint8_t> constraintFlags = config->getHevcConfiguration().getGeneralConstraintFlags();
        const auto generalProfileIdc               = config->getHevcConfiguration().getGeneralProfileIdc();
        const auto generalLevelIdc                 = config->getHevcConfiguration().getGeneralLevelIdc();

        enum class HevcProfile
        {
            // HEVC Basic profile
            HEVC_MAIN_STILL_PICTURE,
            HEVC_MAIN,
            // HEVC Advanced profile
            MAIN_10,
            MAIN_10_INTRA,
            MAIN_10_STILL_PICTURE,
            MAIN_422_10,
            MAIN_422_10_INTRA,
            // HEVC Extended profile
            MAIN_444_10,
            MAIN_444_STILL_PICTURE,
            MAIN_444_10_INTRA,
            MONOCHROME_10
        };

        HevcProfile profile;

        const std::bitset<32> generalProfileCompatibilityFlag(
            config->getHevcConfiguration().getGeneralProfileCompatibilityFlags());
        if ((generalProfileIdc == 1) || (generalProfileCompatibilityFlag.test(1)))
        {
            // Main profile
            profile = HevcProfile::HEVC_MAIN;
        }
        else if ((generalProfileIdc == 2) || (generalProfileCompatibilityFlag.test(2)))
        {
            // Main 10 profile
            profile = HevcProfile::MAIN_10;
        }
        else if ((generalProfileIdc == 3) || (generalProfileCompatibilityFlag.test(3)))
        {
            // Main Still Picture profile
            profile = HevcProfile::HEVC_MAIN_STILL_PICTURE;
        }
        else if ((generalProfileIdc == 4) || (generalProfileCompatibilityFlag.test(4)))
        {
            // Format range extension profile
            struct ExtensionProfile
            {
                HevcProfile profile;
                bool generalMax12bitConstraint;
                bool generalMax10bitConstraint;
                bool generalMax8bitConstraint;
                bool generalMax422chromaConstraint;
                bool generalMax420chromaConstraint;
                bool generalMaxMonochromeConstraint;
                bool generalIntraConstraint;
                bool generalOnePictureOnlyConstraint;
                bool generalLowerBitRateConstraint;

                bool operator==(const ExtensionProfile& other) const
                {
                    return (generalMax12bitConstraint == other.generalMax12bitConstraint) &&
                           (generalMax10bitConstraint == other.generalMax10bitConstraint) &&
                           (generalMax8bitConstraint == other.generalMax8bitConstraint) &&
                           (generalMax422chromaConstraint == other.generalMax422chromaConstraint) &&
                           (generalMax420chromaConstraint == other.generalMax420chromaConstraint) &&
                           (generalMaxMonochromeConstraint == other.generalMaxMonochromeConstraint) &&
                           (generalIntraConstraint == other.generalIntraConstraint) &&
                           (generalOnePictureOnlyConstraint == other.generalOnePictureOnlyConstraint) &&
                           (generalLowerBitRateConstraint == other.generalLowerBitRateConstraint);
                }
            };

            ExtensionProfile configFlags;
            const std::bitset<8> constraintFlagBits1(constraintFlags.at(0));
            const std::bitset<8> constraintFlagBits2(constraintFlags.at(1));
            configFlags.generalMax12bitConstraint       = constraintFlagBits1.test(3);
            configFlags.generalMax10bitConstraint       = constraintFlagBits1.test(2);
            configFlags.generalMax8bitConstraint        = constraintFlagBits1.test(1);
            configFlags.generalMax422chromaConstraint   = constraintFlagBits1.test(0);
            configFlags.generalMax420chromaConstraint   = constraintFlagBits2.test(7);
            configFlags.generalMaxMonochromeConstraint  = constraintFlagBits2.test(6);
            configFlags.generalIntraConstraint          = constraintFlagBits2.test(5);
            configFlags.generalOnePictureOnlyConstraint = constraintFlagBits2.test(4);
            configFlags.generalLowerBitRateConstraint   = constraintFlagBits2.test(3);

            const std::vector<ExtensionProfile> rangeExtensionProfiles = {
                {HevcProfile::MAIN_10_INTRA, true, true, false, true, true, false, true, false, true},
                {HevcProfile::MAIN_10_INTRA, true, true, false, true, true, false, true, false, false},
                {HevcProfile::MAIN_10_STILL_PICTURE, true, true, false, true, true, false, true, true, false},
                {HevcProfile::MAIN_10_STILL_PICTURE, true, true, false, true, true, false, true, true, true},
                {HevcProfile::MAIN_422_10, true, true, false, true, false, false, false, false, true},
                {HevcProfile::MAIN_422_10_INTRA, true, true, false, true, false, false, true, false, false},
                {HevcProfile::MAIN_422_10_INTRA, true, true, false, true, false, false, true, false, true},
                {HevcProfile::MAIN_444_STILL_PICTURE, true, true, true, false, false, false, true, true, false},
                {HevcProfile::MAIN_444_STILL_PICTURE, true, true, true, false, false, false, true, true, true},
                {HevcProfile::MAIN_444_10, true, true, false, false, false, false, false, false, true},
                {HevcProfile::MAIN_444_10_INTRA, true, true, false, false, false, false, true, false, false},
                {HevcProfile::MAIN_444_10_INTRA, true, true, false, false, false, false, true, false, true},
                {HevcProfile::MONOCHROME_10, true, true, false, true, true, true, false, false, true},
            };

            const auto iter = std::find(rangeExtensionProfiles.cbegin(), rangeExtensionProfiles.cend(), configFlags);
            if (iter == rangeExtensionProfiles.cend())
            {
                return false;
            }
            profile = iter->profile;
        }
        else
        {
            return false;
        }

        if (isVideoTrack)
        {
            // Maximum allowed level 5.1 equals to general_level_idc value 153.
            if (generalLevelIdc > 153)
            {
                return false;
            }

            if (profile == HevcProfile::MAIN_444_10)
            {
                if (mMiafHevcExtendedProfile)
                {
                    return true;
                }
            }

            if ((profile == HevcProfile::MAIN_422_10) || (profile == HevcProfile::MAIN_10))
            {
                if (mMiafHevcExtendedProfile || mMiafHevcAdvancedProfile)
                {
                    return true;
                }
            }

            if (profile == HevcProfile::HEVC_MAIN)
            {
                if (mMiafHevcExtendedProfile || mMiafHevcAdvancedProfile || mMiafHevcBasicProfile)
                {
                    return true;
                }
            }
        }
        else
        {
            // Maximum allowed level 6 equals to general_level_idc value 160.
            if (generalLevelIdc > 160)
            {
                return false;
            }

            if ((profile == HevcProfile::MAIN_444_10) || (profile == HevcProfile::MAIN_444_STILL_PICTURE) ||
                (profile == HevcProfile::MAIN_444_10_INTRA) || (profile == HevcProfile::MONOCHROME_10))
            {
                if (mMiafHevcExtendedProfile)
                {
                    return true;
                }
            }

            if ((profile == HevcProfile::MAIN_10) || (profile == HevcProfile::MAIN_10_INTRA) ||
                (profile == HevcProfile::MAIN_10_STILL_PICTURE) || (profile == HevcProfile::MAIN_422_10_INTRA))
            {
                if (mMiafHevcAdvancedProfile || mMiafHevcExtendedProfile)
                {
                    return true;
                }
            }

            if ((profile == HevcProfile::HEVC_MAIN_STILL_PICTURE) || (profile == HevcProfile::HEVC_MAIN))
            {
                if (mMiafHevcAdvancedProfile || mMiafHevcExtendedProfile || mMiafHevcBasicProfile)
                {
                    return true;
                }
            }
        }

        return false;
    }

    bool MiafChecker::isAvcConfigurationBoxConforming(const AvcConfigurationBox* config, const bool isVideoTrack) const
    {
        const AvcDecoderConfigurationRecord& record = config->getAvcConfiguration();
        const auto constraintFlags                  = std::bitset<6>(record.getProfileCompatibility());
        const auto profileIdc                       = record.getAvcProfileIndication();
        const auto levelIdc                         = record.getAvcLevelIndication();

        enum class AvcProfile
        {
            // AVC Basic profile
            HIGH_PROFILE,
            PROGRESSIVE_HIGH_PROFILE,
            CONSTRAINED_HIGH_PROFILE
        };

        AvcProfile profile;

        // All High, Progressive High profile and Constrained High profile are indicated by profile_idc value 100.
        if (profileIdc != 100)
        {
            return false;
        }

        if (constraintFlags.test(4) && constraintFlags.test(5))
        {
            profile = AvcProfile::CONSTRAINED_HIGH_PROFILE;
        }
        else if (constraintFlags.test(4))
        {
            profile = AvcProfile::PROGRESSIVE_HIGH_PROFILE;
        }
        else
        {
            profile = AvcProfile::HIGH_PROFILE;
        }

        if (isVideoTrack)
        {
            // Maximum allowed level 5.1 equals to level_idc value 51.
            if (levelIdc > 51)
            {
                return false;
            }

            if ((profile == AvcProfile::HIGH_PROFILE) && mMiafAvcBasicProfile)
            {
                return true;
            }
        }
        else
        {
            // Maximum allowed level 5.2 equals to level_idc value 52.
            if (levelIdc > 52)
            {
                return false;
            }

            if ((profile == AvcProfile::PROGRESSIVE_HIGH_PROFILE) || (profile == AvcProfile::CONSTRAINED_HIGH_PROFILE))
            {
                if (mMiafAvcBasicProfile)
                {
                    return true;
                }
            }
        }

        return false;
    }

    HEIF::ErrorCode MiafChecker::checkCodingFormat() const
    {
        for (const auto& image : mWriter->mImageCollection.images)
        {
            const auto imageId  = image.first.get();
            const auto itemType = getItemType(imageId);
            if (((itemType == "hvc1") || (itemType == "avc1")) == false)
            {
                continue;
            }

            const DecoderConfigId itemConfigId      = mWriter->mImageCollection.images.at(imageId).decoderConfigId;
            const PropertyIndex configPropertyIndex = mWriter->mDecoderConfigs.at(itemConfigId);
            const auto& iprp                        = mWriter->mMetaBox.getItemPropertiesBox();
            const Box* config = iprp.getPropertyByIndex(static_cast<uint32_t>(configPropertyIndex - 1));

            if (itemType == "hvc1")
            {
                const HevcConfigurationBox* configBox = dynamic_cast<const HevcConfigurationBox*>(config);
                if (isHevcConfigurationBoxConforming(configBox, false) == false)
                {
                    return ErrorCode::MIAF_ENCODING;
                }
            }

            if (itemType == "avc1")
            {
                const AvcConfigurationBox* configBox = dynamic_cast<const AvcConfigurationBox*>(config);
                if (isAvcConfigurationBoxConforming(configBox, false) == false)
                {
                    return ErrorCode::MIAF_ENCODING;
                }
            }
        }

        for (const auto& track : mWriter->mMovieBox.getTrackBoxes())
        {
            const auto& stbl    = track->getMediaBox().getMediaInformationBox().getSampleTableBox();
            const auto& handler = track->getMediaBox().getHandlerBox().getHandlerType();
            const Vector<UniquePtr<SampleEntryBox>>& sampleEntries = stbl.getSampleDescriptionBox().getSampleEntries();
            for (const auto& sampleEntry : sampleEntries)
            {
                const Box* configBox                      = sampleEntry->getConfigurationBox();
                const HevcConfigurationBox* hevcConfigBox = dynamic_cast<const HevcConfigurationBox*>(configBox);
                if (hevcConfigBox && isHevcConfigurationBoxConforming(hevcConfigBox, handler == "vide") == false)
                {
                    return ErrorCode::MIAF_ENCODING;
                }

                const AvcConfigurationBox* avcConfigBox = dynamic_cast<const AvcConfigurationBox*>(configBox);
                if (avcConfigBox && isAvcConfigurationBoxConforming(avcConfigBox, handler == "vide") == false)
                {
                    return ErrorCode::MIAF_ENCODING;
                }
            }
        }

        return ErrorCode::OK;
    }

    HEIF::ErrorCode MiafChecker::checkCleanApertureChroma() const
    {
        const auto& iprp = mWriter->mMetaBox.getItemPropertiesBox();

        for (const auto& image : mWriter->mImageCollection.images)
        {
            const auto itemId = image.first.get();
            const std::uint32_t index =
                iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::CLAP, image.first.get());
            if (index > 0)
            {
                const CleanApertureBox* clap =
                    dynamic_cast<const CleanApertureBox*>(iprp.getPropertyByIndex(index - 1));
                const CleanApertureBox::Fraction heightFraction           = clap->getHeight();
                const CleanApertureBox::Fraction widthFraction            = clap->getWidth();
                const CleanApertureBox::Fraction horizontalOffsetFraction = clap->getHorizOffset();
                const CleanApertureBox::Fraction verticalOffsetFraction   = clap->getVertOffset();

                const unsigned int height    = heightFraction.numerator / heightFraction.denominator;
                const unsigned int heigthMod = heightFraction.numerator % heightFraction.denominator;
                const unsigned int width     = widthFraction.numerator / widthFraction.denominator;
                const unsigned int widthMod  = widthFraction.numerator % widthFraction.denominator;
                const unsigned int horizontalOffset =
                    horizontalOffsetFraction.numerator / horizontalOffsetFraction.denominator;
                const unsigned int horizontalOffsetMod =
                    horizontalOffsetFraction.numerator % horizontalOffsetFraction.denominator;
                const unsigned int verticalOffset =
                    verticalOffsetFraction.numerator / verticalOffsetFraction.denominator;
                const unsigned int verticalOffsetMod =
                    verticalOffsetFraction.numerator % verticalOffsetFraction.denominator;

                const ChromaFormat chromaFormat = getChromaFormat(itemId);
                if ((chromaFormat == CHROMA_422) || (chromaFormat == CHROMA_420))
                {
                    // When the image is 4:2:2 the horizontal cropped offset and width shall be even numbers.
                    if ((width % 2 != 0) || (widthMod != 0) || (horizontalOffset % 2 != 0) ||
                        (horizontalOffsetMod != 0))
                    {
                        return ErrorCode::MIAF_CLAP_DIMENSION;
                    }
                }

                if (chromaFormat == CHROMA_420)
                {
                    // When the image is 4:2:0 both the horizontal and vertical cropped offsets and widths shall be even
                    // numbers.
                    if ((height % 2 != 0) || (heigthMod != 0) || (verticalOffset % 2 != 0) || (verticalOffsetMod != 0))
                    {
                        return ErrorCode::MIAF_CLAP_DIMENSION;
                    }
                }
            }
        }

        return ErrorCode::OK;
    }

    HEIF::ErrorCode MiafChecker::checkGridChroma() const
    {
        for (const auto& image : mWriter->mImageCollection.images)
        {
            const uint32_t itemId = image.first.get();
            if (getItemType(itemId) == "grid")
            {
                const auto& sourceIds = getDerivationSourcesForItem(itemId);
                DecoderConfigId itemConfigId;
                unsigned int tileWidth;
                unsigned int tileHeight;
                ChromaFormat chromaFormat;
                if (getItemType(sourceIds.at(0)) == "iden")
                {
                    const auto& idenSources = getDerivationSourcesForItem(sourceIds.at(0));
                    if (idenSources.size() != 1)
                    {
                        // This shouldn't happen.
                        return ErrorCode::MIAF_MULTIPLE_IDEN_DIMGS;
                    }
                    itemConfigId = mWriter->mImageCollection.images.at(idenSources.at(0)).decoderConfigId;
                    getItemSize(idenSources.at(0), tileWidth, tileHeight);
                    chromaFormat = getChromaFormat(idenSources.at(0));
                }
                else
                {
                    itemConfigId = mWriter->mImageCollection.images.at(sourceIds.at(0)).decoderConfigId;
                    getItemSize(sourceIds.at(0), tileWidth, tileHeight);
                    chromaFormat = getChromaFormat(sourceIds.at(0));
                }

                const auto& itemLocation = mWriter->mMetaBox.getItemLocationBox().getItemLocationForID(itemId);
                const auto& extentList   = itemLocation.getExtentList();
                const auto& length       = extentList.at(0).mExtentLength;
                const auto& offset       = extentList.at(0).mExtentOffset;
                Vector<std::uint8_t> gridData;
                mWriter->mMetaBox.getItemDataBox().read(gridData, offset, length);
                BitStream gridStream(gridData);
                const ImageGrid grid = parseImageGrid(gridStream);

                if ((chromaFormat == CHROMA_422) || (chromaFormat == CHROMA_420))
                {
                    // Horizontal tile offsets and widths, and the output width shall be even numbers.
                    if ((grid.outputWidth % 2 != 0) || (tileWidth % 2 != 0))  // tile width decides offset
                    {
                        return ErrorCode::MIAF_GRID_TILE_ALIGN;
                    }
                }

                if (chromaFormat == CHROMA_420)
                {
                    // Vertical tile offsets and height, and the output height shall be even numbers.
                    if ((grid.outputHeight % 2 != 0) || (tileHeight % 2 != 0))  // tile height decides offset
                    {
                        return ErrorCode::MIAF_GRID_TILE_ALIGN;
                    }
                }
            }
        }
        return ErrorCode::OK;
    }

    /*
     * Constraints:
     * -Either a or c but not both shall be equal to 0x0001000 or 0xFFFF0000, while the
     *  remaining one of a and c shall be equal to 0. (HEIF)
     * -Either b or d but not both shall be equal to 0x0001000 or 0xFFFF0000, while the
     *  remaining one of b and d shall be equal to 0. (HEIF)
     * -No translation (x, y) is allowed. (MIAF)
     */
    HEIF::ErrorCode MiafChecker::checkTrackMatrix() const
    {
        for (const auto& track : mWriter->mMovieBox.getTrackBoxes())
        {
            /* Track header box matrix | a b u |
             *                         | c d v |
             *                         | x y w |
             * values are stored in the order {a, b, u, c, d, v, x, y, w}. */
            const Vector<int32_t>& matrix = track->getTrackHeaderBox().getMatrix();
            const int32_t a               = matrix[0];
            const int32_t b               = matrix[1];
            const int32_t u               = matrix[2];
            const int32_t c               = matrix[3];
            const int32_t d               = matrix[4];
            const int32_t v               = matrix[5];
            const int32_t x               = matrix[6];
            const int32_t y               = matrix[7];
            const int32_t w               = matrix[8];
            const int32_t ALLOWED1        = 0x00010000;
            const int32_t ALLOWED2        = static_cast<int32_t>(0xffff0000);
            if (((a == ALLOWED1 && c == 0) || (a == ALLOWED2 && c == 0) || (a == 0 && c == ALLOWED1) ||
                 (a == 0 && c == ALLOWED2) || (b == ALLOWED1 && d == 0) || (b == ALLOWED2 && d == 0) ||
                 (b == 0 && d == ALLOWED1) || (b == 0 && d == ALLOWED2)) == false)
            {
                // Allow only combinations of horizontal and vertical mirroring
                // and counter-clockwise rotation by 0, 90, 180, and 270 degrees.
                return ErrorCode::MIAF_TRACK_MATRIX;
            }

            if ((x != 0) || (y != 0))
            {
                // Allow no translation
                return ErrorCode::MIAF_TRACK_MATRIX;
            }

            // From HEIF specification
            if ((u != 0) || (v != 0) || (w != 0x40000000))
            {
                return ErrorCode::MIAF_TRACK_MATRIX;
            }
        }

        return ErrorCode::OK;
    }

    ErrorCode MiafChecker::checkGridTileSize() const
    {
        for (const auto& image : mWriter->mImageCollection.images)
        {
            const auto itemId = image.first.get();
            const auto type   = getItemType(itemId);
            if (type == "grid")
            {
                const auto& sources = getDerivationSourcesForItem(itemId);
                for (const auto sourceId : sources)
                {
                    uint32_t width, height;
                    getItemSize(sourceId, width, height);
                    // Tile dimensions should also follow (width % 64 != 0) || (height % 64 != 0), but
                    // check is skipped as it is not mandatory.
                    if ((width < 64) || (height < 64))
                    {
                        return ErrorCode::MIAF_GRID_TILE_SIZE;
                    }
                }
            }
        }

        return ErrorCode::OK;
    }

    /*
     * It is allowed to have max 200x between the total number of pixels in a MIAF thumbnail
     * image item and the next larger MIAF thumbnail image item, or the associated MIAF master
     * image item if there is no larger MIAF thumbnail image item.
     */
    ErrorCode MiafChecker::checkThumbnailDimensions() const
    {
        for (const auto& image : mWriter->mImageCollection.images)
        {
            const auto masterItemId = image.first.get();
            if (isMasterImage(masterItemId))
            {
                Vector<unsigned int> sizes;
                const auto& thumbIds = getThumbnails(masterItemId);
                if (thumbIds.size() > 0)
                {
                    for (const auto thumbId : thumbIds)
                    {
                        sizes.push_back(getItemPixelCount(thumbId));
                    }
                    std::sort(sizes.begin(), sizes.end());
                    const unsigned int MAX_TOTAL_PIXEL_FACTOR = 200;
                    for (unsigned int i = 0; i < sizes.size() - 1; ++i)
                    {
                        if (sizes.at(i) * MAX_TOTAL_PIXEL_FACTOR < sizes.at(i + 1))
                        {
                            return ErrorCode::MIAF_THUMBNAIL_SIZE;
                        }
                    }
                    if (sizes.size())
                    {
                        if (sizes.back() * MAX_TOTAL_PIXEL_FACTOR < getItemPixelCount(masterItemId))
                        {
                            return ErrorCode::MIAF_THUMBNAIL_SIZE;
                        }
                    }
                }
            }
        }

        return ErrorCode::OK;
    }

    unsigned int MiafChecker::getItemPixelCount(const uint32_t itemId) const
    {
        uint32_t width;
        uint32_t height;
        getItemSize(itemId, width, height);
        return width * height;
    }

    /// @todo Probably there should be an option to take account also possible transformative properties of the item.
    void MiafChecker::getItemSize(uint32_t itemId, uint32_t& width, uint32_t& height) const
    {
        const auto& iprp     = mWriter->mMetaBox.getItemPropertiesBox();
        const auto ispeIndex = iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::ISPE, itemId);

        const ImageSpatialExtentsProperty* ispe =
            static_cast<const ImageSpatialExtentsProperty*>(iprp.getPropertyByIndex(ispeIndex - 1));

        height = ispe->getDisplayHeight();
        width  = ispe->getDisplayWidth();
    }

    Vector<std::uint32_t> MiafChecker::getThumbnails(const uint32_t masterItemId) const
    {
        Vector<std::uint32_t> thumbIds;
        const auto& iref           = mWriter->mMetaBox.getItemReferenceBox();
        const auto& thmbReferences = iref.getReferencesOfType("thmb");
        for (const auto& thmbReference : thmbReferences)
        {
            const auto& masterIds = thmbReference.getToItemIds();
            if (std::find(masterIds.cbegin(), masterIds.cend(), masterItemId) != masterIds.cend())
            {
                thumbIds.push_back(thmbReference.getFromItemID());
            }
        }

        return thumbIds;
    }

    /*
     * All grid input images must use the same coding format,
     * chroma sub-sampling, and the same decoder configuration.
     */
    ErrorCode MiafChecker::checkGridDecoderConfigurations() const
    {
        for (const auto& images : mWriter->mImageCollection.images)
        {
            const uint32_t itemId = images.first.get();
            const auto itemType   = getItemType(itemId);
            bool configFound      = false;
            DecoderConfigId gridConfigId;
            if (itemType == "grid")
            {
                const auto& sourceIds = getDerivationSourcesForItem(itemId);
                for (const auto sourceId : sourceIds)
                {
                    DecoderConfigId itemConfigId;
                    if (getItemType(sourceId) == "iden")
                    {
                        const auto& idenSources = getDerivationSourcesForItem(sourceId);
                        if (idenSources.size() != 1)
                        {
                            return ErrorCode::MIAF_MULTIPLE_IDEN_DIMGS;
                        }
                        itemConfigId = mWriter->mImageCollection.images.at(idenSources.at(0)).decoderConfigId;
                    }
                    else
                    {
                        itemConfigId = mWriter->mImageCollection.images.at(sourceId).decoderConfigId;
                    }

                    if (configFound)
                    {
                        if (mWriter->mDecoderConfigs.at(itemConfigId) != mWriter->mDecoderConfigs.at(gridConfigId))
                        {
                            return ErrorCode::MIAF_GRID_DECODER_CONFIG_DIFFER;
                        }
                    }
                    else
                    {
                        configFound  = true;
                        gridConfigId = itemConfigId;
                    }
                }
            }
        }

        return ErrorCode::OK;
    }

    bool MiafChecker::isValidItemId(const uint32_t itemId) const
    {
        for (const auto& image : mWriter->mImageCollection.images)
        {
            if (image.first == itemId)
            {
                return true;
            }
        }

        return false;
    }

    bool MiafChecker::isMasterImage(const uint32_t itemId) const
    {
        if (!isValidItemId(itemId))
        {
            return false;
        }

        return !(hasItemReferencesOfType(itemId, "thmb") || hasItemReferencesOfType(itemId, "auxl"));
    }

    ErrorCode MiafChecker::checkCodecProfile() const
    {
        if (mMiafHevcBasicProfile || mMiafHevcAdvancedProfile || mMiafHevcExtendedProfile || mMiafAvcBasicProfile)
        {
            return ErrorCode::OK;
        }

        return ErrorCode::MIAF_CODEC_PROFILE;
    }

    ErrorCode MiafChecker::checkPrimaryItemIsMiafMasterImage() const
    {
        const auto primaryItemId = mWriter->mMetaBox.getPrimaryItemBox().getItemId();
        if (!isValidItemId(primaryItemId))
        {
            return ErrorCode::MIAF_PRIMARY_ITEM;
        }

        if (!isMiafImageItem(primaryItemId))
        {
            return ErrorCode::MIAF_PRIMARY_NOT_MIAF_ITEM;
        }

        if (!isMasterImage(primaryItemId))
        {
            return ErrorCode::MIAF_PRIMARY_NOT_MASTER;
        }

        return ErrorCode::OK;
    }

    bool MiafChecker::isMiafImageItem(const uint32_t itemId) const
    {
        /* A MIAF image item is an item that conforms to several requirements:
         * a) Box-level requirements - are taken care of by the writer.
         * b) Item level requirements - are checked by other methods.
         * c) Codec profile conformancy - checked by method checkCodingFormat().
         * d) MIAF brand -- checked by method checkMiafBrand().
         * Here it is enough just check if item id is valid (though the writer should
         * handle it already). */

        /// @todo Refactor checks in way that also non-MIAF content in a file would be possible.

        return isValidItemId(itemId);
    }

    // Check mandatory 'mif1' brand presence
    ErrorCode MiafChecker::checkMif1Brand() const
    {
        if (!isBrandPresent("mif1"))
        {
            return ErrorCode::MIAF_MIF1_BRAND_MISSING;
        }
        return ErrorCode::OK;
    }

    // Check mandatory 'msf1' brand presence in cae the file contains an image seqeunce track.
    ErrorCode MiafChecker::checkMsf1BrandForImageSequence() const
    {
        bool isImageSequencePresent = false;
        for (const auto& it : mWriter->mImageSequences)
        {
            const auto& sequence = it.second;
            if (sequence.handlerType == "pict")
            {
                isImageSequencePresent = true;
                break;
            }
        }

        if (isImageSequencePresent && !isBrandPresent("msf1"))
        {
            return ErrorCode::MIAF_MSF1_BRAND_MISSING;
        }
        return ErrorCode::OK;
    }

    /// Check 'miaf' brand presence.
    ErrorCode MiafChecker::checkMiafBrand() const
    {
        if (!isBrandPresent("miaf"))
        {
            return ErrorCode::MIAF_MIAF_BRAND_MISSING;
        }
        return ErrorCode::OK;
    }

    bool MiafChecker::isBrandPresent(FourCCInt brand) const
    {
        const auto brands = mWriter->mFileTypeBox.getCompatibleBrands();
        return std::find(brands.cbegin(), brands.cend(), brand) != brands.cend();
    }

    void MiafChecker::detectMiafBrands()
    {
        const auto compatibleBrands = mWriter->mFileTypeBox.getCompatibleBrands();

        mProgressiveApplicationBrand  = isBrandPresent("MiPr");
        mAnimationApplicationBrand    = isBrandPresent("MiAn");
        mBurstCaptureApplicationBrand = isBrandPresent("MiBu");
        mFragmentedAlphaVideoBrand    = isBrandPresent("MiAC");
        mCommonMediaFragmentedBrand   = isBrandPresent("MiCm");

        mMiafHevcBasicProfile    = isBrandPresent("MiHB");
        mMiafHevcAdvancedProfile = isBrandPresent("MiHA");
        mMiafHevcExtendedProfile = isBrandPresent("MiHE");
        mMiafAvcBasicProfile     = isBrandPresent("MiAB");
    }

    ErrorCode MiafChecker::checkSingleTrack() const
    {
        bool masterFound = false;

        for (const auto& sequence : mWriter->mImageSequences)
        {
            const uint32_t trackId = sequence.second.trackId.get();
            if (isMasterTrack(trackId))
            {
                if (masterFound && !isInMasterAlternateGroup(trackId))
                {
                    return ErrorCode::MIAF_SINGLE_TRACK;
                }
                masterFound = true;
            }
        }

        return ErrorCode::OK;
    }

    bool MiafChecker::hasTrackReferencesOfType(uint32_t trackId, FourCCInt referenceType) const
    {
        const Vector<UniquePtr<TrackBox>>& trackBoxes = mWriter->mMovieBox.getTrackBoxes();
        for (const auto& track : trackBoxes)
        {
            if (track->getTrackHeaderBox().getTrackID() == trackId &&
                track->getTrackReferenceBox().isReferenceTypePresent(referenceType))
            {
                return true;
            }
        }

        return false;
    }

    bool MiafChecker::hasItemReferencesOfType(uint32_t itemId, FourCCInt referenceType) const
    {
        const ItemReferenceBox& iref = mWriter->mMetaBox.getItemReferenceBox();

        const Vector<SingleItemTypeReferenceBox>& references = iref.getReferencesOfType(referenceType);

        for (const auto& reference : references)
        {
            if (reference.getFromItemID() == itemId)
            {
                return true;
            }
        }

        return false;
    }

    bool MiafChecker::isMasterTrack(uint32_t trackId) const
    {
        if (hasTrackReferencesOfType(trackId, "thmb") || hasTrackReferencesOfType(trackId, "auxl"))
        {
            return false;
        }
        return true;
    }

    bool MiafChecker::isInMasterAlternateGroup(uint32_t trackId) const
    {
        const Vector<EntityToGroupBox>& entityToGroupBoxes =
            mWriter->mMetaBox.getGroupsListBox().getEntityToGroupsBoxes();
        for (const auto& entityToGroup : entityToGroupBoxes)
        {
            if (entityToGroup.getType() == "altr")
            {
                const auto& ids = entityToGroup.getEntityIds();
                if (find(ids.cbegin(), ids.cend(), trackId) == ids.cend())
                {
                    break;
                }

                // Group members should be masters
                for (const auto id : ids)
                {
                    if (!isMasterTrack(id))
                    {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }

    /* Transformative properties, if present, must be associated always in the same order:
     * 1. clean aperture
     * 2. rotation
     * 3. mirror
     */
    ErrorCode MiafChecker::checkClapIrotImirPropertyOrder() const
    {
        /// @todo This checks now all image items, although there could be items which are not MIAF items.
        for (const auto& images : mWriter->mImageCollection.images)
        {
            bool clapFound = false;
            bool irotFound = false;
            bool imirFound = false;

            ItemPropertiesBox::PropertyInfos properties =
                mWriter->mMetaBox.getItemPropertiesBox().getItemProperties(images.first.get());
            for (const auto& property : properties)
            {
                if (property.type == ItemPropertiesBox::PropertyType::CLAP)
                {
                    if (clapFound || irotFound || imirFound)
                    {
                        return ErrorCode::MIAF_TRANSFORMATIVE_PROPERTY_ORDER;
                    }
                    clapFound = true;
                }
                else if (property.type == ItemPropertiesBox::PropertyType::IROT)
                {
                    if (irotFound || imirFound)
                    {
                        return ErrorCode::MIAF_TRANSFORMATIVE_PROPERTY_ORDER;
                    }
                    irotFound = true;
                }
                else if (property.type == ItemPropertiesBox::PropertyType::IMIR)
                {
                    if (imirFound)
                    {
                        return ErrorCode::MIAF_TRANSFORMATIVE_PROPERTY_ORDER;
                    }
                    imirFound = true;
                }
            }
        }

        return ErrorCode::OK;
    }

    /* Possible derivations must be in following order:
       1. Mandatory Coded Image(s)
       2. Optional Identity derivation
       3. Optional Grid
       4. Optional Identity derivation
       5. Optional Overlay
       6. Optional Identity derivation
     */
    ErrorCode MiafChecker::checkImageDerivationChains() const
    {
        for (const auto& images : mWriter->mImageCollection.images)
        {
            const uint32_t itemId = images.first.get();
            const ErrorCode error = checkImageDerivationChains(itemId, false, false, false);
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }

        return ErrorCode::OK;
    }

    ErrorCode MiafChecker::checkImageDerivationChains(const uint32_t itemId,
                                                      bool gridFound,
                                                      bool overlayFound,
                                                      bool isCallerIden) const
    {
        const FourCCInt itemType = getItemType(itemId);
        const auto& sources      = getDerivationSourcesForItem(itemId);

        if (isCodedImage(itemId) && sources.size() == 0)
        {
            return ErrorCode::OK;
        }

        if (itemType == "grid")
        {
            if (gridFound || sources.size() == 0)
            {
                return ErrorCode::MIAF_DERIVATION_CHAIN;
            }
            gridFound = true;
        }

        if (itemType == "iovl")
        {
            if (overlayFound || gridFound || sources.size() == 0)
            {
                return ErrorCode::MIAF_DERIVATION_CHAIN;
            }
            overlayFound = true;
        }

        if (itemType == "iden" && sources.size() != 1)
        {
            return ErrorCode::MIAF_DERIVATION_CHAIN;
        }

        if (itemType == "iden" && isCallerIden)
        {
            return ErrorCode::MIAF_DERIVATION_CHAIN;
        }

        for (const auto sourceId : sources)
        {
            const ErrorCode error = checkImageDerivationChains(sourceId, gridFound, overlayFound, isCallerIden);
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }

        if (!(isCodedImage(itemId) || itemType == "iden" || itemType == "iovl" || itemType == "grid"))
        {
            return ErrorCode::MIAF_DERIVATION_CHAIN;
        }

        return ErrorCode::OK;
    }

    // A derived image item of the item_type value 'iden' shall not be derived from an image item of item_type value
    // 'iden'.
    /// @todo Probably this could be removed as derivation chain constraints should take care of this.
    ErrorCode MiafChecker::checkIdentityDerivedImage() const
    {
        for (const auto& images : mWriter->mImageCollection.images)
        {
            const auto itemId = images.first;
            if (getItemType(itemId.get()) == "iden")
            {
                const auto& sourceIds = getDerivationSourcesForItem(itemId.get());
                for (const auto sourceItemId : sourceIds)
                {
                    if (getItemType(sourceItemId) == "iden")
                    {
                        return ErrorCode::MIAF_DERIVATION_CHAIN;
                    }
                }
            }
        }
        return ErrorCode::OK;
    }

    bool MiafChecker::isCodedImage(const std::uint32_t itemId) const
    {
        const FourCCInt type = getItemType(itemId);
        return (type == "hvc1") || (type == "avc1") || (type == "jpeg");
    }

    FourCCInt MiafChecker::getItemType(const uint32_t itemId) const
    {
        const auto& itemInfo = mWriter->mMetaBox.getItemInfoBox().getItemById(itemId);
        return itemInfo.getItemType();
    }

    std::vector<std::uint32_t> MiafChecker::getDerivationSourcesForItem(const std::uint32_t itemId) const
    {
        std::vector<std::uint32_t> sourceIds;

        const auto& idenReferences = mWriter->mMetaBox.getItemReferenceBox().getReferencesOfType("dimg");
        for (const auto& reference : idenReferences)
        {
            if (reference.getFromItemID() == itemId)
            {
                const Vector<std::uint32_t>& toIds = reference.getToItemIds();
                for (const auto sourceItemId : toIds)
                {
                    sourceIds.push_back(sourceItemId);
                }
            }
        }

        return sourceIds;
    }

    // The pixel information property shall be associated with every displayable image.
    ErrorCode MiafChecker::checkPixiProperty() const
    {
        const auto& iprp = mWriter->mMetaBox.getItemPropertiesBox();
        for (const auto& image : mWriter->mImageCollection.images)
        {
            const auto itemId   = image.first.get();
            const auto isHidden = image.second.isHidden;
            if (!isHidden)
            {
                if (iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::PIXI, itemId) == 0)
                {
                    return ErrorCode::MIAF_PIXI_MISSING;
                }
            }
        }
        return ErrorCode::OK;
    }

    // All input items to an overlay shall have the same explicit or default colour information.
    ErrorCode MiafChecker::checkDerivedItemColourInformation() const
    {
        for (const auto& images : mWriter->mImageCollection.images)
        {
            const uint32_t itemId               = images.first.get();
            const auto itemType                 = getItemType(itemId);
            bool defaultColorInformationFound   = false;  // an input without explicit color information found
            bool explicitColourInformationFound = false;
            Vector<std::uint8_t> colrData;
            if (itemType == "grid" || itemType == "iovl")
            {
                if (!isDerivationSourceColrMatching(itemId, colrData, defaultColorInformationFound,
                                                    explicitColourInformationFound))
                {
                    return ErrorCode::MIAF_COLR_PROPERTY;
                }
            }
        }

        return ErrorCode::OK;
    }

    bool MiafChecker::isDerivationSourceColrMatching(uint32_t imageId,
                                                     Vector<std::uint8_t>& colrData,
                                                     bool defaultColorInformationFound,
                                                     bool explicitColourInformationFound) const
    {
        const ItemPropertiesBox& iprp = mWriter->mMetaBox.getItemPropertiesBox();
        const auto colrIndex          = iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::COLR, imageId);
        if (colrIndex == 0)
        {
            if (explicitColourInformationFound)
            {
                // No 'colr' found, but the deriving image already had it.
                return false;
            }
            defaultColorInformationFound = true;
        }

        if (colrIndex != 0)
        {
            if (defaultColorInformationFound)
            {
                // 'colr' found, but the deriving image did not have it.
                return false;
            }

            Vector<std::uint8_t> itemColr = iprp.getPropertyDataByIndex(colrIndex - 1);
            if (explicitColourInformationFound)
            {
                if (colrData != itemColr)
                {
                    return false;
                }
            }
            else
            {
                colrData                       = itemColr;
                explicitColourInformationFound = true;
            }
        }

        const auto& sourceIds = getDerivationSourcesForItem(imageId);
        for (const auto sourceId : sourceIds)
        {
            if (!isDerivationSourceColrMatching(sourceId, colrData, defaultColorInformationFound,
                                                explicitColourInformationFound))
            {
                return false;
            }
        }

        return true;
    }

    MiafChecker::ChromaFormat MiafChecker::getChromaFormat(const std::uint32_t itemId) const
    {
        DecoderConfigId itemConfigId            = mWriter->mImageCollection.images.at(itemId).decoderConfigId;
        const PropertyIndex configPropertyIndex = mWriter->mDecoderConfigs.at(itemConfigId);
        const auto& iprp                        = mWriter->mMetaBox.getItemPropertiesBox();
        const Box* config            = iprp.getPropertyByIndex(static_cast<uint32_t>(configPropertyIndex - 1));
        const auto decoderConfigType = config->getType();

        ChromaFormat chromaFormat;
        if (decoderConfigType == "hvcC")
        {
            const HevcConfigurationBox* hvc1 = dynamic_cast<const HevcConfigurationBox*>(config);
            chromaFormat = static_cast<ChromaFormat>(hvc1->getHevcConfiguration().getChromaFormat());
        }
        else if (decoderConfigType == "avcC")
        {
            const AvcConfigurationBox* avc1 = dynamic_cast<const AvcConfigurationBox*>(config);
            chromaFormat                    = static_cast<ChromaFormat>(avc1->getAvcConfiguration().getChromaFormat());
        }
        else
        {
            /// @todo jpeg chroma format handling.
            chromaFormat = CHROMA_420;
        }

        return chromaFormat;
    }

}  // namespace MIAF
