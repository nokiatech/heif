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

#ifndef VVCSUBPICSAMPLEENTRY_HPP
#define VVCSUBPICSAMPLEENTRY_HPP

#include "bitstream.hpp"
#include "visualsampleentrybox.hpp"
#include "vvcnaluconfigbox.hpp"

/** @brief VVC Subpic Sample Entry class. Extends from VisualSampleEntry.
 */

class VvcSubpicSampleEntry : public VisualSampleEntryBox
{
public:
    VvcSubpicSampleEntry();
    VvcSubpicSampleEntry(const VvcSubpicSampleEntry& other);
    ~VvcSubpicSampleEntry() override = default;

    /** @brief Gets the VvcConfigurationBox
     *  @return Reference to the VvcConfigurationBox */
    VvcNaluConfigBox& getVvcNaluConfigBox();

    /** @brief Gets the VvcConfigurationBox
     *  @return Reference to the contained VvcConfigurationBox */
    const VvcNaluConfigBox& getVvcNaluConfigBox() const;

    /**
     * @brief Set VVC NAL unit configuration box of the entry
     * @param [in] config VVC NAL unit configuration box
     */
    void setVvcNaluConfigBox(const VvcNaluConfigBox& config);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a VvcSubpicSampleEntry Box bitstream and fills in the necessary member variables
     *  @details If there is an unknown box present then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

    /** @brief Returns always nullptr as a 'vvs1' sample entry does not contain a configuration record. */
    const DecoderConfigurationRecord* getConfigurationRecord() const override;

    /** @brief Returns the configuration box for this sample */
    const Box* getConfigurationBox() const override;

    VvcSubpicSampleEntry* clone() const override;

private:
    VvcNaluConfigBox mConfig;
};

#endif
