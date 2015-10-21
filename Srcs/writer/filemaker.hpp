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

#ifndef FILEMAKER_HPP
#define FILEMAKER_HPP

#include "isomediafile.hpp"

/**
 * @brief This is an implementation of the generic interface.
 * @details FileMaker implements the interface specified in IsoMediaFile.
 */
class FileMaker : public IsoMediaFile
{
public:
    FileMaker() = default;
    virtual ~FileMaker() = default;

    /**
     * @brief Implements the interface method to verify all inputs and make the output file.
     * @details This method handles the writing of the output file given all the inputs.
     *          Before the writing can commence all configurations must be verified.
     *          Then proceed to load the configs into the file writer class which
     *          handles all the writing process.
     * @param [in] configuration The writer configuration structure.
     */
    void makeFile(const Configuration& configuration);
};

#endif /* end of include guard: FILEMAKER_HPP */
