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

#ifndef DATAREFERENCEBOX_HPP
#define DATAREFERENCEBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/** @brief Data Entry Box class. Extends from FullBox.
 *  @details This is the base class for DataEntryUrlBox and DataEntryUrnBox
 *  @details Can be present in DataInformationBox. There can be multiple data entries.
 */
class DataEntryBox : public FullBox
{
public:
    DataEntryBox(const FourCCInt boxType, std::uint8_t version, std::uint32_t flags);
    virtual ~DataEntryBox() = default;

    /** @brief Set the location field as specificed in ISOBMFF specification.
     *  @param [in] location value of the location field as a string**/
    void setLocation(const std::string& location);

    /** @brief Get the location field as specificed in ISOBMFF.
     *  @return value of the location field as a string**/
    const std::string getLocation() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr) = 0;

    /** @brief Parses a Data Entry Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr) = 0;

private:
    std::string mLocation;  ///< location field as specificed in ISOBMFF specification.
};

/** @brief Data Entry URL Box class. Extends from DataEntryBox.
 */
class DataEntryUrlBox : public DataEntryBox
{
public:
    DataEntryUrlBox();
    virtual ~DataEntryUrlBox() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a Data Entry URL Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);
};

/** @brief Data Entry URN Box class. Extends from DataEntryBox.
 */
class DataEntryUrnBox : public DataEntryBox
{
public:
    DataEntryUrnBox();
    virtual ~DataEntryUrnBox() = default;

    /** @brief Sets the name field as specified in ISOBMFF specification
     *  @param [in]  name name field as a string */
    void setName(const std::string& name);

    /** @brief Returns the name field as specified in ISOBMFF specification
     *  @return  name field as a string */
    const std::string getName() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a Data Entry URN Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::string mName; ///< name field as specificed in ISOBMFF specification
};


/** @brief Data Reference Box class. Extends from FullBox.
 *  @details Based on the current HEIF specification, there must be at least one entry in this box (URL), eventhough it contains an empty string
 */
class DataReferenceBox : public FullBox
{
public:
    DataReferenceBox();
    virtual ~DataReferenceBox() = default;

    /// @return 1-based entry index
    unsigned int addEntry(std::shared_ptr<DataEntryBox> dataEntryBox);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a Data Reference Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data
     *  @throws Runtime Error if there is an unknown box inside Data Reference Box */
    virtual void parseBox(BitStream& bitstr);

private:
    /// @todo Preferably use unique_ptr here when writer architecture permits it.
    std::vector<std::shared_ptr<DataEntryBox>> mDataEntries; ///< data reference entries present in the box
};

#endif /* end of include guard: DATAREFERENCEBOX_HPP */
