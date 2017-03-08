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

#include "filewriter.hpp"
#include "auxiliaryimagewriter.hpp"
#include "buildinfo.hpp"
#include "deriveditemmediawriter.hpp"
#include "entitygroupwriter.hpp"
#include "ftypwriter.hpp"
#include "idspace.hpp"
#include "imagemasterwriter.hpp"
#include "imagemediawriter.hpp"
#include "imagethumbswriter.hpp"
#include "layerimagewriter.hpp"
#include "log.hpp"
#include "metadatamediawriter.hpp"
#include "metadatawriter.hpp"
#include "metaderivedimagewriter.hpp"
#include "metapropertywriter.hpp"
#include "metawriter.hpp"
#include "moovwriter.hpp"
#include "moviebox.hpp"
#include "primaryitemwriter.hpp"
#include "trackaltrepwriter.hpp"
#include "trackmasterwriter.hpp"
#include "trackthumbswriter.hpp"
#include "trackwriter.hpp"
#include "writerconstants.hpp"

#include <algorithm>
#include <fstream>

void FileWriter::writeFile(const IsoMediaFile::Configuration& configuration)
{
    IsoMediaFile::Configuration config = configuration;

    createWriters(config);

    // Write root level boxes with an empty offsets map to find out box locations.
    OffsetMap offsets;
    BitStream ftyp = writeFtyp(config.general.brands);
    writeMeta(offsets);
    writeMoov(offsets);
    std::vector<MediaDataBox> mdatBoxes = writeMdat();

    // Calculate box offsets in file from box sizes.
    // This can be done as long as the write order stays same (ftyp, meta, moov, mdat)
    offsets = makeOffsetMap(mBoxSizes);

    // Rewrite needed boxes with correct offsets
    BitStream meta = writeMeta(offsets);
    BitStream moov = writeMoov(offsets);

    mdatBoxes.push_back(createVersionMdat());

    writeStream(config.general.output_file, ftyp, meta, moov, mdatBoxes);
}

void FileWriter::createWriters(IsoMediaFile::Configuration& config)
{
    // Fill an intermediate map of altr uniq_bsids
    for (const auto& idxs : config.egroups)
    {
        for (const auto& id : idxs.idxs_lists)
        {
            mAltrUniqBsids[mAlterId].insert(id.uniq_bsid);
        }
        ++mAlterId;
    }

    for (auto& content : config.content)
    {
        // If encapsulation type is track, then start track writers
        if (content.master.encp_type == "trak")
        {
            mHasTrack = true;
            createTrackWriters(content);
        }
        // If encapsulation type is track, then start track writers
        else if (content.master.encp_type == "meta")
        {
            mHasImage = true;
            createImageWriters(config, content);
        }
    }

    // Give first found meta content (if any) context as a fallback option to the primary item writer
    for (const auto& content : config.content)
    {
        if (content.master.encp_type == "meta")
        {
            mMetaWriterMap.insert(std::make_pair(Context::getValue(),
                std::unique_ptr<PrimaryItemWriter>(new PrimaryItemWriter(config.general.prim_refr,
                    config.general.prim_indx, content.master.contextId))));
            break;
        }
    }

    createEntityGroupWriters(config.egroups);
}

void FileWriter::createEntityGroupWriters(const std::vector<IsoMediaFile::Egroup>& egroups)
{
    for (const auto& egroup : egroups)
    {
        mMetaWriterMap.insert(std::make_pair(Context::getValue(),
            std::unique_ptr<EntityGroupWriter>(new EntityGroupWriter(egroup))));
    }
}

void FileWriter::createTrackWriters(const IsoMediaFile::Content& config)
{
    // This track writer is for the master
    // There must be at least one master per content. If there are thumb tracks
    // present then the master must be marked as an alternative.
    const bool isAlter = (config.thumbs.size() || config.master.make_vide) && config.master.write_alternates;
    const ContextId masterId = createMasterTrackWriter(config.master, isAlter);

    // A track writer for each thumbs
    for (const auto& thumbs : config.thumbs)
    {
        createThumbTrackWriter(thumbs, masterId);
    }

    // If a vide track is also requested make a track writer for it
    if (config.master.make_vide == true)
    {
        createVideTrackWriter(config.master, masterId);
    }
}

void FileWriter::createImageWriters(const IsoMediaFile::Configuration& config, IsoMediaFile::Content& content)
{
    const ContextId masterId = createMasterImageWriter(content.master);
    content.master.contextId = masterId;

    for (const auto& thumbs : content.thumbs)
    {
        createThumbImageWriter(thumbs, masterId);
    }

    // Create metadata (Exif, xml) writers
    for (const auto& thumbs : content.metadata)
    {
        createMetadataWriters(thumbs, masterId);
    }

    for (const auto& auxiliary : content.auxiliary)
    {
        createAuxiliaryImageWriter(auxiliary, masterId);
    }

    for (const auto& layers : content.layers)
    {
        createLayerImageWriter(layers, masterId);
    }

    content.property.contextId = Context::getValue();
    content.derived.contextId = Context::getValue();

    createPropertyWriters(config, content);
    createDerivedImageWriters(content.derived, masterId);
}

ContextId FileWriter::createMasterTrackWriter(const IsoMediaFile::Master& config, const bool isAlter)
{
    // Obtain a unique identifier for this track. This identifier is used both
    // as a context-id as well as a track-id for this track.
    const ContextId contextId = Context::getValue();

    bool inAlterList = false;
    ContextId alterId = 0;
    for (const auto& idxlist : mAltrUniqBsids)
    {
        if (idxlist.second.count(config.uniq_bsid))
        {
            alterId = idxlist.first;
            mAlterLinkerMap[alterId].insert(contextId);
            inAlterList = true;
        }
    }

    if (isAlter == true && inAlterList == false)
    {
        mAlterLinkerMap[mAlterId].insert(contextId);
        alterId = mAlterId;
        ++mAlterId;
    }

    // Instantiate a track writer and a media writer for this track
    std::unique_ptr<TrackWriter> trackWriter(new TrackMasterWriter(contextId, config));
    std::unique_ptr<MediaWriter> mediaWriter(new ImageMediaWriter(config.file_path, config.code_type));

    // Initialization of this track writer
    if (isAlter == true || inAlterList == true)
    {
        trackWriter->setTrackAsAlter(alterId);
    }

    // Store the track writer and media writer in their respective maps
    mTrackWriterMap.insert(std::make_pair(contextId, move(trackWriter)));
    mMediaWriterMap.insert(std::make_pair(contextId, move(mediaWriter)));
    mMediaLinkerMap[contextId].insert(contextId);

    // Return the contextId assigned for this track. This is required to link
    // this track to all tracks that reference this track.
    return contextId;
}

ContextId FileWriter::addToAlterLinkerMap(const ContextId masterId, const ContextId contextId)
{
    ContextId alterId = 0;
    for (auto& idxlist : mAlterLinkerMap)
    {
        if (idxlist.second.count(masterId))
        {
            idxlist.second.insert(contextId);
            alterId = idxlist.first;
        }
    }
    return alterId;
}

void FileWriter::createVideTrackWriter(const IsoMediaFile::Master& config, const ContextId masterId)
{
    TrackAltrepWriter::Altrep altrep;

    altrep.file_path = config.file_path;
    altrep.hdlr_type = VIDE_HANDLER;
    altrep.code_type = config.code_type;
    altrep.encp_type = config.encp_type;
    altrep.disp_xdim = config.disp_xdim;
    altrep.disp_ydim = config.disp_ydim;
    altrep.disp_rate = config.disp_rate;
    altrep.tick_rate = config.tick_rate;
    altrep.edit_list = config.edit_list;

    // Obtain a unique identifier for this track. This identifier is used both
    // as a context-id as well as a track-id for this track.
    const ContextId contextId = Context::getValue();
    const ContextId alterId = addToAlterLinkerMap(masterId, contextId);

    // Instantiate a track writer and a media writer for this track
    std::unique_ptr<TrackWriter> trackWriter(new TrackAltrepWriter(contextId, altrep));
    trackWriter->setTrackAsAlter(alterId);

    // Store the track writer and media writer in their respective maps
    mTrackWriterMap.insert(std::make_pair(contextId, move(trackWriter)));
    mMediaLinkerMap[masterId].insert(contextId);
}

ContextId FileWriter::createMasterImageWriter(const IsoMediaFile::Master& config)
{
    const ContextId contextId = Context::getValue();
    const std::vector<std::string> type = { config.hdlr_type, config.code_type, config.encp_type };

    std::unique_ptr<MetaWriter> imageWriter(new ImageMasterWriter(config, type, contextId));
    std::unique_ptr<MediaWriter> mediaWriter(new ImageMediaWriter(config.file_path, config.code_type));

    // Store the track writer and media writer in their respective maps
    mMetaWriterMap.insert(std::make_pair(contextId, move(imageWriter)));
    mMediaWriterMap.insert(std::make_pair(contextId, move(mediaWriter)));
    mMediaLinkerMap[contextId].insert(contextId);

    return contextId;
}

void FileWriter::createThumbTrackWriter(const IsoMediaFile::Thumbs& config, const ContextId masterId)
{
    const ContextId contextId = Context::getValue();
    const ContextId alterId = addToAlterLinkerMap(masterId, contextId);

    std::unique_ptr<TrackWriter> trackWriter(new TrackThumbsWriter(contextId, config, masterId));
    std::unique_ptr<MediaWriter> mediaWriter(new ImageMediaWriter(config.file_path, config.code_type));

    trackWriter->setPreviewTrack(true);
    trackWriter->setTrackAsAlter(alterId);

    // Store the track writer and media writer in their respective maps
    mTrackWriterMap.insert(std::make_pair(contextId, move(trackWriter)));
    mMediaWriterMap.insert(std::make_pair(contextId, move(mediaWriter)));
    mMediaLinkerMap[contextId].insert(contextId);
}

void FileWriter::createThumbImageWriter(const IsoMediaFile::Thumbs& config, const ContextId masterId)
{
    const ContextId contextId = Context::getValue();
    const std::vector<std::string> type = { "pict", config.code_type, "meta" };

    std::unique_ptr<MetaWriter> imageWriter(new ImageThumbsWriter(config, type, contextId));
    std::unique_ptr<MediaWriter> mediaWriter(new ImageMediaWriter(config.file_path, config.code_type));
    imageWriter->linkMasterDataStore(masterId);

    mMetaWriterMap.insert(std::make_pair(contextId, move(imageWriter)));
    mMediaWriterMap.insert(std::make_pair(contextId, move(mediaWriter)));
    mMediaLinkerMap[contextId].insert(contextId);
}

void FileWriter::createMetadataWriters(const IsoMediaFile::Metadata& config, const ContextId masterId)
{
    std::unique_ptr<MetaWriter> metadataWriter(new MetadataWriter(config));
    std::unique_ptr<MediaWriter> mediaWriter(new MetadataMediaWriter(config.file_path, config.hdlr_type));
    metadataWriter->linkMasterDataStore(masterId);

    // MetadataWriter does not really have a context, it is used as a map key only
    const ContextId contextId = Context::getValue();
    mMetaWriterMap.insert(std::make_pair(contextId, move(metadataWriter)));
    mMediaWriterMap.insert(std::make_pair(contextId, move(mediaWriter)));
    mMediaLinkerMap[contextId].insert(contextId);
}

void FileWriter::createPropertyWriters(const IsoMediaFile::Configuration& config, const IsoMediaFile::Content& content)
{
    std::unique_ptr<MetaWriter> propertyWriter(new MetaPropertyWriter(config, content));
    mMetaWriterMap.insert(std::make_pair(content.property.contextId, move(propertyWriter)));
}

void FileWriter::createDerivedImageWriters(const IsoMediaFile::Derived& config, const ContextId masterId)
{
    const ContextId derivedContextId = config.contextId;
    std::unique_ptr<MediaWriter> mediaWriter(new DerivedItemMediaWriter());
    DerivedItemMediaWriter* derivedItemMediaWriter = dynamic_cast<DerivedItemMediaWriter*>(mediaWriter.get());

    std::unique_ptr<MetaWriter> derivedImageWriter(new MetaDerivedImageWriter(config, derivedItemMediaWriter));
    derivedImageWriter->linkMasterDataStore(masterId);

    mMetaWriterMap.insert(std::make_pair(derivedContextId, move(derivedImageWriter)));
    mMediaWriterMap.insert(std::make_pair(derivedContextId, move(mediaWriter)));
    mMediaLinkerMap[derivedContextId].insert(derivedContextId);
}

void FileWriter::createAuxiliaryImageWriter(const IsoMediaFile::Auxiliary& config, const ContextId masterId)
{
    const ContextId contextId = Context::getValue();

    std::unique_ptr<MediaWriter> mediaWriter(new ImageMediaWriter(config.file_path, config.code_type));

    std::unique_ptr<MetaWriter> auxiliaryImageWriter(new AuxiliaryImageWriter(config, contextId));
    auxiliaryImageWriter->linkMasterDataStore(masterId);

    mMetaWriterMap.insert(std::make_pair(contextId, move(auxiliaryImageWriter)));
    mMediaWriterMap.insert(std::make_pair(contextId, move(mediaWriter)));
    mMediaLinkerMap[contextId].insert(contextId);
}

void FileWriter::createLayerImageWriter(const IsoMediaFile::Layer& config, const ContextId masterId)
{
    const ContextId contextId = Context::getValue();

    if (config.file_path == "")
    {
        // Use master context IDs 'mdat'
        mMediaLinkerMap[masterId].insert(contextId);
    }
    else
    {
        std::unique_ptr<MediaWriter> mediaWriter(new ImageMediaWriter(config.file_path, config.code_type));
        mMediaWriterMap.insert(std::make_pair(contextId, move(mediaWriter)));
        mMediaLinkerMap[contextId].insert(contextId);
    }

    std::unique_ptr<MetaWriter> layerImageWriter(new LayerImageWriter(config, contextId));
    layerImageWriter->linkMasterDataStore(masterId);

    mMetaWriterMap.insert(std::make_pair(contextId, move(layerImageWriter)));
}

BitStream FileWriter::writeFtyp(const IsoMediaFile::Brands& config)
{
    BitStream output;
    FileTypeBox ftypBox;
    FtypWriter ftypWriter(config);

    ftypBox = ftypWriter.writeBox(output);
    mBoxSizes.insert(std::make_pair(OffsetMapKeys::FTYP, ftypBox.getSize()));
    return output;
}

BitStream FileWriter::writeMeta(const OffsetMap& offsets)
{
    BitStream output;
    MetaBoxWriter writer;
    for (auto& imageWriter : mMetaWriterMap)
    {
        const Offset offset = getMdatOffset(offsets, imageWriter.first);
        writer.writeToMetaBox(imageWriter.second.get(), offset);
    }

    if (mHasImage)
    {
        writer.serialize(output);
        mBoxSizes.insert(std::make_pair(OffsetMapKeys::META, output.getSize()));
    }

    return output;
}

FileWriter::Offset FileWriter::getMdatOffset(const OffsetMap& offsets, const ContextId contextId) const
{
    if (offsets.size() == 0)
    {
        return 0;
    }

    for (const auto& mapItr : mMediaLinkerMap)
    {
        if (mapItr.second.count(contextId))
        {
            return offsets.at(mapItr.first);
        }
    }
    return 0;
}

BitStream FileWriter::writeMoov(const OffsetMap& offsets)
{
    MoovWriter moovWriter;
    BitStream output;

    for (auto& trackWriter : mTrackWriterMap)
    {
        const ContextId trackId = trackWriter.first;
        const Offset offset = getMdatOffset(offsets, trackId);
        moovWriter.addTrack(trackWriter.second->writeTrack(), offset);
    }

    if (mHasTrack == true)
    {
        moovWriter.serialize(output);
        mBoxSizes.insert(std::make_pair(OffsetMapKeys::MOOV, output.getSize()));
    }

    return output;
}


std::vector<MediaDataBox> FileWriter::writeMdat()
{
    std::vector<MediaDataBox> boxes;

    for (auto& mediaWriter : mMediaWriterMap)
    {
        MediaDataBox newMdat = mediaWriter.second->writeMedia();

        // Do not write unnecessary empty mdats.
        if (newMdat.getSize() > 8)
        {
            boxes.push_back(newMdat);
            mBoxSizes.insert(std::make_pair(mediaWriter.first, boxes.back().getSize()));
        }
        else
        {
            mBoxSizes.insert(std::make_pair(mediaWriter.first, 0));
        }
    }

    return boxes;
}

FileWriter::OffsetMap FileWriter::makeOffsetMap(const SizesMap& boxSizes) const
{
    OffsetMap offsets;

    std::uint32_t offset = 0;
    for (const auto& entry : boxSizes)
    {
        offsets.insert(std::make_pair(entry.first, offset));
        offset += entry.second;
    }

    return offsets;
}

void FileWriter::writeStream(const std::string& filePath, BitStream& ftyp, BitStream& meta, BitStream& moov,
    std::vector<MediaDataBox>& mdats) const
{
    std::ofstream file;

    file.open(filePath, std::ofstream::out | std::ofstream::binary);
    if (not file.is_open())
    {
        throw std::runtime_error("Unable to create output file '" + filePath + "'.");
    }

    writeBitstream(ftyp, file);

    if (mHasImage)
    {
        writeBitstream(meta, file);
    }
    if (mHasTrack)
    {
        writeBitstream(moov, file);
    }
    for (const auto& mdat : mdats)
    {
        file.write(reinterpret_cast<const char*>(mdat.getData().data()), mdat.getData().size());
    }

    file.close();
}

void FileWriter::writeBitstream(BitStream& input, std::ofstream& output) const
{
    const std::vector<uint8_t>& data = input.getStorage();
    output.write(reinterpret_cast<const char*>(data.data()), data.size());
}

MediaDataBox FileWriter::createVersionMdat() const
{
    MediaDataBox mdat;
    const std::string version = std::string("NHW_") + BuildInfo::CompatibilityVersion;
    const std::vector<uint8_t> data(version.cbegin(), version.cend());
    mdat.addData(data);

    return mdat;
}
