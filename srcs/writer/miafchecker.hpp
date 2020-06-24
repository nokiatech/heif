/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2020 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef MIAFCHECKER_HPP
#define MIAFCHECKER_HPP

#include <vector>

#include "decodepts.hpp"
#include "fourccint.hpp"
#include "heifcommondatatypes.h"
#include "writerdatatypesinternal.hpp"

namespace HEIF
{
    class WriterImpl;
}  // namespace HEIF

class AvcConfigurationBox;
class HevcConfigurationBox;

namespace MIAF
{
    class MiafChecker
    {
    public:
        MiafChecker(HEIF::WriterImpl* writer);
        HEIF::ErrorCode runChecks();

    private:
        // MIAF constraint checkers
        HEIF::ErrorCode checkGridDecoderConfigurations() const;
        HEIF::ErrorCode checkCodecProfile() const;
        HEIF::ErrorCode checkMif1Brand() const;
        HEIF::ErrorCode checkMiafBrand() const;
        HEIF::ErrorCode checkMsf1BrandForImageSequence() const;
        HEIF::ErrorCode checkSingleTrack() const;
        HEIF::ErrorCode checkPrimaryItemIsMiafMasterImage() const;
        HEIF::ErrorCode checkClapIrotImirPropertyOrder() const;
        HEIF::ErrorCode checkImageDerivationChains() const;
        HEIF::ErrorCode checkIdentityDerivedImage() const;
        HEIF::ErrorCode checkPixiProperty() const;
        HEIF::ErrorCode checkDerivedItemColourInformation() const;
        HEIF::ErrorCode checkThumbnailDimensions() const;
        HEIF::ErrorCode checkGridTileSize() const;
        HEIF::ErrorCode checkTrackMatrix() const;
        HEIF::ErrorCode checkGridChroma() const;
        HEIF::ErrorCode checkCleanApertureChroma() const;
        HEIF::ErrorCode checkCodingFormat() const;
        HEIF::ErrorCode checkMatchedDuration() const;
        HEIF::ErrorCode checkProgressiveApplicationBrand() const;
        HEIF::ErrorCode checkBurstCaptureApplicationBrand() const;
        HEIF::ErrorCode checkAnimationApplicationBrand() const;
        HEIF::ErrorCode checkAlphaTrackDimensions() const;
        HEIF::ErrorCode checkAlphaSequenceCompositionTimes() const;
        HEIF::ErrorCode checkLargeDerivedImageAlternatives() const;
        HEIF::ErrorCode checkFilenameExtension() const;

        // Shared checkers
        HEIF::ErrorCode checkEditLists() const;


        // Helper methods
        void detectMiafBrands();
        bool checkLumaSampleRate(const HEIF::ImageSequence& sequence) const;
        bool isMiafImageItem(uint32_t itemId) const;
        bool isBrandPresent(FourCCInt brand) const;
        bool hasItemReferencesOfType(uint32_t itemId, FourCCInt referenceType) const;
        bool hasTrackReferencesOfType(uint32_t trackId, FourCCInt reference) const;
        bool isInMasterAlternateGroup(uint32_t trackId) const;

        /**
         * @param itemId Item id to check.
         * @return Return true if the item is the primary item or in same alternative entity group with the primary
         * item.
         */
        bool isPrimaryItemOrAlternative(uint32_t itemId) const;

        /**
         * @param itemId Item id of the image.
         * @return Total sum of input pixels, which considers also input images of derived images.
         */
        unsigned int getItemInputPixelCount(uint32_t itemId) const;

        /**
         * @param itemId Item id to check.
         * @return True if the item is a derived grid item, false otherwise.
         */
        bool isItemGrid(uint32_t itemId) const;

        /**
         * @param itemId Item id to check.
         * @return True if the item is a derived overlay item, false otherwise.
         */
        bool isItemOverlay(uint32_t itemId) const;

        bool isValidItemId(uint32_t itemId) const;
        bool isMasterTrack(uint32_t trackId) const;
        bool isMasterImage(uint32_t itemId) const;
        Vector<std::uint32_t> getThumbnails(uint32_t itemId) const;
        unsigned int getItemPixelCount(uint32_t itemId) const;
        void getItemSize(uint32_t itemId, uint32_t& width, uint32_t& height) const;
        std::vector<std::uint32_t> getDerivationSourcesForItem(std::uint32_t itemId) const;
        FourCCInt getItemType(std::uint32_t itemId) const;
        bool isCodedImage(std::uint32_t itemId) const;
        HEIF::ErrorCode
        checkImageDerivationChains(std::uint32_t itemId, bool gridFound, bool overlayFound, bool isCallerIden) const;
        bool isDerivationSourceColrMatching(std::uint32_t imageId,
                                            Vector<std::uint8_t>& colrData,
                                            bool defaultColorInformationFound,
                                            bool explicitColourInformationFound) const;
        /**
         * @brief isHevcConfigurationBoxConforming
         * @param config Configuration box.
         * @return True if configuration signals conforming to MIAF HEVC profile constraints, false if it does not.
         */
        bool isHevcConfigurationBoxConforming(const HevcConfigurationBox* config, bool isVideoTrack) const;
        bool isAvcConfigurationBoxConforming(const AvcConfigurationBox* config, bool isVideoTrack) const;

        /**
         * @return True if track is an image sequence which follows decoding dependency restrictions of a MIAF
         * burst capture application brand.
         */
        bool isBurstSequence(uint32_t trackId) const;

        /**
         * @return Number of decoded samples needed to present this sample (sample itself and direct and non-direct
         * reference samples), meaning 1 or more.
         */
        unsigned int getDependencyCount(const Vector<HEIF::ImageSequence::Sample>& samples, uint32_t sample) const;

        enum ChromaFormat
        {
            MONOCHROME = 0,
            CHROMA_420 = 1,
            CHROMA_422 = 2,
            CHROMA_444 = 3
        };
        ChromaFormat getChromaFormat(std::uint32_t itemId) const;

        std::uint64_t getItemDataOffset(std::uint32_t itemId) const;
        std::uint64_t getItemDataLastByteOffset(std::uint32_t itemId) const;
        bool isValidEmptyEdit(const HEIF::EditUnit& editUnit) const;
        bool isValidMediaEdit(const HEIF::EditUnit& editUnit, bool onlyForward, bool onlyReverse) const;
        bool getTrackTimeStamps(const HEIF::ImageSequence& sequence,
                                double& duration,
                                DecodePts::PMap& presentationMap) const;
        bool isAlphaPlaneSequence(const HEIF::ImageSequence& sequence) const;
        std::vector<HEIF::SequenceId> getAuxMasterSequences(HEIF::SequenceId auxSequenceId) const;

    private:
        HEIF::WriterImpl* mWriter;

        // Applied application brands
        bool mProgressiveApplicationBrand  = false;
        bool mAnimationApplicationBrand    = false;
        bool mBurstCaptureApplicationBrand = false;
        bool mFragmentedAlphaVideoBrand    = false;
        bool mCommonMediaFragmentedBrand   = false;

        // MIAF profiles (Annex A)
        bool mMiafHevcBasicProfile    = false;
        bool mMiafHevcAdvancedProfile = false;
        bool mMiafHevcExtendedProfile = false;
        bool mMiafAvcBasicProfile     = false;
    };

}  // namespace MIAF

#endif
