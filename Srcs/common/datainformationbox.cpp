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

#include "datainformationbox.hpp"
#include "log.hpp"

#include <stdexcept>

DataInformationBox::DataInformationBox() :
    Box("dinf"),
    mDataReferenceBox()
{
}

unsigned int DataInformationBox::addDataEntryBox(std::shared_ptr<DataEntryBox> dataEntryBox)
{
    return mDataReferenceBox.addEntry(dataEntryBox);
}

void DataInformationBox::writeBox(BitStream& bitstr)
{
    writeBoxHeader(bitstr);
    mDataReferenceBox.writeBox(bitstr);
    updateSize(bitstr);
}

void DataInformationBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    if (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);
        mDataReferenceBox.parseBox(subBitstr);
    }
    else
    {
        logError() << "Read an empty dinf box." << std::endl;
    }
}
