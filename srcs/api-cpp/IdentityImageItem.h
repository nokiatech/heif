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

#include <DerivedImageItem.h>
namespace HEIFPP
{
    class Identity : public HEIFPP::DerivedImageItem
    {
    public:
        Identity(Heif* aHeif);
        virtual ~Identity() = default;

        /** Returns the origin image of the derived image */
        ImageItem* getImage();
        const ImageItem* getImage() const;

        /** Sets the origin image of the derived image
         * @param [in] aImage: Origin image. */
        void setImage(ImageItem* aImage);

        /** Removes the given image from the origin
         * @param [in] aImage: Image to be removed */
        Result removeImage(ImageItem* aImage);

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        Identity& operator=(const Identity&) = delete;
        Identity(const Identity&)            = delete;
        Identity(Identity&&)                 = delete;
        Identity()                           = delete;
    };

}  // namespace HEIFPP
