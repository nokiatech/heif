/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef HEIFREADERSEGMENT_HPP
#define HEIFREADERSEGMENT_HPP

#include "heiffiledatatypesinternal.hpp"

namespace HEIF
{
    class HeifReaderImpl;

    class Segments
    {
    public:
        Segments(HeifReaderImpl& impl);

        class iterator
        {
        public:
            iterator(HeifReaderImpl& impl, SequenceToSegmentMap& sequenceToSegment, SequenceToSegmentMap::iterator);
            SegmentProperties& operator*() const;
            SegmentProperties* operator->() const;
            iterator& operator++();
            HeifReaderImpl& mImpl;
            SequenceToSegmentMap& mSequenceToSegment;
            SequenceToSegmentMap::iterator mIterator;

            bool operator!=(const iterator& other) const;
        };

        class const_iterator
        {
        public:
            const_iterator(const HeifReaderImpl& impl,
                           const SequenceToSegmentMap& sequenceToSegment,
                           SequenceToSegmentMap::const_iterator);
            const SegmentProperties& operator*() const;
            const SegmentProperties* operator->() const;
            const_iterator& operator++();
            const HeifReaderImpl& mImpl;
            const SequenceToSegmentMap& mSequenceToSegment;
            SequenceToSegmentMap::const_iterator mIterator;

            bool operator!=(const const_iterator& other) const;
        };

        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;

    private:
        HeifReaderImpl& mImpl;
    };

    class ConstSegments
    {
    public:
        ConstSegments(const HeifReaderImpl& impl);

        typedef Segments::const_iterator iterator;
        typedef Segments::const_iterator const_iterator;

        const_iterator begin() const;
        const_iterator end() const;

    private:
        const HeifReaderImpl& mImpl;
    };
}  // namespace HEIF

#endif  // HEIFREADERSEGMENT_HPP
