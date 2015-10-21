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

#ifndef WRITERAPP_HPP
#define WRITERAPP_HPP

#include "isomediafile.hpp"

#include <cstdlib>
#include <memory>
#include <string>

/** Create an HEVC file from configuration data */
class WriterApp
{
public:
    /** Process a writer application configuration file */
    void processConfiguration(const std::string& fileName);

private:
    /** Read configuration file */
    IsoMediaFile::Configuration readConfigFile(const std::string& fileName) const;
};

#endif

