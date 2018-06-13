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

#ifndef AVCSAMPLEENTRY_HPP
#define AVCSAMPLEENTRY_HPP

#include "avcconfigurationbox.hpp"
#include "bitstream.hpp"
#include "codingconstraintsbox.hpp"
#include "customallocator.hpp"
#include "visualsampleentrybox.hpp"

/** @brief AVC Sample Entry class. Extends from VisualSampleEntry.
 *  @details 'avc1' box implementation as specified in the ISO/IEC 14496-15 specification
 *  @details AvcConfigurationBox and CodingConstraintsBox are mandatory to be present
 *  @todo Implement other AvcSampleEntry internal boxes or extra boxes if needed:
 *  @todo MPEG4BitRateBox, MPEG4ExtensionDescriptorsBox, extra_boxes */

class AvcSampleEntry : public VisualSampleEntryBox
{
public:
    AvcSampleEntry();
    AvcSampleEntry(const AvcSampleEntry& box);
    virtual ~AvcSampleEntry() = default;

    /** @brief Gets the AvcConfigurationBox
    *  @return Reference to the AvcConfigurationBox */
    AvcConfigurationBox& getAvcConfigurationBox();

    /** @brief Gets the AvcConfigurationBox
    *  @return Reference to the AvcConfigurationBox */
    const AvcConfigurationBox& getAvcConfigurationBox() const;

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

    /** @brief Parses a AvcSampleEntry Box bitstream and fills in the necessary member variables
     *  @details If there is an unknown box present then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr) override;

    /* @brief Make a copy of this box that has dynamically the same type as this */
    virtual AvcSampleEntry* clone() const override;

    /* @brief Returns the configuration record for this sample */
    virtual const DecoderConfigurationRecord* getConfigurationRecord() const override;

    /* @brief Returns the configuration box for this sample */
    virtual const Box* getConfigurationBox() const override;

private:
    AvcConfigurationBox mAvcConfigurationBox;
    CodingConstraintsBox mCodingConstraintsBox;
    bool mIsCodingConstraintsPresent;
};

#endif
