/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <VideoTrack.h>

namespace HEIFPP
{
    class CleanApertureProperty;
    class AuxProperty;

    class ImageSequence : public VideoTrack
    {
    public:
        ImageSequence(Heif* aHeif);
        ~ImageSequence();

        const HEIF::CleanAperture* clap() const;
        const HEIF::AuxiliaryType* aux() const;

        //
        bool getAllRefPicsIntra() const;  ///< This flag when set to one indicates the restriction that samples that are
                                          ///< not sync samples, if any, are predicted only from sync samples.
        bool getIntraPredUsed()
            const;  ///< False indicates that intra prediction is not used in the inter predicted images. True
                    ///< indicates that intra prediction may or may not be used in the inter predicted images.
        std::uint8_t
        getMaxRefPerPic() const;  ///< Maximum number of reference images that may be used for decoding any single image

        void setAllRefPicsIntra(bool);  ///< This flag when set to one indicates the restriction that samples that are
                                        ///< not sync samples, if any, are predicted only from sync samples.
        void setIntraPredUsed(
            bool);  ///< False indicates that intra prediction is not used in the inter predicted images. True
                    ///< indicates that intra prediction may or may not be used in the inter predicted images.
        void setMaxRefPerPic(
            std::uint8_t);  ///< Maximum number of reference images that may be used for decoding any single image

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::SequenceId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;
        bool mHasAux, mHasClap, mHasCodingConstraints;
        HEIF::AuxiliaryType mAuxProperty;
        HEIF::CleanAperture mClapProperty;
        HEIF::CodingConstraints mCodingConstraints;

    private:
        ImageSequence& operator=(const ImageSequence&) = delete;
        ImageSequence& operator=(ImageSequence&&)      = delete;
        ImageSequence(const ImageSequence&)            = delete;
        ImageSequence(ImageSequence&&)                 = delete;
        ImageSequence()                                = delete;
    };
}  // namespace HEIFPP
