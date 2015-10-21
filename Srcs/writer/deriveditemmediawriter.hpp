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

#ifndef DERIVEDITEMMEDIAWRITER_HPP
#define DERIVEDITEMMEDIAWRITER_HPP

#include "mediadatabox.hpp"
#include "mediawriter.hpp"

/**
 * @brief MediaWriter for creating a MediaDataBox which contains derived item data.
 * @details Some derived image items (at least 'grid' and 'iovl') contain data which is located in MediaDataBox. This
 * writer co-operates with MetaDerivedimageWriter to create that MediaDataBox. To prevent item data creation twice,
 * MetaDerivedImageWriter creates MediaDataBox content and sets it to DerivedItemMediaWriter by using the method setMdat,
 * to make data accessible in later phase of file writing.
 * @see MetaDerivedImageWriter
 */
class DerivedItemMediaWriter : public MediaWriter
{
public:
    DerivedItemMediaWriter() = default;
    virtual ~DerivedItemMediaWriter() = default;

    /**
     * @brief Set the MediaDataBox content.
     * @details This method should be called from the corresponding MetaDerivedImageWriter.
     * @param mediaDataBox Data
     */
    void setMdat(const MediaDataBox& mediaDataBox);

    /**
     * @brief Create a MediaDataBox containing derived image item data.
     * @see MediaWriter::writeMedia()
     */
    virtual MediaDataBox writeMedia();

private:
    MediaDataBox mMdat; ///< Data content of derived items
};

#endif
