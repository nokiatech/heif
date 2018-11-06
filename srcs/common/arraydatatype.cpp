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

#define _SCL_SECURE_NO_WARNINGS

#include <algorithm>
#if defined(HEIF_READER_LIB)
#include "heifreaderdatatypes.h"
#endif
#if defined(HEIF_WRITER_LIB)
#include "heifwriterdatatypes.h"
#endif
#include "customallocator.hpp"

namespace HEIF
{
    template <typename T>
    Array<T>::~Array()
    {
        CUSTOM_DELETE_ARRAY(elements, T);
    }

    template <typename T>
    Array<T>::Array()
        : size(0)
        , elements(nullptr)
    {
    }

    template <typename T>
    Array<T>::Array(size_t n)
        : size(n)
        , elements(CUSTOM_NEW_ARRAY(T, n))
    {
    }

    template <typename T>
    Array<T>::Array(const Array& other)
        : size(other.size)
        , elements(CUSTOM_NEW_ARRAY(T, other.size))
    {
        std::copy(other.elements, other.elements + other.size, elements);
    }

    template <typename T>
    Array<T>& Array<T>::operator=(const Array<T>& other)
    {
        if (this != &other)
        {
            CUSTOM_DELETE_ARRAY(elements, T);
            size     = other.size;
            elements = CUSTOM_NEW_ARRAY(T, size);
            std::copy(other.elements, other.elements + other.size, elements);
        }
        return *this;
    }

    template <typename T>
    template <typename U>
    Array<T>::Array(U begin, U end)
        : Array(static_cast<size_t>(std::distance(begin, end)))
    {
        auto it      = begin;
        size_t index = 0;
        while (it != end)
        {
            elements[index] = *it;
            ++it;
            ++index;
        }
    }

    // Put here to avoid? possible binary-incompatibilites with std::initializer_list
    template <typename T>
    Array<T>::Array(std::initializer_list<T> aInit)
        : Array(aInit.size())
    {
        std::copy(aInit.begin(), aInit.end(), elements);
    }

#define instance(X) template struct Array<X>
    template HEIF_DLL_PUBLIC Array<char>::Array(String::iterator begin, String::iterator end);
    template HEIF_DLL_PUBLIC Array<char>::Array(String::const_iterator begin, String::const_iterator end);

    instance(DecoderSpecificInfo);
    instance(FourCC);
    instance(ImageId);
    instance(Overlay::Offset);
    instance(SequenceImageId);
    instance(char);
    instance(int8_t);
    instance(int16_t);
    instance(int32_t);
    instance(int64_t);
    instance(uint8_t);
    instance(uint16_t);
    instance(uint32_t);
    instance(uint64_t);

#if HEIF_READER_LIB
    instance(EntityGrouping);
    instance(FourCCToIds);
    instance(SampleGrouping);
    instance(ItemInformation);
    instance(ItemPropertyInfo);
    instance(SampleAndEntryIds);
    instance(SampleInformation);
    instance(SampleVisualEquivalence);
    instance(SampleToMetadataItem);
    instance(DirectReferenceSamples);
    instance(SequenceId);
    instance(TimestampIDPair);
    instance(TrackInformation);
    instance(EditUnit);

#endif
#if HEIF_WRITER_LIB
    instance(EditUnit);
#endif

}  // namespace HEIF
