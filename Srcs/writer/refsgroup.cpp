/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "refsgroup.hpp"
#include "samplegroupdescriptionbox.hpp"
#include "samplegroupentry.hpp"
#include "sampletogroupbox.hpp"
#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <tuple>

RefsGroup::RefsGroup() :
    SampleGroup("refs")
{
}

void RefsGroup::loadRefs(const RefsList& refsList)
{
    mRefsList = refsList;
}

void RefsGroup::allotId(void)
{
    // First insert all reference pictures into a set
    std::set<std::uint32_t> rps;
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
    // std::map<std::uint32_t, std::uint32_t> sampleTag;
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
        std::vector<std::uint32_t> v = sample;
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
        std::vector<std::uint32_t> refs;
        std::tie(tag, refs) = entry;

        std::unique_ptr<DirectReferenceSampleListEntry> refsEntry(new DirectReferenceSampleListEntry);
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
    std::vector<std::uint32_t> entryIdx;
    for (auto sample : mRefsList)
    {
        std::vector<std::uint32_t> v = sample;
        for (std::uint32_t& refIdx : v)
        {
            refIdx += 1;
        }
        RefsEntry element = std::make_tuple(mSampleId.at(sampleIdx), v);
        entryIdx.push_back(std::find(mIdxEntry.begin(), mIdxEntry.end(), element) - mIdxEntry.begin() + 1);
        sampleIdx += 1;
    }

    std::vector<std::tuple<std::uint32_t, std::uint32_t>> runsList;
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

