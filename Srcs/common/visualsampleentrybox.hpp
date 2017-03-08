/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef VISUALSAMPLEENTRYBOX_HPP
#define VISUALSAMPLEENTRYBOX_HPP

#include "bitstream.hpp"
#include "sampleentrybox.hpp"

#include <memory>
#include <string>

class CleanAperture;
class CodingConstraintsBox;

/** @brief VisualSampleEntryBox class. Extends from SampleEntryBox.
 *  @details This box contains information related to the visual samples of the track as defined in the ISOBMFF and HEIF standards.
 *  @details there may be multiple visual sample entries which map to different samples in the track. **/

class VisualSampleEntryBox : public SampleEntryBox
{
public:
    VisualSampleEntryBox(FourCCInt codingName,
                         const std::string& compressorName);

    virtual ~VisualSampleEntryBox() = default;

    /** @brief Sets sample's display width as defined in ISOBMFF
     *  @param [in] width sample's display width **/
    void setWidth(std::uint32_t width);

    /** @brief Gets sample's display width as defined in ISOBMFF
     *  @returns Sample's display width **/
    std::uint32_t getWidth() const;

    /** @brief Sets sample's display height as defined in ISOBMFF
     *  @param [in] height sample's display height **/
    void setHeight(std::uint32_t height);

    /** @brief Gets sample's display height as defined in ISOBMFF
     *  @returns Sample's display height **/
    std::uint32_t getHeight() const;

    /** @brief Gets sample's clean aperture data structure as defined in ISOBMFF
     *  @returns Sample's clean aperture data structure **/
    const CleanAperture* getClap() const;

    /** @brief Gets the CodingConstraintsBox from the derived class instance
     *  @return Pointer to CodingConstraintsBox if present, nullptr if not. */
    virtual CodingConstraintsBox* getCodingConstraintsBox() { return nullptr; }

    /** @brief Check if CodingConstraintsBox is present
     *  @return TRUE if CodingConstraintsBox is present, FALSE otherwise */
    bool isCodingConstraintsBoxPresent() const
    {
        // Check if pointer to CodingConstraintsBox is valid, doesn't modify anything.
        return const_cast<VisualSampleEntryBox*>(this)->getCodingConstraintsBox();
    }

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a VisualSampleEntryBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::uint16_t mWidth;        ///< Sample display width
    std::uint16_t mHeight;       ///< Sample display height
    std::string mCompressorName; ///< Compressor name used, e.g. "HEVC Coding"
    std::shared_ptr<CleanAperture> mClap; ///< Clean Aperture data structure
};

#endif
