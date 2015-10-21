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

#ifndef METABOXWRITER_HPP
#define METABOXWRITER_HPP

#include "metabox.hpp"
#include <memory>

class MetaWriter;

/**
 * Create a MetaBox, and serialize it to BitStream.
 */
class MetaBoxWriter
{
public:
    MetaBoxWriter();
    virtual ~MetaBoxWriter() = default;

    /**
     * Add data to MetaBox from MetaWriter.
     * @param metaWriter MetaWriter
     * @param offset     Offset of the related MediaDataBox
     */
    void writeToMetaBox(MetaWriter* metaWriter, unsigned int offset);

    /**
     * Serialize MetaBox data to the BitStream.
     * @param bitStream Output BitStream
     */
    void serialize(BitStream& bitStream);

private:
    std::unique_ptr<MetaBox> mMetaBox; ///< MetaBox to be filled and serialized
};

#endif /* end of include guard: METABOXWRITER_HPP */
