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

#include "EntityGroup.h"
#include "Heif.h"

namespace HEIFPP
{
    class EqivGroup : public EntityGroup
    {
    public:
        EqivGroup(Heif* aHeif);
        void addSample(Sample* aItem, std::int16_t aOffset, std::uint16_t aMultiplier);
        std::int16_t getOffset(Sample* aItem);
        std::uint16_t getMultiplier(Sample* aItem);
        virtual void removeEntity(std::uint32_t aIndex);
        virtual void removeSample(Sample* aSample);

    protected:
        std::map<Sample*, std::pair<std::int16_t, std::uint16_t>> mOffsets;

    private:
        EqivGroup& operator=(const EqivGroup&) = delete;
        EqivGroup& operator=(const EqivGroup&&) = delete;
        EqivGroup(const EqivGroup&)             = delete;
        EqivGroup(EqivGroup&&)                  = delete;
    };
}  // namespace HEIFPP