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

#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP

#include "context.hpp"
#include "isomediafile.hpp"
#include "mediadatabox.hpp"
#include "mediawriter.hpp"
#include "metaboxwriter.hpp"
#include "metawriter.hpp"
#include "trackwriter.hpp"

#include <map>
#include <memory>
#include <set>

class ImageWriter;
class MovieBox;

/**
 * Write a HEIF file based on the input configuration.
 * @see IsoMediaFile::Configuration for details about writer configuration.
 */
class FileWriter
{
public:
    /**
     * @brief Write the HEIF file.
     * @param configuration Writer input configuration */
    void writeFile(const IsoMediaFile::Configuration& configuration);

private:
    typedef std::uint32_t Offset; ///< Offset in bytes
    typedef std::uint32_t Size;   ///< Data size in bytes
    typedef std::map<ContextId, Offset> OffsetMap; ///< Convenience type for context offsets
    typedef std::map<ContextId, Size> SizesMap;    ///< Convenience type for context sizes
    typedef std::map<std::uint32_t, std::set<std::uint32_t>> IdSetMap; ///< Convenience type for mapping IDs to sets of different IDs

    ContextId mAlterId = 1000; ///< Alternate group identifier

    /** Define fixed keys used in SizesMap for certain root level boxes. In addition to these, to map will be added
     *  entries whose key is their context ID (should be >1000 so there is no chance of keys conflicting). */
    enum OffsetMapKeys
    {
        FTYP = 0, ///< File Type Box
        META = 1, ///< Meta Box (root level)
        MOOV = 2  ///< Movie Box
    };

    SizesMap mBoxSizes; ///< A map that contains sizes of the root level boxes

    bool mHasImage = false; ///< True if meta based image contents have to be written.
    bool mHasTrack = false; ///< True if trak based image contents have to be written.

    /**
     * Intermediate mapping from alternate group IDs to sets of uniq_bsids. uniq_bsid values are read from the writer
     * input configuration. After context ID of each uniq_bsid is known this map is used to create mAlterLinkerMap.
     * Map keys are alternate group IDs.
     * Map values are sets of uniq_bsids belonging to this group. */
    IdSetMap mAltrUniqBsids;

    /**
     * Mapping from alternate group IDs to sets of context IDs. This map is filled from data in mAltrUniqBsids after
     * context IDs for each uniq_bsid are known.
     * Map keys are alternate group IDs.
     * Map values are sets of context IDs belonging to this group. */
    IdSetMap mAlterLinkerMap;

    /**
     * Mappings from a MediaDataBox to several writers.
     * Map keys are MediaDataBox context IDs.
     * Map values are writer context IDs of TrackWriter writing TrackBoxes related to the MediaDataBox */
    IdSetMap mMediaLinkerMap;

    std::map<ContextId, std::unique_ptr<MediaWriter>> mMediaWriterMap; ///< MediaDataBox writers
    std::map<ContextId, std::unique_ptr<MetaWriter>>  mMetaWriterMap;  ///< MetaBox writers
    std::map<ContextId, std::unique_ptr<TrackWriter>> mTrackWriterMap; ///< TrackBox writers

    /**
     * Create different writers based on the input configuration.
     * @param config Input configuration. */
    void createWriters(IsoMediaFile::Configuration& config);

    /**
     * Create MetaBox writers for a content using meta encapsulation.
     * @param config  Complete writer input configuration.
     * @param content Configuration of this content. */
    void createImageWriters(const IsoMediaFile::Configuration& config, IsoMediaFile::Content& content);

    /**
     * Create a MetaBox writer and a corresponding MediaWriter for writing master images.
     * @param config Input configuration.
     * @return Context ID of created writer. */
    ContextId createMasterImageWriter(const IsoMediaFile::Master& config);

    /**
     * Create a MetaWriter and a corresponding MediaWriter for writing thumbnail images.
     * @param config   Input configuration.
     * @param masterId Context ID of the master context of this content. */
    void createThumbImageWriter(const IsoMediaFile::Thumbs& config, ContextId masterId);

    /**
     * Create a MetaWriter and a corresponding MediaWriter for writing auxiliary images.
     * @param config   Input configuration.
     * @param masterId Context ID of the master context of this content. */
    void createAuxiliaryImageWriter(const IsoMediaFile::Auxiliary& config, ContextId masterId);

    /**
     * Create a MetaWriter and a corresponding MediaWriter for writing derived images.
     * @param config   Input configuration.
     * @param masterId Context ID of the master context of this content. */
    void createDerivedImageWriters(const IsoMediaFile::Derived& config, ContextId masterId);

    /**
     * Create MetaWriter of type EntityGroupWriter
     * @param egroups Input configuration. */
    void createEntityGroupWriters(const std::vector<IsoMediaFile::Egroup>& egroups);

    /**
     * Create a MetaWriter and a corresponding MediaWriter for writing Exif and XML metadata.
     * @param config   Input configuration.
     * @param masterId Context ID of the master context of this content. */
    void createMetadataWriters(const IsoMediaFile::Metadata& config, ContextId masterId);

    /**
     * Create a MetaWriter for writing Properties.
     * @param config  Complete writer input configuration.
     * @param content Configuration of this content. */
    void createPropertyWriters(const IsoMediaFile::Configuration& config, const IsoMediaFile::Content& content);

    /**
     * Create TrackBox writers for a content using trak encapsulation.
     * @param config Configuration of this content. */
    void createTrackWriters(const IsoMediaFile::Content& config);

    /**
     * Create TrackBox writer for writing master images.
     * @param config  Input configuration.
     * @param isAlter True if the resulting master TrackBox is an alternate to an another TrackBox.
     * @return Context ID of created writer. */
    ContextId createMasterTrackWriter(const IsoMediaFile::Master& config, bool isAlter);

    /**
     * Create a TrackBox writer for writing thumbnail images.
     * @param config   Input configuration.
     * @param masterId Context ID of the master context of this content. */
    void createThumbTrackWriter(const IsoMediaFile::Thumbs& config, ContextId masterId);

    /**
     * Create a TrackBox writer for writing 'vide' video track as an alternative to the master TrackBox of this content.
     * @param config   Input configuration.
     * @param masterId Context ID of the master context of this content. */
    void createVideTrackWriter(const IsoMediaFile::Master& config, ContextId masterId);

    /**
     * Create a MetaBox writer for writing multi-layer 'lhv1' image items.
     * @param config   Input configuration.
     * @param masterId Context ID of the master context of this content. */
    void createLayerImageWriter(const IsoMediaFile::Layer& config, const ContextId masterId);

    /**
     * Create and serialize FileTypeBox
     * @details Size of the serialized FileTypeBox is inserted to mBoxSizes.
     * @param config Input configuration.
     * @return Serialized FileTypeBox BitStream. */
    BitStream writeFtyp(const IsoMediaFile::Brands& config);

    /**
     * Create and serialize the root-level MetaBox
     * @details Size of the serialized MetaBox is inserted to mBoxSizes.
     * @param offsets Root-level box (ftyp, meta, moov, mdat) offsets. Context IDs are used as mdat keys. Other keys
     *                 are from OffsetMapKeys.
     * @return Serialized MetaBox BitStream. In case the input configuration has no meta-type contents nothing is
     *         written to the bitstream. */
    BitStream writeMeta(const OffsetMap& offsets);

    /**
     * Create and serialize the MovieBox
     * @details Size of the serialized MovieBox is inserted to mBoxSizes.
     * @param offsets Root-level box (ftyp, meta, moov, mdat) offsets. Context IDs are used as mdat keys. Other keys
     *                 are from OffsetMapKeys.
     * @return Serialized MovieBox BitStream. In case the input configuration has no trak encapsulation contents
     *         nothing is written to the bitstream. */
    BitStream writeMoov(const OffsetMap& offsets);

    /**
     * Call MediaWriters to generate MediaDataBoxes
     * @details Sizes of serialized MediaDataBoxes are inserted to mBoxSizes.
     * @return Filled MediaDataBoxes */
    std::vector<MediaDataBox> writeMdat();

    /**
     * Generate box start offset map based on box sizes.
     * @details This assumes that boxes are written to file in order of OffsetMapKeys, and ascending context ID order for mdats.
     * @param boxSizes Root-level box (ftyp, meta, moov, mdat) sizes. Context IDs are used as mdat keys. Other keys
     *                 are from OffsetMapKeys.
     * @return Box start offsets in the file. */
    OffsetMap makeOffsetMap(const SizesMap& boxSizes) const;

    /**
     * Write HEIF bitstream to a file
     * @param filePath File name to write
     * @param ftyp     FileTypeBox bitstream
     * @param meta     MetaBox bitstream
     * @param moov     MovieBox bitstream
     * @param mdats    MediaDataBoxes */
    void writeStream(const std::string& filePath, BitStream& ftyp, BitStream& meta, BitStream& moov,
        std::vector<MediaDataBox>& mdats) const;

    /**
     * Write BitStream content to std::ofstream
     * @param [in]  input  Input BitStream
     * @param [out] output Output stream */
    void writeBitstream(BitStream& input, std::ofstream& output) const;

    /**
     * @brief Get file offset of the MediaDataBox referred from the context ID.
     * @details Several writers with different context IDs might be referring to the same MediaDataBox.
     *          This method enables easily finding the correct offset.
     * @param offsets Box offset map
     * @param contextId Context ID of a writer.
     * @return MediaDataBox offset in the file in bytes. Zero if parameter offsets map is empty or
     *         context ID was not found (the writer has no related 'mdat'). */
    Offset getMdatOffset(const OffsetMap& offsets, ContextId contextId) const;

    /**
     * @brief Insert contextId to the mAlterLinkerMap entry where masterId exists.
     * @details If masterId is not found, no new entry to map is added.
     * @param masterId  The ContextId value to search in mAlterLinkerMap.
     * @param contextId The ContextId to insert.
     * @return The alternative group ID of the group from where the masterId was found (the key of corresponding
     *         mAlterLinkerMap entry). 0 if masterId was not found. */
    ContextId addToAlterLinkerMap(ContextId masterId, ContextId contextId);

    /**
     * @return A MediaDataBox which includes HEIF writer compatibility version string.
     */
    MediaDataBox createVersionMdat() const;
};

#endif /* end of include guard: FILEWRITER_HPP */
