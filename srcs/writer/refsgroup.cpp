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

#include "refsgroup.hpp"
#include <algorithm>
#include <iterator>
#include <set>
#include <tuple>
#include "directreferencesampleslist.hpp"
#include "samplegroupdescriptionbox.hpp"
#include "samplegroupdescriptionentry.hpp"
#include "sampletogroupbox.hpp"

RefsGroup::RefsGroup()
    : SampleGroup("refs")
{
}

void RefsGroup::loadRefs(const RefsList& refsList)
{
    mRefsList = refsList;
}

void RefsGroup::allotId(void)
{
    // First insert all reference pictures into a set
    Set<std::uint32_t> rps;
    for (auto sample : mRefsList)
    {
        for (auto refIdx : sample)
        {
            rps.insert(refIdx);
        }
    }

    // Tag samples:
    //  - every reference picture is give a non-zero positive tag
    //  - every non-reference picture is give a tag a zero
    // The non-zero tags are currently set to sample index value + 1
    // Map<std::uint32_t, std::uint32_t> sampleTag;
    std::uint32_t sampleIdx = 0;
    for (auto sample : mRefsList)
    {
        if (rps.find(sampleIdx) != rps.end())
        {
            mSampleId.insert(std::make_pair(sampleIdx, sampleIdx + 1));
        }
        else
        {
            mSampleId.insert(std::make_pair(sampleIdx, 0));
        }
        sampleIdx += 1;
    }
}

void RefsGroup::makeSet(void)
{
    // Construct a tuple and dump into a set
    std::uint32_t sampleIdx = 0;
    for (auto sample : mRefsList)
    {
        Vector<std::uint32_t> v = sample;
        for (std::uint32_t& refIdx : v)
        {
            refIdx += 1;
        }
        mEntrySet.insert(std::make_tuple(mSampleId.at(sampleIdx), v));
        sampleIdx += 1;
    }
}

void RefsGroup::fillSgpd(SampleGroupDescriptionBox* sgpd)
{
    allotId();
    makeSet();

    sgpd->FullBox::setVersion(1);
    sgpd->setGroupingType(mGroupingType);
    for (auto entry : mEntrySet)
    {
        std::uint32_t tag;
        Vector<std::uint32_t> refs;
        std::tie(tag, refs) = entry;

        UniquePtr<DirectReferenceSamplesList, SampleGroupDescriptionEntry> refsEntry(
            CUSTOM_NEW(DirectReferenceSamplesList, ()));
        refsEntry->setSampleId(tag);
        refsEntry->setDirectReferenceSampleIds(refs);
        sgpd->addEntry(std::move(refsEntry));

        mIdxEntry.push_back(std::make_tuple(tag, refs));
    }
}


void RefsGroup::fillSbgp(SampleToGroupBox& sbgp)
{
    sbgp.setGroupingType(mGroupingType);

    std::uint32_t sampleIdx = 0;
    Vector<std::uint32_t> entryIdx;
    for (auto sample : mRefsList)
    {
        Vector<std::uint32_t> v = sample;
        for (std::uint32_t& refIdx : v)
        {
            refIdx += 1;
        }
        RefsEntry element = std::make_tuple(mSampleId.at(sampleIdx), v);
        entryIdx.push_back(
            static_cast<uint32_t>(std::find(mIdxEntry.begin(), mIdxEntry.end(), element) - mIdxEntry.begin() + 1));
        sampleIdx += 1;
    }

    Vector<std::tuple<std::uint32_t, std::uint32_t>> runsList;
    std::uint32_t prevIdx = entryIdx.at(0);
    std::uint32_t idxRuns = 0;
    for (auto currIdx : entryIdx)
    {
        if (currIdx == prevIdx)
        {
            idxRuns += 1;
        }
        else
        {
            runsList.push_back(std::make_tuple(prevIdx, idxRuns));
            prevIdx = currIdx;
            idxRuns = 1;
        }
    }
    runsList.push_back(std::make_tuple(prevIdx, idxRuns));

    for (auto entry : runsList)
    {
        std::uint32_t idx;
        std::uint32_t run;
        std::tie(idx, run) = entry;
        sbgp.addSampleRun(run, idx);
    }
}
