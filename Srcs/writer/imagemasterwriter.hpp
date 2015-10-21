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

#ifndef IMAGEMASTERWRITER_HPP
#define IMAGEMASTERWRITER_HPP

#include "rootmetaimagewriter.hpp"
#include "isomediafile.hpp"

/**
 * @brief MetaBox writer for writing master images.
 */
class ImageMasterWriter : public RootMetaImageWriter
{
public:
    /**
    * @brief ImageMasterWriter constructor
    * @param config      Configuration of this writer
    * @param handlerType Handler type
    * @param contextId   Context ID of this writer
    */
    ImageMasterWriter(const IsoMediaFile::Master& config, const std::vector<std::string>& handlerType, uint32_t contextId);
    virtual ~ImageMasterWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    virtual void write(MetaBox* metaBox);

private:
    IsoMediaFile::Master mConfig; ///< Input configuration of the writer
};

#endif /* end of include guard: IMAGEMASTERWRITER_HPP */

