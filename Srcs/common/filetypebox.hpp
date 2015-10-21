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

#ifndef FILETYPEBOX_HPP
#define FILETYPEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"

#include <string>

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
     *  @param [in] majorBrand Major Brand as string value and as defined in relevant file format standards **/
    void setMajorBrand(const std::string& majorBrand);

    /** @brief Get the Major Brand
     *  @returns Major Brand as string value **/
    const std::string& getMajorBrand() const;

    /** @brief Set the minor version
     *  @param [in] minorVersion Minor Version as an unsigned integer value. **/
    void setMinorVersion(std::uint32_t minorVersion);

    /** @brief Get the minor version
     *  @returns Minor Version as an unsigned integer value. **/
    uint32_t getMinorVersion() const;

    /** @brief Adds a compatible brand to the list of compatible brands
     *  @param [in] compatibleBrand Compatible Brand as string value and as defined in relevant file format standards **/
    void addCompatibleBrand(const std::string& compatibleBrand);

    /** @brief Get the Compatible Brands List
     *  @returns Compatible Brand as vector of string values **/
    std::vector<std::string> getCompatibleBrands() const;

    /** @brief Check if a brand is among the list of compatible brands
     *  @param [in] brand Brand value to be checked
     *  @returns TRUE: if the brand is in the compatible brands list. FALSE otherwise. **/
    bool checkCompatibleBrand(const std::string& brand) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a File Type Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::string mMajorBrand; ///< Major Brand as string value
    std::uint32_t mMinorVersion; ///< Minor version as an unsigned integer
    std::vector<std::string> mCompatibleBrands; ///< Vector containing the Compatible Brands as strings
};

#endif /* end of include guard: FILETYPEBOX_HPP */

