/* Copyright (c) 2017, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#ifndef AVCSAMPLEENTRY_HPP
#define AVCSAMPLEENTRY_HPP

#include "bitstream.hpp"
#include "codingconstraintsbox.hpp"
#include "avcconfigurationbox.hpp"
#include "visualsampleentrybox.hpp"

/** @brief AVC Sample Entry class. Extends from VisualSampleEntry.
 *  @details 'avc1' box implementation as specified in the HEIF and ISO/IEC 14496-15 specifications
 *  @details AvcConfigurationBox and CodingConstraintsBox are mandatory to be present
 *  @todo Implement other AvcSampleEntry internal boxes or extra boxes if needed:
 *  @todo MPEG4BitRateBox, MPEG4ExtensionDescriptorsBox, extra_boxes **/

class AvcSampleEntry : public VisualSampleEntryBox
{
public:
    AvcSampleEntry();
    virtual ~AvcSampleEntry() = default;

    /** @brief Gets the AvcConfigurationBox
     *  @return Reference to the AvcConfigurationBox **/
    AvcConfigurationBox& getAvcConfigurationBox();

    /** @brief Create CodingConstraintsBox */
    void createCodingConstraintsBox();

    /** @brief Gets the CodingConstraintsBox
     *  @return Pointer to CodingConstraintsBox, if present. */
    virtual CodingConstraintsBox* getCodingConstraintsBox() override;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr) override;

    /** @brief Parses a AvcSampleEntry Box bitstream and fills in the necessary member variables
     *  @details If there is an unknown box present then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr) override;

private:
    AvcConfigurationBox mAvcConfigurationBox;
    CodingConstraintsBox mCodingConstraintsBox;
    bool mIsCodingConstraintsPresent;
};

#endif
