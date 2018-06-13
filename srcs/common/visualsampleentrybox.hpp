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

#ifndef VISUALSAMPLEENTRYBOX_HPP
#define VISUALSAMPLEENTRYBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "sampleentrybox.hpp"


class CleanApertureBox;
class CodingConstraintsBox;
class AuxiliaryTypeInfoBox;

/** @brief VisualSampleEntryBox class. Extends from SampleEntryBox.
 *  @details This box contains information related to the visual samples of the track as defined in the ISOBMFF standard.
 *  @details there may be multiple visual sample entries which map to different samples in the track. */

class VisualSampleEntryBox : public SampleEntryBox
{
public:
    VisualSampleEntryBox(FourCCInt codingName,
                         const String& compressorName);
    VisualSampleEntryBox(const VisualSampleEntryBox& box);

    virtual ~VisualSampleEntryBox() = default;

    /** @brief Sets sample's display width as defined in ISOBMFF
     *  @param [in] width sample's display width */
    void setWidth(std::uint16_t width);

    /** @brief Gets sample's display width as defined in ISOBMFF
     *  @returns Sample's display width */
    std::uint32_t getWidth() const;

    /** @brief Sets sample's display height as defined in ISOBMFF
     *  @param [in] height sample's display height */
    void setHeight(std::uint16_t height);

    /** @brief Gets sample's display height as defined in ISOBMFF
     *  @returns Sample's display height */
    std::uint32_t getHeight() const;

    /** @brief Creates sample's clean aperture data structure as defined in ISOBMFF */
    void createClap();

    /** @brief Gets sample's clean aperture data structure as defined in ISOBMFF
     *  @returns Sample's clean aperture data structure */
    const CleanApertureBox* getClap() const;
    CleanApertureBox* getClap();

    /** @brief Creates sample's AuxiliaryTypeInfoBox as defined in ISOBMFF */
    void createAuxi();

    /** @brief Gets sample's AuxiliaryTypeInfoBox as defined in ISOBMFF
    *  @returns Sample's AuxiliaryTypeInfoBox */
    const AuxiliaryTypeInfoBox* getAuxi() const;
    AuxiliaryTypeInfoBox* getAuxi();

    /** @brief Create an optional HEIF CodingConstraintsBox. */
    virtual void createCodingConstraintsBox()
    {
        // base class implementation intentionally empty
    }

    /** @brief Gets the CodingConstraintsBox from the derived class instance
     *  @return Pointer to CodingConstraintsBox if present, nullptr if not. */
    virtual const CodingConstraintsBox* getCodingConstraintsBox() const
    {
        return nullptr;
    }

    /** @brief Gets the CodingConstraintsBox from the derived class instance
     *  @return Pointer to CodingConstraintsBox if present, nullptr if not. */
    virtual CodingConstraintsBox* getCodingConstraintsBox()
    {
        return nullptr;
    }

    /** @brief Check if CodingConstraintsBox is present
 *  @return TRUE if CodingConstraintsBox is present, FALSE otherwise */
    bool isCodingConstraintsBoxPresent() const
    {
        // Check if pointer to CodingConstraintsBox is valid, doesn't modify anything.
        return (const_cast<VisualSampleEntryBox*>(this)->getCodingConstraintsBox() != nullptr);
    }

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a VisualSampleEntryBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr) override;

    /** @brief Check if this sample entry is a visual sample
    *  @return TRUE */
    virtual bool isVisual() const override;
private:
    std::uint16_t mWidth;                         ///< Sample display width
    std::uint16_t mHeight;                        ///< Sample display height
    String mCompressorName;                       ///< Compressor name used, e.g. "HEVC Coding"
    std::shared_ptr<CleanApertureBox> mClap;      ///< Clean Aperture data structure
    std::shared_ptr<AuxiliaryTypeInfoBox> mAuxi;  ///< HEIF: AuxiliaryTypeInfoBox
};

#endif
