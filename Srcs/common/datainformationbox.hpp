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

#ifndef DATAINFORMATIONBOX_HPP
#define DATAINFORMATIONBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "datareferencebox.hpp"

#include <memory>


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
     *  @param [in] dataEntryBox pointer to the DataEntryBox **/
    unsigned int addDataEntryBox(std::shared_ptr<DataEntryBox> dataEntryBox);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a Data Information Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    DataReferenceBox mDataReferenceBox; ///< private member that contains the Data Reference Box
};

#endif /* end of include guard: DATAINFORMATIONBOX_HPP */
