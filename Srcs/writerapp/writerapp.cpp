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

#include "isomediafile.hpp"
#include "writerapp.hpp"
#include "writercfg.hpp"

#include <memory>
#include <sstream>

void WriterApp::processConfiguration(const std::string& fileName)
{
    IsoMediaFile::Configuration configuration = readConfigFile(fileName);
    std::unique_ptr<IsoMediaFile> isoMediaFile = IsoMediaFile::newInstance();
    isoMediaFile->makeFile(configuration);
}

IsoMediaFile::Configuration WriterApp::readConfigFile(const std::string& fileName) const
{
    WriterConfig mConfig;
    return mConfig.readJson(fileName.c_str());
}
