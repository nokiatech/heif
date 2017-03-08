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

#ifndef WRITERCONFIG_HPP
#define WRITERCONFIG_HPP

#include "isomediafile.hpp"
#include "json/json-forwards.h"

#include <limits>
#include <string>
#include <vector>

/**
 * \brief Handles setting up of the general parameter of the file.
 */
class WriterConfig
{
public:
    WriterConfig() = default;
    virtual ~WriterConfig() = default;

    IsoMediaFile::Configuration readJson(const std::string& fileName);

private:
    void confDump(const IsoMediaFile::Configuration& config, const Json::Value& jsonValues) const;
    IsoMediaFile::General readGeneral(const Json::Value& config) const;
    std::vector<IsoMediaFile::Egroup> readEgroups(const Json::Value& config) const;
    IsoMediaFile::Content readContent(const Json::Value& config) const;
    IsoMediaFile::Auxiliary readAuxiliary(const Json::Value& config) const;
    IsoMediaFile::Property readProperty(const Json::Value& config) const;

    /** @return Coding Constraints Box values all_ref_pic_intra and intra_pred_used, for tracks */
    IsoMediaFile::CodingConstraints readCodingConstraints(const Json::Value& ccstValues) const;

    void readEditList(const std::string& fileName, IsoMediaFile::Master& master) const;
    void readEditList(const std::string& fileName, IsoMediaFile::Thumbs& thumbs) const;

    /** @return value as integer, use value from defaultValue if the value is not set */
    std::uint32_t readOptionalUint(const Json::Value& value,
        std::uint32_t defaultValue = std::numeric_limits<std::uint32_t>::max()) const;

    std::uint32_t readUint32(const Json::Value& value, const std::string& name) const;

    template<class t> void readIntVector(const Json::Value& config, std::vector<t>& data) const;

    /** @return True if "1", false if "0", fail if the value is not set */
    bool readBool(const Json::Value& value, const std::string& name) const;

    /** @return True if "1", false if "0", 'defaultValue' if the value is not set */
    bool readOptionalBool(const Json::Value& value, bool defaultValue) const;

    IsoMediaFile::ReferenceList parseRefsList(const Json::Value& config) const;
    IsoMediaFile::IndexList parseIndexList(const Json::Value& config) const;
    IsoMediaFile::Iovl readIovl(const Json::Value& config) const;
    IsoMediaFile::Grid readGrid(const Json::Value& config) const;
    IsoMediaFile::Thumbs readThumbs(const Json::Value& config, const std::string& encapsulationType) const;
    IsoMediaFile::Clap readClap(const Json::Value& config) const;
    IsoMediaFile::Rloc readRloc(const Json::Value& config) const;
    IsoMediaFile::Irot readIrot(const Json::Value& config) const;
    IsoMediaFile::Imir readImir(const Json::Value& config) const;
    IsoMediaFile::PreDerived readPrederived(const Json::Value& config) const;
    IsoMediaFile::Layer readLayer(const Json::Value& config) const;
};

#endif /* end of include guard: WRITERCONFIG_HPP */

