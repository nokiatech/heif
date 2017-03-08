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

#ifndef ENTITYGROUPWRITER_HPP
#define ENTITYGROUPWRITER_HPP

#include "isomediafile.hpp"
#include "metawriter.hpp"
#include <map>

class MetaBox;

/**
 * @brief MetaBox writer for writing 'altr' type EntityToGroupBox
 * @details This writer enables mapping items as alternatives for tracks or other items.
 */
class EntityGroupWriter : public MetaWriter
{
public:
    EntityGroupWriter(const IsoMediaFile::Egroup& config);
    virtual ~EntityGroupWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    virtual void write(MetaBox* metaBox);

private:
    typedef std::uint32_t EntityId; ///< Track or item ID
    typedef std::uint32_t UniqBsid; ///< uniq_bsid value in writer input configuration
    typedef std::uint32_t ItemIndex; ///< 1-based item index
    typedef std::map<std::pair<UniqBsid, ItemIndex>, EntityId> IdPairToEntityIdMap;

    /**
     * @brief Create map from writer input configuration uniq_bsid and item indexes to EntityId.
     * @return Filled IdPairToEntityIdMap
     * @todo Need for this method could/should be eliminated by providing equivalent information to all writers.
     */
    IdPairToEntityIdMap makeEntityMap() const;

    IsoMediaFile::Egroup mConfig; ///< Input configuration of the writer
};

#endif /* end of include guard: ENTITYGROUPWRITER_HPP */
