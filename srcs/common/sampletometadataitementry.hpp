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

#ifndef SAMPLETOMETADATAITEMENTRY_HPP
#define SAMPLETOMETADATAITEMENTRY_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "samplegroupdescriptionentry.hpp"

/** @brief SampleToMetadataItemEntry class. Inherits from SampleGroupDescriptionEntry.
 *  @details Implements SampleToMetadataItemEntry as defined in HEIF standard. */
class SampleToMetadataItemEntry : public SampleGroupDescriptionEntry
{
public:
    SampleToMetadataItemEntry();
    ~SampleToMetadataItemEntry() override = default;

    /** @brief Set the metabox handler type
     *  @param [in] FourCCInt of metadata handler type*/
    void setMetaBoxHandlerType(FourCCInt type);

    /** @brief Get the metabox handler type
     *  @returns FourCCInt metabox handler type */
    FourCCInt getMetaBoxHandlerType() const;

    /** @brief Set the item IDs
     *  @param [in]  Vector<std::uint32_t> of item IDs */
    void setItemIds(const Vector<std::uint32_t>& itemIds);

    /** @brief Get the item IDs
     *  @returns FourCCInt metabox handler type */
    Vector<uint32_t>& getItemIds();

    /** @brief Get the item IDs
     *  @returns FourCCInt metabox handler type */
    const Vector<uint32_t>& getItemIds() const;

    /** @see SampleGroupEntry::getSize() */
    /** @brief Get the serialized byte size of VisualEquivalenceEntry.
     *  @return Byte size of the Entry*/
    std::uint32_t getSize() const override;

    /** @brief Serialize the VisualEquivalenceEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized VisualEquivalenceEntry data structure */
    void writeEntry(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parse a serialized VisualEquivalenceEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized VisualEquivalenceEntry data structure */
    void parseEntry(ISOBMFF::BitStream& bitstr) override;

private:
    std::uint32_t mMetaBoxHandlerType;
    Vector<uint32_t> mItemIds;
};

#endif /* end of include guard: SAMPLETOMETADATAITEMENTRY_HPP */
