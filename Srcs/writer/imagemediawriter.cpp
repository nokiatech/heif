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

#include "imagemediawriter.hpp"

#include "mediadatabox.hpp"
#include "mediatypedefs.hpp"
#include "parserfactory.hpp"
#include "parserinterface.hpp"

#include <memory>

ImageMediaWriter::ImageMediaWriter(const std::string& fileName,
                                   const std::string& codeType) :
    mFilePath(fileName),
    mCodeType(codeType)
{
}

MediaDataBox ImageMediaWriter::writeMedia()
{
    // Create bitstream parser for this code type
    MediaType mediaType = MediaTypeTool::getMediaTypeByCodeType(mCodeType, mFilePath);
    std::unique_ptr<ParserInterface> mediaParser = ParserFactory::getParser(mediaType);

    if (!mediaParser->openFile(mFilePath.c_str()))
    {
        throw std::runtime_error("Not able to open " + MediaTypeTool::getBitStreamTypeName(mediaType) +
                                 " bit stream file '" + mFilePath + "'");
    }

    MediaDataBox mdat;

    ParserInterface::AccessUnit* accessUnit = new ParserInterface::AccessUnit { };

    const bool hasNalUnits = (accessUnit->mNalUnits.size() > 0) ? true : false;
    bool hasMoreImages = (mediaParser->parseNextAU(*accessUnit));
    const bool hasSpsNalUnits = (accessUnit->mSpsNalUnits.size() > 0) ? true : false;
    const bool hasPpsNalUnits = (accessUnit->mPpsNalUnits.size() > 0) ? true : false;

    const bool hasMediaData = (hasMoreImages || hasNalUnits || hasSpsNalUnits || hasPpsNalUnits);

    // Handle AVC & HEVC media data
    if (hasMediaData
        && ((mediaType == MediaType::AVC) || (mediaType == MediaType::HEVC) || (mediaType == MediaType::LHEVC)))
    {
        // Decoder configuration in the beginning is discarded
        while (hasMoreImages)
        {
            if (accessUnit == nullptr)
            {
                accessUnit = new ParserInterface::AccessUnit { };
                hasMoreImages = mediaParser->parseNextAU(*accessUnit);
            }
            if (hasMoreImages)
            {
                mdat.addNalData(accessUnit->mNalUnits);
            }
            delete accessUnit;
            accessUnit = nullptr;
        }
    }

    return mdat;
}
