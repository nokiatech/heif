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

#ifndef VVCSAMPLEENTRY_HPP
#define VVCSAMPLEENTRY_HPP

#include "bitstream.hpp"
#include "codingconstraintsbox.hpp"
#include "customallocator.hpp"
#include "visualsampleentrybox.hpp"
#include "vvcconfigurationbox.hpp"

/** @brief VVC Sample Entry class. Extends from VisualSampleEntry.
 *  @details 'vvc1' box implementation as specified in the ISO/IEC 14496-15 specification
 *  @todo Add optional MPEG4ExtensionDescriptorsBox.
 */

class VvcSampleEntry : public VisualSampleEntryBox
{
public:
    VvcSampleEntry();
    VvcSampleEntry(const VvcSampleEntry& other);
    ~VvcSampleEntry() override = default;

    /** @brief Gets the VvcConfigurationBox
     *  @return Reference to the VvcConfigurationBox */
    VvcConfigurationBox& getVvcConfigurationBox();

    /** @brief Gets the VvcConfigurationBox
     *  @return Reference to the VvcConfigurationBox */
    const VvcConfigurationBox& getVvcConfigurationBox() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a VvcSampleEntry Box bitstream and fills in the necessary member variables
     *  @details If there is an unknown box present then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

    VvcSampleEntry* clone() const override;

    /* @brief Returns the configuration record for this sample */
    const DecoderConfigurationRecord* getConfigurationRecord() const override;

    /* @brief Returns the configuration box for this sample */
    const Box* getConfigurationBox() const override;

private:
    VvcConfigurationBox mVvcConfigurationBox;
};

#endif
