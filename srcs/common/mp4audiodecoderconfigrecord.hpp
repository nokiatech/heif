/* This file is part of Nokia HEIF library
*
* Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
*
* Contact: heif@nokia.com
*
* This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
*
* Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
*/

#ifndef MP4AUDIODECODERCONFIGRECORD_HPP
#define MP4AUDIODECODERCONFIGRECORD_HPP

#include "decoderconfigrecord.hpp"
class ElementaryStreamDescriptorBox;

class MP4AudioDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:
    MP4AudioDecoderConfigurationRecord(ElementaryStreamDescriptorBox& aBox);
    virtual ~MP4AudioDecoderConfigurationRecord() = default;
    
    /* @brief Returns configuration parameter map for this record */
    virtual void getConfigurationMap(ConfigurationMap& aMap) const override;

protected:
    const ElementaryStreamDescriptorBox& mESDBox;
};

#endif /* end of include guard: MP4AUDIODECODERCONFIGRECORD_HPP*/
