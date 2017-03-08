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

#ifndef METAPROPERTYWRITER_HPP
#define METAPROPERTYWRITER_HPP

#include "isomediafile.hpp"
#include "metawriter.hpp"

class MetaBox;

/**
 * @brief MetaBox writer for writing properties, and adding references to items.
 * @details Currently only 'imir', 'irot', 'rloc' and 'clap' are supported.
 * @todo Add support for other properties.
 */
class MetaPropertyWriter : public MetaWriter
{
public:
    /**
     * @brief MetaPropertyWriter constructor
     * @param config Complete writer configuration
     * @param contentConfig Configuration of this writer
     */
    MetaPropertyWriter(const IsoMediaFile::Configuration& config, const IsoMediaFile::Content& contentConfig);
    virtual ~MetaPropertyWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    virtual void write(MetaBox* metaBox);

private:
    /**
     * @brief Write imir properties and references to MetaBox
     * @param [in,out] metaBox The MetaBox in which properties are added to.
     */
    void writeImir(MetaBox* metaBox) const;

    /**
     * @brief Write irot properties and references to MetaBox
     * @param [in,out] metaBox The MetaBox in which properties are added to.
     */
    void writeIrot(MetaBox* metaBox) const;

    /**
     * @brief Write rloc properties and references to MetaBox
     * @param [in,out] metaBox The MetaBox in which properties are added to.
     */
    void writeRloc(MetaBox* metaBox) const;

    /**
     * @brief Write clap properties and references to MetaBox
     * @param [in,out] metaBox The MetaBox in which properties are added to.
     */
    void writeClap(MetaBox* metaBox) const;

    /**
     * @brief Find item IDs by uniq_bsid and indexes
     * @param refsList Vector of uniq_bsids to reference images from
     * @param indexList 1-based index number of images to reference, one vector for each entry in refsList
     * @return Corresponding item IDs
     */
    std::vector<uint32_t> getReferenceItemIds(const IsoMediaFile::ReferenceList& refsList,
        const IsoMediaFile::IndexList& indexList) const;

    IsoMediaFile::Configuration mConfig; ///< Whole writer configuration, for looking for references
    IsoMediaFile::Content mContentConfig; ///< Input configuration of the writer
};

#endif /* end of include guard: METAPROPERTYWRITER_HPP */
