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

#ifndef SAMPLEENTRYBOX_HPP
#define SAMPLEENTRYBOX_HPP

#include <cstdint>
#include "bbox.hpp"
#include "customallocator.hpp"
#include "decoderconfigrecord.hpp"

/** @brief Sample Entry Box class. Extends from Box.
 *  @details Contains Sample Entry data structure as defined in the ISOBMFF standard. */
class SampleEntryBox : public Box
{
public:
    SampleEntryBox(FourCCInt codingname);
    SampleEntryBox(const SampleEntryBox& box);
    virtual ~SampleEntryBox() = default;

    /** @brief Get Data Reference Index.
     *  @returns Data reference index value */
    std::uint16_t getDataReferenceIndex() const;

    /** @brief Set Data Reference Index.
     *  @param [in] dataReferenceIndex Data reference index value */
    void setDataReferenceIndex(std::uint16_t dataReferenceIndex);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a SampleEntryBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr) override;

    /* @brief Make a copy of this box that has dynamically the same type as this */
    virtual SampleEntryBox* clone() const = 0;

    /* @brief Returns the configuration record for this sample */
    virtual const DecoderConfigurationRecord* getConfigurationRecord() const = 0;

    /* @brief Returns the configuration box for this sample */
    virtual const Box* getConfigurationBox() const = 0;

    /** @brief Check if this sample entry is a visual sample
    *  @return TRUE if visual sample. */
    virtual bool isVisual() const = 0;

private:
    std::uint16_t mDataReferenceIndex;  /// data reference index value
};

#endif
