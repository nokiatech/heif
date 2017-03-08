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

#ifndef AUXILIARYIMAGEWRITER_HPP
#define AUXILIARYIMAGEWRITER_HPP

#include "isomediafile.hpp"
#include "rootmetaimagewriter.hpp"
#include <map>
#include <memory>
#include <set>
#include <string>

class Metabox;

/**
 * @brief MetaBox writer for writing auxiliary images and corresponding references.
 */
class AuxiliaryImageWriter : public RootMetaImageWriter
{
public:
    /**
    * @brief AuxiliaryImageWriter constructor
    * @param config    Configuration of this writer
    * @param contextId Context ID of this writer
    */
    AuxiliaryImageWriter(const IsoMediaFile::Auxiliary& config, std::uint32_t contextId);
    virtual ~AuxiliaryImageWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    virtual void write(MetaBox* metaBox);

private:
    IsoMediaFile::Auxiliary mConfig; ///< Input configuration of the writer

    typedef std::uint32_t ItemId;
    typedef std::uint32_t UniqBsid;
    typedef std::vector<ItemId> ItemIdVector;
    typedef std::set<ItemId> ItemSet;
    typedef std::map<UniqBsid, ItemIdVector> ReferenceToItemIdMap;

    /**
     * @return Mapping from uniq_bsid context references and image indexes to item IDs
     * @todo Need for this method could be refactored away, if there was a global service for such mappings.
     */
    ReferenceToItemIdMap createReferenceToItemIdMap() const;

    /**
     * @brief Add entries to the Item Location Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     * @param          itemIds Item IDs to write
     */
    void ilocWrite(MetaBox* metaBox, const ItemSet& itemIds) const;

    /**
     * @brief Add entries to the Item Information Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     * @param          itemIds Item IDs to write
     */
    void iinfWrite(MetaBox* metaBox, const ItemSet& itemIds) const;

    /**
     * @brief Add entries to the Item Properties Box
     * @param [in,out] metaBox      Pointer to the MetaBox where to add data.
     * @param          itemIds      Item IDs to write
     */
    void iprpWrite(MetaBox* metaBox, const ItemSet& itemIds) const;

    /**
     * @brief Add 'auxl' type item references, based on writer input configuration
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     * @return Item IDs of auxiliary images which are being added.
     */
    ItemSet addItemReferences(MetaBox* metaBox) const;
};

#endif /* end of include guard: AUXILIARYIMAGEWRITER_HPP */

