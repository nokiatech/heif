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

#ifndef REFSGROUP_HPP
#define REFSGROUP_HPP

#include "samplegroup.hpp"
#include <map>
#include <set>
#include <tuple>
#include <vector>

class SampleGroupDescriptionBox;
class SampleToGroupBox;

/**
 * @brief RefsGroup class implementation. Extends from SampleGroup class.
 * @brief Class to generate the ReferencedSampleList sample group.
 */
class RefsGroup : public SampleGroup
{
public:
    RefsGroup();
    virtual ~RefsGroup() = default;

    typedef std::vector<std::vector<std::uint32_t>> RefsList; /// list of reference groups as a vector of vectors

    /**
     * @brief Loads the reference in refsList into a local member variable.
     * @param [in] refsList Reference List to be loaded.
     */
    void loadRefs(const RefsList& refsList);

    /**
     * @brief Fill the SampleGroupDescription Box
     * @details While filling up entries it also makes a list so that it could be used by the fillSbgp
     * method to tie entries to sample.
     * @param [out] sgpd SampleGroupDescriptionBox.
     */
    void fillSgpd(SampleGroupDescriptionBox* sgpd);

    /**
     * @brief Fill the SampleToGroup Box
     * @param [in] sbgp SampleToGroupBox.
     */
    void fillSbgp(SampleToGroupBox& sbgp);

private:
    typedef std::tuple<std::uint32_t, std::vector<std::uint32_t>> RefsEntry; /// internal storage of refs entries
    typedef std::set<RefsEntry>                     EntrySet;
    typedef std::vector<RefsEntry>                  IdxEntry;
    typedef std::map<std::uint32_t, std::uint32_t>  SampleId;

    RefsList mRefsList; /// list of refs
    SampleId mSampleId; /// sample id refs mapping
    EntrySet mEntrySet; /// set of Refs entries
    IdxEntry mIdxEntry; /// vector of Refs entries

    /**
     * @brief Tags a sample in the track with a unique number if the sample is
     * used as a reference for other coded frames.
     * @details If the frame is not used a reference for any other coded frames, the sample is tagged zero. Currently,
     * the tags, if non-zero, are simply the sample index plus one.
     */
    void allotId();

    /**
     * @brief This method makes a set of 2-tuples.
     * @details In a set all elements are unique. The first element of the 2-tuple is a unique tag (or zero) given to a
     * sample. The second element is a vector of reference sample tags. A sample that is Intra coded has no reference
     * sample and hence will have an empty vector as its second element.
     */
    void makeSet();
};
#endif /* end of include guard: REFSGROUP_HPP */

