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

#ifndef DATAREFERENCEBOX_HPP
#define DATAREFERENCEBOX_HPP

#include <cstdint>

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/** @brief Data Entry Box class. Extends from FullBox.
 *  @details This is the base class for DataEntryUrlBox and DataEntryUrnBox
 *  @details Can be present in DataInformationBox. There can be multiple data entries.
 */
class DataEntryBox : public FullBox
{
public:
    DataEntryBox(FourCCInt boxType, std::uint8_t version, std::uint32_t flags);
    ~DataEntryBox() override = default;

    /** @brief Set the location field as specified in ISOBMFF specification.
     *  @param [in] location value of the location field as a string*/
    void setLocation(const String& location);

    /** @brief Get the location field as specified in ISOBMFF.
     *  @return value of the location field as a string*/
    const String& getLocation() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override = 0;

    /** @brief Parses a Data Entry Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override = 0;

private:
    String mLocation;  ///< Location field as specified in ISOBMFF specification.
};

/** @brief Data Entry URL Box class. Extends from DataEntryBox.
 */
class DataEntryUrlBox : public DataEntryBox
{
public:
    enum IsSelfContained
    {
        NotSelfContained,
        SelfContained
    };

    DataEntryUrlBox(IsSelfContained isSelfContained = NotSelfContained);
    ~DataEntryUrlBox() override = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a Data Entry URL Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;
};

/** @brief Data Entry URN Box class. Extends from DataEntryBox.
 */
class DataEntryUrnBox : public DataEntryBox
{
public:
    DataEntryUrnBox();
    ~DataEntryUrnBox() override = default;

    /** @brief Sets the name field as specified in ISOBMFF specification
     *  @param [in]  name name field as a string */
    void setName(const String& name);

    /** @brief Returns the name field as specified in ISOBMFF specification
     *  @return  name field as a string */
    const String& getName() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a Data Entry URN Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    String mName;  ///< name field as specified in ISOBMFF specification
};

/** @brief Data Reference Box class. Extends from FullBox.
 */
class DataReferenceBox : public FullBox
{
public:
    DataReferenceBox();
    ~DataReferenceBox() override = default;

    /// @return 1-based entry index
    unsigned int addEntry(const std::shared_ptr<DataEntryBox>& dataEntryBox);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a Data Reference Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data
     *  @throws Runtime Error if there is an unknown box inside Data Reference Box */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    /// @todo Preferably use unique_ptr here when writer architecture permits it.
    Vector<std::shared_ptr<DataEntryBox>> mDataEntries;  ///< data reference entries present in the box
};

#endif /* end of include guard: DATAREFERENCEBOX_HPP */
