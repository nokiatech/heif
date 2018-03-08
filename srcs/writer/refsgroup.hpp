/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef REFSGROUP_HPP
#define REFSGROUP_HPP

#include <set>
#include <tuple>
#include "samplegroup.hpp"

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

    typedef Vector<Vector<std::uint32_t>> RefsList;  ///< List of reference groups as a vector of vectors

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
    typedef std::tuple<std::uint32_t, Vector<std::uint32_t>> RefsEntry;  ///< Internal storage of refs entries
    typedef Set<RefsEntry> EntrySet;
    typedef Vector<RefsEntry> IdxEntry;
    typedef Map<std::uint32_t, std::uint32_t> SampleId;

    RefsList mRefsList;  ///< List of refs
    SampleId mSampleId;  ///< Sample id refs mapping
    EntrySet mEntrySet;  ///< Set of Refs entries
    IdxEntry mIdxEntry;  ///< Vector of Refs entries

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
