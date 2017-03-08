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

#include "writercfg.hpp"
#include "json/json.h"
#include "log.hpp"

#include <fstream>

#define ANDROID_STOI_HACK
#include "androidhacks.hpp"

IsoMediaFile::Configuration WriterConfig::readJson(const std::string& filename)
{
    Json::Reader jsonReader;
    std::ifstream configFile(filename);
    if (!configFile.is_open())
    {
        throw std::runtime_error("Unable to open input file '" + filename + "'");
    }

    // Parse the configuration file
    Json::Value jsonValues;
    const bool parsingSuccess = jsonReader.parse(configFile, jsonValues, false);
    if (not parsingSuccess)
    {
        throw std::runtime_error("Failed to parse input configuration: " + jsonReader.getFormattedErrorMessages());
    }

    // Read JSON values into local structures
    IsoMediaFile::Configuration configuration;
    configuration.general = readGeneral(jsonValues["general"]);

    // The Contents
    Json::Value contents = jsonValues["content"];
    if (contents.isNull())
    {
        throw std::runtime_error("No content section in input configuration.");
    }
    for (const auto& content : contents)
    {
        configuration.content.push_back(readContent(content));
    }

    for (const auto& egroup : jsonValues["egroups"])
    {
        const auto egroups = readEgroups(egroup);
        configuration.egroups.insert(configuration.egroups.end(), egroups.cbegin(), egroups.cend());
    }

    confDump(configuration, jsonValues);
    return configuration;
}

void WriterConfig::readEditList(const std::string& filename, IsoMediaFile::Master& master) const
{
    Json::Reader jsonReader;
    std::ifstream editListFile(filename);
    if (!editListFile.is_open())
    {
        throw std::runtime_error("Unable to open input file '" + filename + "'");
    }

    // Parse the editlist file
    Json::Value edit;
    const bool parsingSuccess = jsonReader.parse(editListFile, edit, false);
    if (not parsingSuccess)
    {
        throw std::runtime_error("Failed to parse edit list file: " + jsonReader.getFormattedErrorMessages());
    }

    master.edit_list.numb_rept = std::stoi(edit["numb_rept"].asString());
    for (unsigned int editIndex = 0; editIndex < edit["edit_unit"].size(); ++editIndex)
    {
        IsoMediaFile::EditUnit editUnit;

        editUnit.edit_type = edit["edit_unit"][editIndex]["edit_type"].asString();
        editUnit.mdia_time = std::stoi(edit["edit_unit"][editIndex]["mdia_time"].asString());
        editUnit.time_span = std::stoi(edit["edit_unit"][editIndex]["time_span"].asString());

        master.edit_list.edit_unit.push_back(editUnit);
    }
}

void WriterConfig::readEditList(const std::string& filename, IsoMediaFile::Thumbs& thumbs) const
{
    Json::Reader jsonReader;
    std::ifstream editListFile(filename);
    if (!editListFile.is_open())
    {
        throw std::runtime_error("Unable to open input file '" + filename + "'");
    }

    // Parse the editlist file
    Json::Value edit;
    bool parsingSuccess = jsonReader.parse(editListFile, edit, false);
    if (not parsingSuccess)
    {
        throw std::runtime_error("Failed to parse edit list file: " + jsonReader.getFormattedErrorMessages());
    }

    thumbs.edit_list.numb_rept = std::stoi(edit["numb_rept"].asString());
    for (unsigned int editIndex = 0; editIndex < edit["edit_unit"].size(); ++editIndex)
    {
        IsoMediaFile::EditUnit newEditUnit;

        newEditUnit.edit_type = edit["edit_unit"][editIndex]["edit_type"].asString();
        newEditUnit.mdia_time = std::stoi(edit["edit_unit"][editIndex]["mdia_time"].asString());
        newEditUnit.time_span = std::stoi(edit["edit_unit"][editIndex]["time_span"].asString());

        thumbs.edit_list.edit_unit.push_back(newEditUnit);
    }
}

IsoMediaFile::General WriterConfig::readGeneral(const Json::Value& config) const
{
    IsoMediaFile::General general;
    // The General.Output parameters
    if (config["output"]["file_path"].isNull())
    {
        throw std::runtime_error("No output file path defined in input configuration");
    }
    general.output_file = config["output"]["file_path"].asString();

    // The General.Brands parameters
    if (config["brands"]["major"].isNull())
    {
        throw std::runtime_error("No general brands defined in input configuration");
    }
    general.brands.major = config["brands"]["major"].asString();

    Json::Value brandValues = config["brands"]["other"];
    for (const auto& brand : brandValues)
    {
        general.brands.other.push_back(brand.asString());
    }

    // Primary Item uniq_bsid and index
    general.prim_refr = readOptionalUint(config["prim_refr"]);
    general.prim_indx = readOptionalUint(config["prim_indx"]);

    return general;
}

std::vector<IsoMediaFile::Egroup> WriterConfig::readEgroups(const Json::Value& config) const
{
    std::vector<IsoMediaFile::Egroup> egroups;
    for (const auto& member : config.getMemberNames())
    {
        const Json::Value entryValues = config[member];
        for (unsigned int i = 0; i < entryValues.size(); ++i)
        {
            IsoMediaFile::Egroup egroup;
            egroup.type = member;
            const Json::Value idxsListValues = entryValues[i]["idxs_list"];
            for (unsigned int k = 0; k < idxsListValues.size(); ++k)
            {
                IsoMediaFile::IndexPair indexPair;
                indexPair.uniq_bsid = stoi(idxsListValues[k][0].asString());
                indexPair.item_indx = stoi(idxsListValues[k][1].asString());
                egroup.idxs_lists.push_back(indexPair);
            }
            egroups.push_back(egroup);
        }
    }
    return egroups;
}

IsoMediaFile::Iovl WriterConfig::readIovl(const Json::Value& config) const
{
    IsoMediaFile::Iovl iovl;
    iovl.outputHeight = readUint32(config, "output_height");
    iovl.outputWidth = readUint32(config, "output_width");
    iovl.uniq_bsid = readUint32(config, "uniq_bsid");
    for (const auto& canvasFill : config["canvas_fill"])
    {
        iovl.canvasFillValue.push_back(std::stoi(canvasFill.asString()));
    }
    if (iovl.canvasFillValue.size() != 4)
    {
        throw std::runtime_error("Invalid iovl canvas_fill");
    }
    for (const auto& reference : config["refs_list"])
    {
        iovl.refs_list.push_back(std::stoi(reference.asString()));
    }
    for (const auto& idxsList : config["idxs_list"])
    {
        std::vector<std::uint32_t> idxListEntries;
        for (const auto& index : idxsList)
        {
            idxListEntries.push_back(std::stoi(index.asString()));
        }
        iovl.idxs_list.push_back(idxListEntries);
    }
    for (const auto& offset : config["offsets"])
    {
        IsoMediaFile::Iovl::Offset newOffset;
        newOffset.horizontal = std::stoi(offset[0].asString()); ///< @todo Check that this is the intended order
        newOffset.vertical = std::stoi(offset[1].asString()); ///< @todo Check that this is the intended order
        iovl.offsets.push_back(newOffset);
    }
    return iovl;
}

IsoMediaFile::Grid WriterConfig::readGrid(const Json::Value& config) const
{
    IsoMediaFile::Grid grid;
    grid.columns = readUint32(config, "columns");
    grid.outputHeight = readUint32(config, "output_height");
    grid.outputWidth = readUint32(config, "output_width");
    grid.rows = readUint32(config, "rows");
    grid.uniq_bsid = readUint32(config, "uniq_bsid");
    for (const auto& reference : config["refs_list"])
    {
        grid.refs_list.push_back(std::stoi(reference.asString()));
    }
    for (const auto& idxsList : config["idxs_list"])
    {
        std::vector<std::uint32_t> idxListEntries;
        for (const auto& index : idxsList)
        {
            idxListEntries.push_back(std::stoi(index.asString()));
        }
        grid.idxs_list.push_back(idxListEntries);
    }
    return grid;
}

IsoMediaFile::Thumbs WriterConfig::readThumbs(const Json::Value& config, const std::string& encapsulationType) const
{
    IsoMediaFile::Thumbs thumb;
    thumb.uniq_bsid = readOptionalUint(config["uniq_bsid"]);
    thumb.file_path = config["file_path"].asString();
    thumb.hdlr_type = config["hdlr_type"].asString();
    thumb.code_type = config["code_type"].asString();
    if (encapsulationType == "trak")
    {
        thumb.disp_xdim = readUint32(config, "disp_xdim");
        thumb.disp_ydim = readUint32(config, "disp_ydim");
        thumb.ccst = readCodingConstraints(config["ccst"]);
    }
    thumb.tick_rate = readOptionalUint(config["tick_rate"], 90000);
    thumb.sync_rate = readOptionalUint(config["sync_rate"], 0);

    const Json::Value syncidxs_array = config["sync_idxs"];
    for (const auto& synxIndex : syncidxs_array)
    {
        thumb.sync_idxs.push_back(std::stoi(synxIndex.asString()));
    }

    return thumb;
}

IsoMediaFile::Layer WriterConfig::readLayer(const Json::Value& config) const
{
    IsoMediaFile::Layer layer;
    layer.uniq_bsid = readOptionalUint(config["uniq_bsid"]);
    layer.base_refr = readOptionalUint(config["base_refr"]);
    layer.file_path = config["file_path"].asString();
    layer.hdlr_type = config["hdlr_type"].asString();
    layer.code_type = config["code_type"].asString();
    layer.hidden = readOptionalBool(config["hidden"], false);

    layer.target_outputlayer = config["tols_value"].asUInt();
    if (config["lsel_value"].empty())
    {
        layer.layer_selection = -1;
    }
    else
    {
        layer.layer_selection = config["lsel_value"].asInt();
    }

    return layer;
}

IsoMediaFile::PreDerived WriterConfig::readPrederived(const Json::Value& config) const
{
    IsoMediaFile::PreDerived preDerived;
    preDerived.uniq_bsid = readUint32(config, "uniq_bsid");
    preDerived.pre_refs_list = parseRefsList(config["pre_refs_list"]);
    preDerived.pre_idxs_list = parseIndexList(config["pre_idxs_list"]);
    preDerived.base_refs_list = parseRefsList(config["base_refs_list"]);
    preDerived.base_idxs_list = parseIndexList(config["base_idxs_list"]);
    return preDerived;
}

IsoMediaFile::Content WriterConfig::readContent(const Json::Value& config) const
{
    IsoMediaFile::Content content;
    auto& master = content.master;

    /// @todo Throw an exception if a content does not have the master.

    // The Content.Master parameters
    const Json::Value& masterValues = config["master"];
    master.uniq_bsid = readOptionalUint(masterValues["uniq_bsid"]);
    master.make_vide = readOptionalBool(masterValues["make_vide"], false);
    master.write_alternates = readOptionalBool(masterValues["write_alternates"], true);
    master.hidden = readOptionalBool(masterValues["hidden"], false);
    master.file_path = masterValues["file_path"].asString();
    master.hdlr_type = masterValues["hdlr_type"].asString();
    master.code_type = masterValues["code_type"].asString();
    master.encp_type = masterValues["encp_type"].asString();
    if (master.encp_type == "trak")
    {
        master.disp_xdim = readUint32(masterValues, "disp_xdim");
        master.disp_ydim = readUint32(masterValues, "disp_ydim");
        master.ccst = readCodingConstraints(masterValues["ccst"]);
    }
    master.disp_rate = readOptionalUint(masterValues["disp_rate"], 0);
    master.tick_rate = readOptionalUint(masterValues["tick_rate"], 90000);

    // Check that tick_rate is divisible by disp_rate so no rounding errors occur in frame timing
    if (master.disp_rate && (master.tick_rate % master.disp_rate))
    {
        logWarning() << "Frame timing inaccuracy will occur for master '"
                     << master.file_path << "' because given tick_rate ("
                     << master.tick_rate << ") is not divisible by given disp_rate ("
                     << master.disp_rate << ")!" << std::endl;
    }

    // If content master has an edit list file
    if (masterValues["edit_file"].asString() != "")
    {
        readEditList(masterValues["edit_file"].asString(), master);
    }

    // The Content.Thumb parameters
    for (const auto& thumb : config["thumbs"])
    {
        IsoMediaFile::Thumbs newThumb = readThumbs(thumb, content.master.encp_type);
        // If this thumbs has an edit list file
        if (thumb["edit_file"].asString() != "")
        {
            readEditList(thumb["edit_file"].asString(), newThumb);
        }

        content.thumbs.push_back(newThumb);
    }

    for (const auto& layer : config["layers"])
    {
        content.layers.push_back(readLayer(layer));
    }

    for (const auto& metadata : config["metadata"])
    {
        IsoMediaFile::Metadata newMetadata;

        newMetadata.file_path = metadata["file_path"].asString();
        newMetadata.hdlr_type = metadata["hdlr_type"].asString();

        content.metadata.push_back(newMetadata);
    }

    const Json::Value& derivedValues = config["derived"];
    if (not derivedValues.isNull())
    {
        IsoMediaFile::Derived newDerived;

        for (const auto& imir : derivedValues["imir"])
        {
            IsoMediaFile::Imir newImir = readImir(imir);
            newDerived.imirs.push_back(newImir);
        }

        for (const auto& irot : derivedValues["irot"])
        {
            IsoMediaFile::Irot newIrot = readIrot(irot);
            newDerived.irots.push_back(newIrot);
        }

        for (const auto& rloc : derivedValues["rloc"])
        {
            IsoMediaFile::Rloc newRloc = readRloc(rloc);
            newDerived.rlocs.push_back(newRloc);
        }

        for (const auto& clap : derivedValues["clap"])
        {
            IsoMediaFile::Clap newClap = readClap(clap);
            newDerived.claps.push_back(newClap);
        }

        for (const auto& prederived : derivedValues["pre-derived"])
        {
            IsoMediaFile::PreDerived newPreDerived = readPrederived(prederived);
            newDerived.prederiveds.push_back(newPreDerived);
        }

        for (const auto& grid : derivedValues["grid"])
        {
            IsoMediaFile::Grid newGrid = readGrid(grid);
            newDerived.grids.push_back(newGrid);
        }

        for (const auto& iovl : derivedValues["iovl"])
        {
            IsoMediaFile::Iovl newIovl = readIovl(iovl);
            newDerived.iovls.push_back(newIovl);
        }

        /// @todo Add other derived image types as well.

        content.derived = newDerived;
    }

    content.property = readProperty(config["property"]);

    for (const auto& auxiliary : config["auxiliary"])
    {
        content.auxiliary.push_back(readAuxiliary(auxiliary));
    }

    return content;
}

IsoMediaFile::Clap WriterConfig::readClap(const Json::Value& clap) const
{
    IsoMediaFile::Clap newClap;
    newClap.essential = readOptionalBool(clap["essential"], true);
    newClap.clapWidthN = readUint32(clap, "clapWidthN");
    newClap.clapWidthD = readUint32(clap, "clapWidthD");
    newClap.clapHeightN = readUint32(clap, "clapHeightN");
    newClap.clapHeightD = readUint32(clap, "clapHeightD");
    newClap.horizOffN = readUint32(clap, "horizOffN");
    newClap.horizOffD = readUint32(clap, "horizOffD");
    newClap.vertOffN = readUint32(clap, "vertOffN");
    newClap.vertOffD = readUint32(clap, "vertOffD");
    newClap.uniq_bsid = readOptionalUint(clap["uniq_bsid"]);
    newClap.refs_list = parseRefsList(clap["refs_list"]);
    newClap.idxs_list = parseIndexList(clap["idxs_list"]);
    return newClap;
}

IsoMediaFile::Rloc WriterConfig::readRloc(const Json::Value& config) const
{
    IsoMediaFile::Rloc rloc;
    rloc.essential = readOptionalBool(config["essential"], true);
    rloc.horizontal_offset = readUint32(config, "horizontal_offset");
    rloc.vertical_offset = readUint32(config, "vertical_offset");
    rloc.uniq_bsid = readOptionalUint(config["uniq_bsid"]);
    rloc.refs_list = parseRefsList(config["refs_list"]);
    rloc.idxs_list = parseIndexList(config["idxs_list"]);
    return rloc;
}

IsoMediaFile::Irot WriterConfig::readIrot(const Json::Value& config) const
{
    IsoMediaFile::Irot irot;
    irot.essential = readOptionalBool(config["essential"], true);
    irot.angle = readUint32(config, "angle");
    irot.uniq_bsid = readOptionalUint(config["uniq_bsid"]);
    irot.refs_list = parseRefsList(config["refs_list"]);
    irot.idxs_list = parseIndexList(config["idxs_list"]);
    return irot;
}

IsoMediaFile::Imir WriterConfig::readImir(const Json::Value& config) const
{
    IsoMediaFile::Imir imir;
    imir.essential = readOptionalBool(config["essential"], true);
    imir.horizontalAxis = readBool(config, "horiz_axis");
    imir.uniq_bsid = readOptionalUint(config["uniq_bsid"]);
    imir.refs_list = parseRefsList(config["refs_list"]);
    imir.idxs_list = parseIndexList(config["idxs_list"]);
    return imir;
}

IsoMediaFile::Property WriterConfig::readProperty(const Json::Value& config) const
{
    IsoMediaFile::Property property;

    for (const auto& imir : config["imir"])
    {
        IsoMediaFile::Imir newImir = readImir(imir);
        property.imirs.push_back(newImir);
    }

    for (const auto& irot : config["irot"])
    {
        IsoMediaFile::Irot newIrot = readIrot(irot);
        property.irots.push_back(newIrot);
    }

    for (const auto& rloc : config["rloc"])
    {
        IsoMediaFile::Rloc newRloc = readRloc(rloc);
        property.rlocs.push_back(newRloc);
    }

    for (const auto& clap : config["clap"])
    {
        IsoMediaFile::Clap newClap = readClap(clap);
        property.claps.push_back(newClap);
    }

    return property;
}

IsoMediaFile::Auxiliary WriterConfig::readAuxiliary(const Json::Value& config) const
{
    IsoMediaFile::Auxiliary aux;
    aux.file_path = config["file_path"].asString();
    aux.idxs_list = parseIndexList(config["idxs_list"]);
    aux.refs_list = parseRefsList(config["refs_list"]);
    aux.uniq_bsid = readOptionalUint(config["uniq_bsid"]);
    aux.hidden = readOptionalBool(config["hidden"], true);

    aux.urn = config["urn"].asString();
    aux.code_type = config["code_type"].asString();

    return aux;
}

void WriterConfig::confDump(const IsoMediaFile::Configuration& configValues, const Json::Value& jsonValues) const
{
    logInfo() << "config.general.output_file: " << configValues.general.output_file << std::endl;
    logInfo() << "config.general.brands.major: " << configValues.general.brands.major << std::endl;
    logInfo() << "config.general.brands.other: ";
    for (unsigned int i = 0; i < configValues.general.brands.other.size(); ++i)
    {
        logInfo() << configValues.general.brands.other[i] << ", ";
    }
    logInfo() << std::endl;

    for (unsigned int i = 0; i < configValues.content.size(); ++i)
    {
        const std::string contentStr = "config.content[" + std::to_string(i) + "].";
        const std::string masterStr = contentStr + "master.";
        const auto& content = configValues.content[i];
        const auto& master = content.master;
        logInfo() << masterStr << "uniq_bsid: " << master.uniq_bsid << std::endl;
        logInfo() << masterStr << "file_path: " << master.file_path << std::endl;
        logInfo() << masterStr << "code_type: " << master.code_type << std::endl;
        logInfo() << masterStr << "encp_type: " << master.encp_type << std::endl;
        if (master.encp_type == "trak")
        {
            logInfo() << masterStr << "tick_rate: " << master.tick_rate << std::endl;
            logInfo() << masterStr << "disp_rate: " << master.disp_rate << std::endl;
            logInfo() << masterStr << "disp_xdim: " << master.disp_xdim << std::endl;
            logInfo() << masterStr << "disp_ydim: " << master.disp_ydim << std::endl;
        }

        if (jsonValues["content"][i]["master"]["edit_file"].asString() != "")
        {
            const std::string editListStr = masterStr + "edit_list.";
            logInfo() << editListStr << "loop_flag: " << master.edit_list.numb_rept << std::endl;
            for (unsigned int j = 0; j < configValues.content[i].master.edit_list.edit_unit.size(); j++)
            {
                const std::string editUnitStr = editListStr + "edit_unit[" + std::to_string(j) + "].";
                const auto& editUnit = master.edit_list.edit_unit[j];
                logInfo() << editUnitStr << "edit_type: " << editUnit.edit_type << std::endl;
                logInfo() << editUnitStr << "mdia_time: " << editUnit.mdia_time << std::endl;
                logInfo() << editUnitStr << "time_span: " << editUnit.time_span << std::endl;
            }
        }

        for (unsigned int j = 0; j < configValues.content[i].thumbs.size(); ++j)
        {
            const std::string thumbsStr = contentStr + "thumbs[" + std::to_string(j) + "].";
            const auto& thumbs = configValues.content[i].thumbs[j];
            logInfo() << thumbsStr << "uniq_bsid: " << thumbs.uniq_bsid << std::endl;
            logInfo() << thumbsStr << "file_path: " << thumbs.file_path << std::endl;
            logInfo() << thumbsStr << "code_type: " << thumbs.code_type << std::endl;
            logInfo() << thumbsStr << "disp_xdim: " << thumbs.disp_xdim << std::endl;
            logInfo() << thumbsStr << "disp_ydim: " << thumbs.disp_ydim << std::endl;
            logInfo() << thumbsStr << "tick_rate: " << thumbs.tick_rate << std::endl;
            logInfo() << thumbsStr << "sync_rate: " << thumbs.sync_rate << std::endl;

            if (jsonValues["content"][i]["thumbs"][j]["edit_file"].asString() != "")
            {
                const std::string editListStr = thumbsStr + "edit_list.";
                const auto& editList = thumbs.edit_list;
                logInfo() << editListStr << "loop_flag: " << editList.numb_rept << std::endl;
                for (unsigned int k = 0; k < configValues.content[i].thumbs[j].edit_list.edit_unit.size(); k++)
                {
                    const std::string editUnitStr = editListStr + "edit_unit[" + std::to_string(k) + "].";
                    const auto& editUnit = editList.edit_unit[k];
                    logInfo() << editUnitStr << "edit_type: " << editUnit.edit_type << std::endl;
                    logInfo() << editUnitStr << "mdia_time: " << editUnit.mdia_time << std::endl;
                    logInfo() << editUnitStr << "time_span: " << editUnit.time_span << std::endl;
                }
            }

            for (unsigned int k = 0; k < configValues.content[i].thumbs[j].sync_idxs.size(); ++k)
            {
                logInfo() << thumbsStr << "sync_idxs[" << k << "]: " << thumbs.sync_idxs[k] << std::endl;
            }
        }
    }

    for (unsigned int i = 0; i < configValues.egroups.size(); ++i)
    {
        const std::string egroupStr = "config.egroups[" + std::to_string(i) + "].";
        const auto& idxList = configValues.egroups.at(i);
        logInfo() << egroupStr << "type: " << idxList.type << std::endl;
        for (unsigned int j = 0; j < idxList.idxs_lists.size(); ++j)
        {
            const std::string indxStr = "idxs_lists[" + std::to_string(j) + "].";
            logInfo() << egroupStr << indxStr << "uniq_bsid: " << idxList.idxs_lists.at(j).uniq_bsid << std::endl;
            logInfo() << egroupStr << indxStr << "item_id: " << idxList.idxs_lists.at(j).item_indx << std::endl;
        }
    }
}

std::uint32_t WriterConfig::readOptionalUint(const Json::Value& value, const std::uint32_t defaultValue) const
{
    const std::string valueString = value.asString();
    if (valueString != "")
    {
        return std::stoi(valueString);
    }

    return defaultValue;
}

std::uint32_t WriterConfig::readUint32(const Json::Value& value, const std::string& name) const
{
    const std::string valueString = value[name].asString();
    if (valueString != "")
    {
        return std::stoi(valueString);
    }

    throw std::runtime_error("Configuration missing a mandatory value for option '" + name + "'");
}

template<class t> void WriterConfig::readIntVector(const Json::Value& config, std::vector<t>& data) const
{
    for (const auto& value : config)
    {
        data.push_back(std::stoi(value.asString()));
    }
}

bool WriterConfig::readBool(const Json::Value& value, const std::string& name) const
{
    // Check that value exists
    if (value[name].asString() == "")
    {
        throw std::runtime_error("Configuration missing a mandatory value for option '" + name + "'");
    }

    // Continue with the common handling
    return readOptionalBool(value[name], false); // Default value never used because config value exists
}

bool WriterConfig::readOptionalBool(const Json::Value& value, const bool defaultValue) const
{
    const std::string valueString = value.asString();
    if (valueString == "")
    {
        return defaultValue;
    }
    else if (valueString == "1" || valueString == "true")
    {
        return true;
    }
    else if (valueString == "0" || valueString == "false")
    {
        return false;
    }

    throw std::runtime_error("Invalid boolean value in configuration. Must be \"1\" or \"0\"");
}

IsoMediaFile::ReferenceList WriterConfig::parseRefsList(const Json::Value& config) const
{
    IsoMediaFile::ReferenceList result;
    for (const auto& reference : config)
    {
        result.push_back(std::stoi(reference.asString()));
    }
    return result;
}

IsoMediaFile::IndexList WriterConfig::parseIndexList(const Json::Value& config) const
{
    IsoMediaFile::IndexList result;
    for (const auto& index_array : config)
    {
        std::vector<std::uint32_t> idx_list_entries;
        for (const auto& index : index_array)
        {
            idx_list_entries.push_back(std::stoi(index.asString()));
        }
        result.push_back(idx_list_entries);
    }

    return result;
}

IsoMediaFile::CodingConstraints WriterConfig::readCodingConstraints(const Json::Value& config) const
{
    IsoMediaFile::CodingConstraints ccst;
    ccst.allRefPicsIntra = readOptionalBool(config["all_ref_pics_intra"], true);
    ccst.intraPredUsed = readOptionalBool(config["intra_pred_used"], false);
    return ccst;
}
