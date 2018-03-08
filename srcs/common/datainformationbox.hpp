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

#ifndef DATAINFORMATIONBOX_HPP
#define DATAINFORMATIONBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "datareferencebox.hpp"


/** @brief Data Information Box class
 *  @details 'dinf' box implementation. Extends from Box.
 *  @details  Can be present in both 'minf' and 'meta' boxes.
 */
class DataInformationBox : public Box
{
public:
    DataInformationBox();
    virtual ~DataInformationBox() = default;

    /** @brief Add a Data Entry Box. there can be exactly one Data Entry Box.
     *  @param [in] dataEntryBox pointer to the DataEntryBox */
    std::uint16_t addDataEntryBox(std::shared_ptr<DataEntryBox> dataEntryBox);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a Data Information Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    DataReferenceBox mDataReferenceBox;  ///< private member that contains the Data Reference Box
};

#endif /* end of include guard: DATAINFORMATIONBOX_HPP */
