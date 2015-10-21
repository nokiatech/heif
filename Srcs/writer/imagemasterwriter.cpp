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

#include "imagemasterwriter.hpp"
#include "services.hpp"
#include "writerconstants.hpp"

#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

ImageMasterWriter::ImageMasterWriter(const IsoMediaFile::Master& config, const std::vector<std::string>& handlerType,
    const std::uint32_t contextId) :
    RootMetaImageWriter(handlerType, contextId),
    mConfig(config)
{
}

void ImageMasterWriter::write(MetaBox* metaBox)
{
    RootMetaImageWriter::initWrite();
    storeValue("uniq_bsid", std::to_string(mConfig.uniq_bsid));
    storeValue("capsulation", META_ENCAPSULATION);
    RootMetaImageWriter::parseInputBitStream(mConfig.file_path);
    MetaWriter::hdlrWrite(metaBox);
    RootMetaImageWriter::ilocWrite(metaBox);
    RootMetaImageWriter::iinfWrite(metaBox);
    RootMetaImageWriter::iprpWrite(metaBox, mConfig.disp_xdim, mConfig.disp_ydim);
}
