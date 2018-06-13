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

#ifndef HEVCSAMPLEENTRY_HPP
#define HEVCSAMPLEENTRY_HPP

#include "bitstream.hpp"
#include "codingconstraintsbox.hpp"
#include "customallocator.hpp"
#include "hevcconfigurationbox.hpp"
#include "visualsampleentrybox.hpp"

/** @brief HEVC Sample Entry class. Extends from VisualSampleEntry.
 *  @details 'hvc1' box implementation as specified in the ISO/IEC 14496-15 specification
 *  @details HevcConfigurationBox and CodingConstraintsBox are mandatory to be present
 *  @todo Implement other HevcSampleEntry internal boxes or extra boxes if needed:
 *  @todo MPEG4BitRateBox, MPEG4ExtensionDescriptorsBox, extra_boxes */

class HevcSampleEntry : public VisualSampleEntryBox
{
public:
    HevcSampleEntry();
    HevcSampleEntry(const HevcSampleEntry& other);
    virtual ~HevcSampleEntry() = default;

    /** @brief Gets the HevcConfigurationBox
    *  @return Reference to the HevcConfigurationBox */
    HevcConfigurationBox& getHevcConfigurationBox();

    /** @brief Gets the HevcConfigurationBox
    *  @return Reference to the HevcConfigurationBox */
    const HevcConfigurationBox& getHevcConfigurationBox() const;

    /** @brief Create CodingConstraintsBox */
    virtual void createCodingConstraintsBox() override;

    /** @brief Gets the CodingConstraintsBox
     *  @return Pointer to CodingConstraintsBox, if present. */
    virtual const CodingConstraintsBox* getCodingConstraintsBox() const override;

    /** @brief Gets the CodingConstraintsBox
     *  @return Pointer to CodingConstraintsBox, if present. */
    virtual CodingConstraintsBox* getCodingConstraintsBox() override;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a HevcSampleEntry Box bitstream and fills in the necessary member variables
     *  @details If there is an unknown box present then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr) override;

    virtual HevcSampleEntry* clone() const override;

    /* @brief Returns the configuration record for this sample */
    virtual const DecoderConfigurationRecord* getConfigurationRecord() const override;

    /* @brief Returns the configuration box for this sample */
    virtual const Box* getConfigurationBox() const override;

private:
    HevcConfigurationBox mHevcConfigurationBox;
    CodingConstraintsBox mCodingConstraintsBox;
    bool mIsCodingConstraintsPresent;
};

#endif
