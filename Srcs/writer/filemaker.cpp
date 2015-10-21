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

#include "filemaker.hpp"
#include "context.hpp"
#include "filewriter.hpp"
#include "idspace.hpp"
#include "services.hpp"

/**
 * @details This method handles the creation of an instance of a Filemaker class.
 */
std::unique_ptr<IsoMediaFile> IsoMediaFile::newInstance()
{
    return std::unique_ptr<IsoMediaFile>(new FileMaker());
}


/**
 * @todo Do a verification of the input configuration before writeFile is called.
 */
void FileMaker::makeFile(const Configuration& configuration)
{
    // Clear static entities to make subsequent calls possible during unit tests
    DataServe::clear();
    IdSpace::reset();
    Context::reset();

    FileWriter fileWriter;
    fileWriter.writeFile(configuration);
}
