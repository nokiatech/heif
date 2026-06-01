/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */
#pragma once

#include <heifcommondatatypes.h>

#include <cstdint>
#include <vector>

namespace HEIFPP
{
    class Heif;
    class Item;
    class Track;
    class Sample;
}  // namespace HEIFPP

namespace HEIFPP
{
    class EntityGroup
    {
        friend class Heif;

    public:
        EntityGroup(Heif* aHeif, const HEIF::FourCC& aType);
        virtual ~EntityGroup();

        void setContext(const void* aContext);
        const void* getContext() const;

        const HEIF::GroupId& getId() const noexcept;
        const HEIF::FourCC& getType() const;
        std::uint32_t getEntityCount() const;

        bool isItem(std::uint32_t) const;
        Item* getItem(std::uint32_t);
        const Item* getItem(std::uint32_t) const;

        bool isTrack(std::uint32_t) const;
        Track* getTrack(std::uint32_t);
        const Track* getTrack(std::uint32_t) const;

        bool isSample(std::uint32_t) const;
        Sample* getSample(std::uint32_t);
        const Sample* getSample(std::uint32_t) const;


        void addItem(Item* aItem);
        void addTrack(Track* aTrack);
        void addSample(Sample* aSample);

        virtual void removeEntity(std::uint32_t);
        void removeItem(Item* aItem);
        void removeTrack(Track* aTrack);
        virtual void removeSample(Sample* aSample);

        bool hasItems() const;
        bool hasTracks() const;
        bool hasSamples() const;

    protected:
        void setId(const HEIF::GroupId& aId);
        bool addEntity(Item*, Track*, Sample*, std::int32_t&);
        bool removeEntity(Item*, Track*, Sample*, std::int32_t&);

    private:
        Heif* mHeif;
        HEIF::GroupId mId;
        HEIF::FourCC mType;
        class Entity
        {
        public:
            explicit Entity(Item* aItem) noexcept;
            explicit Entity(Track* aTrack) noexcept;
            explicit Entity(Sample* aSample) noexcept;
            virtual ~Entity();
            bool isItem() const;
            bool isTrack() const;
            bool isSample() const;
            Item* item();
            Track* track();
            Sample* sample();
            const Item* item() const;
            const Track* track() const;
            const Sample* sample() const;
            void removeFromGroup(EntityGroup*);
            void addToGroup(EntityGroup*);

        private:
            Item* mItem;
            Sample* mSample;
            Track* mTrack;
        };
        std::vector<Entity> mItems;
        const void* mContext{nullptr};

    public:
        EntityGroup& operator=(const EntityGroup&) = delete;
        EntityGroup& operator=(EntityGroup&&) = delete;
        EntityGroup(const EntityGroup&)       = delete;
        EntityGroup(EntityGroup&&)            = delete;
    };
}  // namespace HEIFPP
