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

#include "writercfg.hpp"
#include "json.hh"
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
        throw std::runtime_error("Failed to parse input configuration: " + jsonReader.getFormatedErrorMessages());
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

    configuration.egroups = readEgroups(jsonValues["egroups"]);

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
        throw std::runtime_error("Failed to parse edit list file: " + jsonReader.getFormatedErrorMessages());
    }

    master.edit_list.numb_rept = std::stoi(edit["numb_rept"].asString());
    for (unsigned int editIndex = 0; editIndex < edit["edit_unit"].size(); ++editIndex)
    {
        IsoMediaFile::EditUnit newEditUnit;

        newEditUnit.edit_type = edit["edit_unit"][editIndex]["edit_type"].asString();
        newEditUnit.mdia_time = std::stoi(edit["edit_unit"][editIndex]["mdia_time"].asString());
        newEditUnit.time_span = std::stoi(edit["edit_unit"][editIndex]["time_span"].asString());

        master.edit_list.edit_unit.push_back(newEditUnit);
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
        throw std::runtime_error("Failed to parse edit list file: " + jsonReader.getFormatedErrorMessages());
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

IsoMediaFile::General WriterConfig::readGeneral(const Json::Value& generalValues) const
{
    IsoMediaFile::General general;
    // The General.Output parameters
    if (generalValues["output"]["file_path"].isNull())
    {
        throw std::runtime_error("No output file path defined in input configuration");
    }
    general.output_file = generalValues["output"]["file_path"].asString();

    // The General.Brands parameters
    if (generalValues["brands"]["major"].isNull())
    {
        throw std::runtime_error("No general brands defined in input configuration");
    }
    general.brands.major = generalValues["brands"]["major"].asString();

    Json::Value brandValues = generalValues["brands"]["other"];
    for (const auto& brand : brandValues)
    {
        general.brands.other.push_back(brand.asString());
    }

    // Primary Item uniq_bsid and index
    general.prim_refr = readOptionalUint(generalValues["prim_refr"]);
    general.prim_indx = readOptionalUint(generalValues["prim_indx"]);

    return general;
}

IsoMediaFile::Egroups WriterConfig::readEgroups(const Json::Value& egroupsValues) const
{
    IsoMediaFile::Egroups egroups;
    const Json::Value altrValues = egroupsValues[0]["altr"];
    for (unsigned int i = 0; i < altrValues.size(); ++i)
    {
        IsoMediaFile::AltrIndexPairVector altrIndexPairVector;
        const Json::Value idxsListValues = altrValues[i]["idxs_list"];
        for (unsigned int k = 0; k < idxsListValues.size(); ++k)
        {
            IsoMediaFile::AltrIndexPair altrIndexPair;
            altrIndexPair.uniq_bsid = stoi(idxsListValues[k][0].asString());
            altrIndexPair.item_indx = stoi(idxsListValues[k][1].asString());
            altrIndexPairVector.push_back(altrIndexPair);
        }
        egroups.altr.idxs_lists.push_back(altrIndexPairVector);
    }

    return egroups;
}

IsoMediaFile::Content WriterConfig::readContent(const Json::Value& contentValues) const
{
    IsoMediaFile::Content newContent;

    /// @todo Throw an exception if a content does not have the master.

    // The Content.Master parameters
    const Json::Value& masterValues = contentValues["master"];
    newContent.master.uniq_bsid = readOptionalUint(masterValues["uniq_bsid"]);
    newContent.master.make_vide = readBool(masterValues["make_vide"], false);
    newContent.master.file_path = masterValues["file_path"].asString();
    newContent.master.hdlr_type = masterValues["hdlr_type"].asString();
    newContent.master.code_type = masterValues["code_type"].asString();
    newContent.master.encp_type = masterValues["encp_type"].asString();
    newContent.master.disp_xdim = readUint32(masterValues, "disp_xdim");
    newContent.master.disp_ydim = readUint32(masterValues, "disp_ydim");
    newContent.master.disp_rate = readOptionalUint(masterValues["disp_rate"], 0);
    newContent.master.tick_rate = readOptionalUint(masterValues["tick_rate"], 90000);

    // If content master has an edit list file
    if (masterValues["edit_file"].asString() != "")
    {
        readEditList(masterValues["edit_file"].asString(), newContent.master);
    }

    // The Content.Thumb parameters
    for (const auto& thumb : contentValues["thumbs"])
    {
        IsoMediaFile::Thumbs newThumb;

        newThumb.uniq_bsid = readOptionalUint(thumb["uniq_bsid"]);
        newThumb.file_path = thumb["file_path"].asString();
        newThumb.hdlr_type = thumb["hdlr_type"].asString();
        newThumb.code_type = thumb["code_type"].asString();
        newThumb.disp_xdim = readUint32(thumb, "disp_xdim");
        newThumb.disp_ydim = readUint32(thumb, "disp_ydim");
        newThumb.tick_rate = readOptionalUint(thumb["tick_rate"], 90000);
        newThumb.sync_rate = readOptionalUint(thumb["sync_rate"], 0);

        const Json::Value syncidxs_array = thumb["sync_idxs"];
        for (const auto& synxIndex : syncidxs_array)
        {
            newThumb.sync_idxs.push_back(std::stoi(synxIndex.asString()));
        }

        // If this thumbs has an edit list file
        if (thumb["edit_file"].asString() != "")
        {
            readEditList(thumb["edit_file"].asString(), newThumb);
        }

        newContent.thumbs.push_back(newThumb);
    }

    for (const auto& metadata : contentValues["metadata"])
    {
        IsoMediaFile::Metadata newMetadata;

        newMetadata.file_path = metadata["file_path"].asString();
        newMetadata.hdlr_type = metadata["hdlr_type"].asString();

        newContent.metadata.push_back(newMetadata);
    }

    const Json::Value& derivedValues = contentValues["derived"];
    if (not derivedValues.isNull())
    {
        IsoMediaFile::Derived newDerived;
        IsoMediaFile::Property newProperty;

        for (const auto& irot : derivedValues["irot"])
        {
            IsoMediaFile::Irot newIrot;
            newIrot.angle = readUint32(irot, "angle");
            newIrot.uniq_bsid  = readOptionalUint(irot["uniq_bsid"]);
            newIrot.refs_list = parseRefsList(irot["refs_list"]);
            newIrot.idxs_list = parseIndexList(irot["idxs_list"]);
            newDerived.irots.push_back(newIrot);
        }

        for (const auto& rloc : derivedValues["rloc"])
        {
            IsoMediaFile::Rloc newRloc;
            newRloc.horizontal_offset =  readUint32(rloc, "horizontal_offset");
            newRloc.vertical_offset =  readUint32(rloc, "vertical_offset");
            newRloc.uniq_bsid = readOptionalUint(rloc["uniq_bsid"]);
            newRloc.refs_list = parseRefsList(rloc["refs_list"]);
            newRloc.idxs_list = parseIndexList(rloc["idxs_list"]);
            newDerived.rlocs.push_back(newRloc);
        }

        for (const auto& clap : derivedValues["clap"])
        {
            IsoMediaFile::Clap newClap;
            newClap.clapWidthN = readUint32(clap, "clapWidthN");
            newClap.clapWidthD = readUint32(clap, "clapWidthD");
            newClap.clapHeightN = readUint32(clap, "clapHeightN");
            newClap.clapHeightD = readUint32(clap, "clapHeightD");
            newClap.horizOffN = readUint32(clap, "horizOffN");
            newClap.horizOffD = readUint32(clap, "horizOffD");
            newClap.vertOffN = readUint32(clap, "vertOffN");
            newClap.vertOffD = readUint32(clap, "vertOffD");
            newClap.uniq_bsid = readUint32(clap, "uniq_bsid");
            newClap.refs_list = parseRefsList(clap["refs_list"]);
            newClap.idxs_list = parseIndexList(clap["idxs_list"]);
            newDerived.claps.push_back(newClap);
        }

        for (const auto& prederived : derivedValues["pre-derived"])
        {
            IsoMediaFile::PreDerived newPreDerived;
            newPreDerived.uniq_bsid = readUint32(prederived, "uniq_bsid");

            newPreDerived.pre_refs_list = parseRefsList(prederived["pre_refs_list"]);
            newPreDerived.pre_idxs_list = parseIndexList(prederived["pre_idxs_list"]);
            newPreDerived.base_refs_list = parseRefsList(prederived["base_refs_list"]);
            newPreDerived.base_idxs_list = parseIndexList(prederived["base_idxs_list"]);
            newDerived.prederiveds.push_back(newPreDerived);
        }

        for (const auto& grid : derivedValues["grid"])
        {
            IsoMediaFile::Grid newGrid;
            newGrid.columns = readUint32(grid, "columns");
            newGrid.outputHeight = readUint32(grid, "output_height");
            newGrid.outputWidth = readUint32(grid, "output_width");
            newGrid.rows = readUint32(grid, "rows");
            newGrid.uniq_bsid = readUint32(grid, "uniq_bsid");

            for (const auto& reference : grid["refs_list"])
            {
                newGrid.refs_list.push_back(std::stoi(reference.asString()));
            }

            for (const auto& idxsList : grid["idxs_list"])
            {
                std::vector<std::uint32_t> idxListEntries;
                for (const auto& index : idxsList)
                {
                    idxListEntries.push_back(std::stoi(index.asString()));
                }
                newGrid.idxs_list.push_back(idxListEntries);
            }
            newDerived.grids.push_back(newGrid);
        }

        for (const auto& iovl : derivedValues["iovl"])
        {
            IsoMediaFile::Iovl newIovl;
            newIovl.outputHeight = readUint32(iovl, "output_height");
            newIovl.outputWidth = readUint32(iovl, "output_width");
            newIovl.uniq_bsid = readUint32(iovl, "uniq_bsid");

            for (const auto& canvasFill : iovl["canvas_fill"])
            {
                newIovl.canvasFillValue.push_back(std::stoi(canvasFill.asString()));
            }
            if (newIovl.canvasFillValue.size() != 4)
            {
                throw std::runtime_error("Invalid iovl canvas_fill");
            }

            for (const auto& reference : iovl["refs_list"])
            {
                newIovl.refs_list.push_back(std::stoi(reference.asString()));
            }

            for (const auto& idxsList : iovl["idxs_list"])
            {
                std::vector<std::uint32_t> idxListEntries;
                for (const auto& index : idxsList)
                {
                    idxListEntries.push_back(std::stoi(index.asString()));
                }
                newIovl.idxs_list.push_back(idxListEntries);
            }

            for (const auto& offset : iovl["offsets"])
            {
                IsoMediaFile::Iovl::Offset newOffset;
                newOffset.horizontal = std::stoi(offset[0].asString()); ///< @todo Check that this is the intended order
                newOffset.vertical = std::stoi(offset[1].asString()); ///< @todo Check that this is the intended order
                newIovl.offsets.push_back(newOffset);
            }
            newDerived.iovls.push_back(newIovl);
        }

        /// @todo Add other derived image types as well.

        newContent.derived = newDerived;
    }

    newContent.property = readProperty(contentValues["property"]);

    for (const auto& auxiliary : contentValues["auxiliary"])
    {
        newContent.auxiliary.push_back(readAuxiliary(auxiliary));
    }

    return newContent;
}

IsoMediaFile::Property WriterConfig::readProperty(const Json::Value& propertyValues) const
{
    IsoMediaFile::Property newProperty;

    for (const auto& irot : propertyValues["irot"])
    {
        IsoMediaFile::Irot newIrot;
        newIrot.essential = readBool(irot["essential"], true);
        newIrot.angle =  readUint32(irot, "angle");
        newIrot.uniq_bsid = readOptionalUint(irot["uniq_bsid"]);
        newIrot.refs_list = parseRefsList(irot["refs_list"]);
        newIrot.idxs_list = parseIndexList(irot["idxs_list"]);
        newProperty.irots.push_back(newIrot);
    }

    for (const auto& rloc : propertyValues["rloc"])
    {
        IsoMediaFile::Rloc newRloc;
        newRloc.essential = readBool(rloc["essential"], true);
        newRloc.horizontal_offset =  readUint32(rloc, "horizontal_offset");
        newRloc.vertical_offset =  readUint32(rloc, "vertical_offset");
        newRloc.uniq_bsid = readOptionalUint(rloc["uniq_bsid"]);
        newRloc.refs_list = parseRefsList(rloc["refs_list"]);
        newRloc.idxs_list = parseIndexList(rloc["idxs_list"]);
        newProperty.rlocs.push_back(newRloc);
    }

    for (const auto& clap : propertyValues["clap"])
    {
        IsoMediaFile::Clap newClap;
        newClap.essential = readBool(clap["essential"], true);
        newClap.clapWidthN = readUint32(clap, "clapWidthN");
        newClap.clapWidthD = readUint32(clap, "clapWidthD");
        newClap.clapHeightN = readUint32(clap, "clapHeightN");
        newClap.clapHeightD = readUint32( clap, "clapHeightD");
        newClap.horizOffN = readUint32(clap, "horizOffN");
        newClap.horizOffD = readUint32(clap, "horizOffD");
        newClap.vertOffN = readUint32(clap, "vertOffN");
        newClap.vertOffD = readUint32(clap, "vertOffD");
        newClap.uniq_bsid = readOptionalUint(clap["uniq_bsid"]);
        newClap.refs_list = parseRefsList(clap["refs_list"]);
        newClap.idxs_list = parseIndexList(clap["idxs_list"]);
        newProperty.claps.push_back(newClap);
    }

    return newProperty;
}

IsoMediaFile::Auxiliary WriterConfig::readAuxiliary(const Json::Value& auxValues) const
{
    IsoMediaFile::Auxiliary aux;
    aux.file_path = auxValues["file_path"].asString();
    aux.idxs_list = parseIndexList(auxValues["idxs_list"]);
    aux.refs_list = parseRefsList(auxValues["refs_list"]);
    aux.uniq_bsid = readOptionalUint(auxValues["uniq_bsid"]);
    aux.disp_xdim = readUint32(auxValues, "disp_xdim");
    aux.disp_ydim = readUint32(auxValues, "disp_ydim");
    aux.hidden = readBool(auxValues["hidden"], true);

    aux.urn = auxValues["urn"].asString();

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
        logInfo() << "config.content[" << i << "].master.uniq_bsid: " << configValues.content[i].master.uniq_bsid << std::endl;
        logInfo() << "config.content[" << i << "].master.file_path: " << configValues.content[i].master.file_path << std::endl;
        logInfo() << "config.content[" << i << "].master.code_type: " << configValues.content[i].master.code_type << std::endl;
        logInfo() << "config.content[" << i << "].master.encp_type: " << configValues.content[i].master.encp_type << std::endl;
        logInfo() << "config.content[" << i << "].master.disp_xdim: " << configValues.content[i].master.disp_xdim << std::endl;
        logInfo() << "config.content[" << i << "].master.disp_ydim: " << configValues.content[i].master.disp_ydim << std::endl;
        logInfo() << "config.content[" << i << "].master.disp_rate: " << configValues.content[i].master.disp_rate << std::endl;
        logInfo() << "config.content[" << i << "].master.tick_rate: " << configValues.content[i].master.tick_rate << std::endl;

        if (jsonValues["content"][i]["master"]["edit_file"].asString() != "")
        {
            logInfo() << "config.content[" << i << "].master.edit_list.loop_flag: " << configValues.content[i].master.edit_list.numb_rept << std::endl;
            for (unsigned int j = 0; j < configValues.content[i].master.edit_list.edit_unit.size(); j++)
            {
                logInfo() << "config.content[" << i << "].master.edit_list.edit_unit[" << j << "].edit_type: " << configValues.content[i].master.edit_list.edit_unit[j].edit_type << std::endl;
                logInfo() << "config.content[" << i << "].master.edit_list.edit_unit[" << j << "].mdia_time: " << configValues.content[i].master.edit_list.edit_unit[j].mdia_time << std::endl;
                logInfo() << "config.content[" << i << "].master.edit_list.edit_unit[" << j << "].time_span: " << configValues.content[i].master.edit_list.edit_unit[j].time_span << std::endl;
            }
        }

        for (unsigned int j = 0; j < configValues.content[i].thumbs.size(); ++j)
        {
            logInfo() << "config.content[" << i << "].thumbs[" << j << "].uniq_bsid: " << configValues.content[i].thumbs[j].uniq_bsid << std::endl;
            logInfo() << "config.content[" << i << "].thumbs[" << j << "].file_path: " << configValues.content[i].thumbs[j].file_path << std::endl;
            logInfo() << "config.content[" << i << "].thumbs[" << j << "].code_type: " << configValues.content[i].thumbs[j].code_type << std::endl;
            logInfo() << "config.content[" << i << "].thumbs[" << j << "].disp_xdim: " << configValues.content[i].thumbs[j].disp_xdim << std::endl;
            logInfo() << "config.content[" << i << "].thumbs[" << j << "].disp_ydim: " << configValues.content[i].thumbs[j].disp_ydim << std::endl;
            logInfo() << "config.content[" << i << "].thumbs[" << j << "].tick_rate: " << configValues.content[i].thumbs[j].tick_rate << std::endl;
            logInfo() << "config.content[" << i << "].thumbs[" << j << "].sync_rate: " << configValues.content[i].thumbs[j].sync_rate << std::endl;

            if (jsonValues["content"][i]["thumbs"][j]["edit_file"].asString() != "")
            {
                logInfo() << "config.content[" << i << "].thumbs[" << j << "].edit_list.loop_flag: " << configValues.content[i].master.edit_list.numb_rept << std::endl;
                for (unsigned int k = 0; k < configValues.content[i].thumbs[j].edit_list.edit_unit.size(); k++)
                {
                    logInfo() << "config.content[" << i << "].thumbs[" << j << "].edit_list.edit_unit[" << k << "].edit_type: " << configValues.content[i].master.edit_list.edit_unit[j].edit_type
                        << std::endl;
                    logInfo() << "config.content[" << i << "].thumbs[" << j << "].edit_list.edit_unit[" << k << "].mdia_time: " << configValues.content[i].master.edit_list.edit_unit[j].mdia_time
                        << std::endl;
                    logInfo() << "config.content[" << i << "].thumbs[" << j << "].edit_list.edit_unit[" << k << "].time_span: " << configValues.content[i].master.edit_list.edit_unit[j].time_span
                        << std::endl;
                }
            }

            for (unsigned int k = 0; k < configValues.content[i].thumbs[j].sync_idxs.size(); ++k)
            {
                logInfo() << "config.content[" << i << "].thumbs[" << j << "].sync_idxs[" << k << "]: " << configValues.content[i].thumbs[j].sync_idxs[k] << std::endl;
            }
        }
    }

    for (unsigned int i = 0; i < configValues.egroups.altr.idxs_lists.size(); ++i)
    {
        for (unsigned int j = 0; j < configValues.egroups.altr.idxs_lists.at(i).size(); ++j)
        {
            logInfo() << "config.egroups.altr.idxs_lists[" << i << "][" << j << "].uniq_bsid:" << configValues.egroups.altr.idxs_lists.at(i).at(j).uniq_bsid << std::endl;
            logInfo() << "config.egroups.altr.idxs_lists[" << i << "][" << j << "].item_id:" << configValues.egroups.altr.idxs_lists.at(i).at(j).item_indx << std::endl;
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

bool WriterConfig::readBool(const Json::Value& value, const bool defaultValue) const
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

IsoMediaFile::ReferenceList WriterConfig::parseRefsList(const Json::Value& referenceList) const
{
    IsoMediaFile::ReferenceList result;
    for (const auto& reference : referenceList)
    {
        result.push_back(std::stoi(reference.asString()));
    }
    return result;
}

IsoMediaFile::IndexList WriterConfig::parseIndexList(const Json::Value& indexList) const
{
    IsoMediaFile::IndexList result;
    for (const auto& index_array : indexList)
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
