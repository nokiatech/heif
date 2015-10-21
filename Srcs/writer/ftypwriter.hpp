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

#ifndef FTYPWRITER_HPP
#define FTYPWRITER_HPP

#include "bitstream.hpp"
#include "filetypebox.hpp"
#include "isomediafile.hpp"

#include <memory>

/**
 * @brief FileType Box writer for writing 'ftyp' box.
 * @details 'ftyp' box lists the major and compatible brands list.
 */
class FtypWriter
{
public:
    FtypWriter(const IsoMediaFile::Brands& brandConf);
    virtual ~FtypWriter() = default;

    /**
     * @brief Create 'ftyp' Box data as a bitstream.
     * @param [in] bstrm 'ftyp' box bitstream
     * @returns File Type Box data structure
     */
    FileTypeBox& writeBox(BitStream& bstrm);

private:
    IsoMediaFile::Brands mBrandConf; /// major and compatible brands configuration

    BitStream mFtypStr; /// 'ftyp' bow bitstream
    FileTypeBox mFtypBox; /// 'ftyp' box data structure
};

#endif /* end of include guard: FTYPWRITER_HPP */

