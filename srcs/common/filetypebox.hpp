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

#ifndef FILETYPEBOX_HPP
#define FILETYPEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"


/** @brief File Type Box class. Extends from Box.
 *  @details 'ftyp' box implementation as specified in the ISOBMFF specification
 *  @details Major and Compatible Brands List are present in this box.
 */

class FileTypeBox : public Box
{
public:
    FileTypeBox();
    virtual ~FileTypeBox() = default;

    /** @brief Set the Major Brand
     *  @param [in] majorBrand Major Brand as string value and as defined in relevant file format standards */
    void setMajorBrand(const FourCCInt& majorBrand);

    /** @brief Get the Major Brand
     *  @returns Major Brand as string value */
    const FourCCInt& getMajorBrand() const;

    /** @brief Set the minor version
     *  @param [in] minorVersion Minor Version as an unsigned integer value. */
    void setMinorVersion(std::uint32_t minorVersion);

    /** @brief Get the minor version
     *  @returns Minor Version as an unsigned integer value. */
    uint32_t getMinorVersion() const;

    /** @brief Adds a compatible brand to the list of compatible brands
     *  @param [in] compatibleBrand Compatible Brand as string value and as defined in relevant file format standards */
    void addCompatibleBrand(const FourCCInt& compatibleBrand);

    /** @brief Get the Compatible Brands List
     *  @returns Compatible Brand as vector of string values */
    const Vector<FourCCInt>& getCompatibleBrands() const;

    /** @brief Check if a brand is among the list of compatible brands
     *  @param [in] brand Brand value to be checked
     *  @returns TRUE: if the brand is in the compatible brands list. FALSE otherwise. */
    bool checkCompatibleBrand(const FourCCInt& brand) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a File Type Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    FourCCInt mMajorBrand;                ///< Major Brand 4CC identifier
    std::uint32_t mMinorVersion;          ///< Minor version as an unsigned integer
    Vector<FourCCInt> mCompatibleBrands;  ///< Vector containing the Compatible Brands as strings
};

#endif /* end of include guard: FILETYPEBOX_HPP */
