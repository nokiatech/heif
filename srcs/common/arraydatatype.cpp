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

#define _SCL_SECURE_NO_WARNINGS

#include <algorithm>
#include "customallocator.hpp"
#include "heifcommondatatypes.h"
#if HEIF_READER_LIB
#include "heifreaderdatatypes.h"
#endif
#if HEIF_WRITER_LIB
#include "heifwriterdatatypes.h"
#endif

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

    //common
    template struct Array<DecoderSpecificInfo>;
    template struct Array<FourCC>;
    template struct Array<ImageId>;
    template struct Array<Overlay::Offset>;
    template struct Array<SequenceImageId>;
    template struct Array<char>;
    template struct Array<int16_t>;
    template struct Array<int32_t>;
    template struct Array<int64_t>;
    template struct Array<uint16_t>;
    template struct Array<uint32_t>;
    template struct Array<uint64_t>;
    template struct Array<uint8_t>;

#if HEIF_READER_LIB
    // reader
    template struct Array<EntityGrouping>;
    template struct Array<FourCCToIds>;
    template struct Array<SampleGrouping>;
    template struct Array<ImageInformation>;
    template struct Array<ItemInformation>;
    template struct Array<ItemPropertyInfo>;
    template struct Array<SampleAndEntryIds>;
    template struct Array<SampleInformation>;
    template struct Array<SampleVisualEquivalence>;
    template struct Array<SampleToMetadataItem>;
    template struct Array<SequenceId>;
    template struct Array<TimestampIDPair>;
    template struct Array<TrackInformation>;
#endif
#if HEIF_WRITER_LIB
    // writer
    template struct Array<EditUnit>;
#endif

    template HEIF_DLL_PUBLIC Array<char>::Array(String::iterator begin, String::iterator end);
    template HEIF_DLL_PUBLIC Array<char>::Array(String::const_iterator begin, String::const_iterator end);

}  // namespace HEIF
