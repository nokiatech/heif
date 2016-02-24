/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#ifndef HEVCSAMPLEENTRY_HPP
#define HEVCSAMPLEENTRY_HPP

#include "bitstream.hpp"
#include "codingconstraintsbox.hpp"
#include "hevcconfigurationbox.hpp"
#include "visualsampleentrybox.hpp"

/** @brief HEVC Sample Entry class. Extends from VisualSampleEntry.
 *  @details 'hvc1' box implementation as specified in the HEIF and ISO/IEC 14496-15 specifications
 *  @details HevcConfigurationBox and CodingConstraintsBox are mandatory to be present
 *  @todo Implement other HevcSampleEntry internal boxes or extra boxes if needed:
 *  @todo MPEG4BitRateBox, MPEG4ExtensionDescriptorsBox, extra_boxes **/

class HevcSampleEntry : public VisualSampleEntryBox
{
public:
    HevcSampleEntry();
    virtual ~HevcSampleEntry() = default;

    /** @brief Gets the HevcConfigurationBox
     *  @return Reference to the HevcConfigurationBox **/
    HevcConfigurationBox& getHevcConfigurationBox();

    /** @brief Create CodingConstraintsBox */
    void createCodingConstraintsBox();

    /** @brief Gets the CodingConstraintsBox
     *  @return Pointer to CodingConstraintsBox, if present. */
    virtual CodingConstraintsBox* getCodingConstraintsBox() override;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a HevcSampleEntry Box bitstream and fills in the necessary member variables
     *  @details If there is an unknown box present then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    HevcConfigurationBox mHevcConfigurationBox;
    CodingConstraintsBox mCodingConstraintsBox;
    bool mIsCodingConstraintsPresent;
};

#endif
