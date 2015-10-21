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

#include "imagemediawriter.hpp"
#include "h265parser.hpp"
#include "mediadatabox.hpp"

ImageMediaWriter::ImageMediaWriter(const std::string& fileName) :
    mFilePath(fileName)
{
}

MediaDataBox ImageMediaWriter::writeMedia()
{
    MediaDataBox mdat;
    H265Parser mediaParser;
    const bool isOpen = (mediaParser.openFile(mFilePath.c_str())) ? true : false;
    if (!isOpen)
    {
        throw std::runtime_error("Not able to open H.265 bit stream file '" + mFilePath + "'");
    }

    ParserInterface::AccessUnit* accessUnit = new ParserInterface::AccessUnit { };

    const bool hasNalUnits = (accessUnit->mNalUnits.size() > 0) ? true : false;
    bool hasMoreImages = (mediaParser.parseNextAU(*accessUnit));
    const bool hasSpsNalUnits = (accessUnit->mSpsNalUnits.size() > 0) ? true : false;
    const bool hasPpsNalUnits = (accessUnit->mPpsNalUnits.size() > 0) ? true : false;
    const bool isHevc = hasMoreImages || hasNalUnits || hasSpsNalUnits || hasPpsNalUnits;

    if (isHevc)
    {
        // Decoder configuration in the beginning is discarded
        while (hasMoreImages)
        {
            if (accessUnit == nullptr)
            {
                accessUnit = new ParserInterface::AccessUnit { };
                hasMoreImages = mediaParser.parseNextAU(*accessUnit);
            }
            if (hasMoreImages)
            {
                mdat.addListOfNalData(accessUnit->mNalUnits);
            }
            delete accessUnit;
            accessUnit = nullptr;
        }
    }

    return mdat;
}
