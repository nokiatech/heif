/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#endif
#include "customallocator.hpp"
#include "heifcommondatatypes.h"
#include <initializer_list>
#include <stddef.h>
#include <stdint.h>
#include <string>

namespace HEIF
{
    template <typename T>
    Array<T>::~Array()
    {
        customDestructAndDeallocateArray<T>(elements);
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
        , elements(customAllocateArray<T>(n))
    {
    }

    template <typename T>
    Array<T>::Array(const Array& other)
        : size(other.size)
        , elements(customAllocateArray<T>(other.size))
    {
        std::copy(other.elements, other.elements + other.size, elements);
    }

    template <typename T>
    Array<T>& Array<T>::operator=(const Array<T>& other)
    {
        if (this != &other)
        {
            customDestructAndDeallocateArray<T>(elements);
            size     = other.size;
            elements = customAllocateArray<T>(size);
            std::copy(other.elements, other.elements + other.size, elements);
        }
        return *this;
    }

    // Put here to avoid? possible binary-incompatibilites with std::initializer_list
    template <typename T>
    Array<T>::Array(std::initializer_list<T> aInit)
        : Array(aInit.size())
    {
        std::copy(aInit.begin(), aInit.end(), elements);
    }

#define instance(X) template struct Array<X>
    template Array<char>::Array(String::iterator begin, String::iterator end);
    template Array<char>::Array(String::const_iterator begin, String::const_iterator end);

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
    instance(Array<FourCC>);
    instance(bool);
    instance(std::string);
    instance(GainMapData::Channel);
    instance(PixelInformation::ChannelEntry);
    instance(OperatingPointsInfo::ProfileTierLevel);
    instance(OperatingPointsInfo::OperatingPoint);
    instance(OperatingPointsInfo::OperatingPointLayer);
    instance(OperatingPointsInfo::LayerInfo);
    instance(SubSampleInfo::SubSampleEntry);

#ifdef HEIF_READER_LIB
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
    instance(RectangularRegion);
    instance(VvcSubpicOrder);
    instance(VvcSubpicLayoutMap);
    instance(VvcMixedNalUnitTypePic);
    instance(SequenceId);
    instance(TimestampIDPair);
    instance(TrackInformation);
    instance(EditUnit);
    instance(SegmentInformation);

#endif
#ifdef HEIF_WRITER_LIB
    instance(EditUnit);
#endif

}  // namespace HEIF
