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

#include "Heif.h"

namespace HEIFPP
{
    class AlternativeTrackGroup
    {
    public:
        AlternativeTrackGroup(Heif* aHeif);
        virtual ~AlternativeTrackGroup();
        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Returns a pointer to the custom user data. */
        const void* getContext() const;

        /** Returns the parent HEIF object of the item */
        Heif* getHeif();
        const Heif* getHeif() const;

        std::uint32_t getTrackCount() const;
        Track* getTrack(std::uint32_t aId);
        const Track* getTrack(std::uint32_t aId) const;
        Result addTrack(Track* aTrack);
        void removeTrack(Track* aTrack);
        std::vector<Track*>::iterator begin();
        std::vector<Track*>::iterator end();

    protected:
        std::vector<Track*> mTracks;
        Heif* mHeif;
        const void* mContext;
    };
}  // namespace HEIFPP