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

#ifndef ITEMDATABOX_HPP
#define ITEMDATABOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"

#include <vector>

class ItemDataBox : public Box
{
public:
    ItemDataBox();
    virtual ~ItemDataBox() = default;

    void read(std::vector<std::uint8_t>& destination, size_t offset, size_t length) const;
    unsigned int addData(const std::vector<std::uint8_t>& data);

    virtual void writeBox(BitStream& bitstr);
    virtual void parseBox(BitStream& bitstr);

private:
    std::vector<std::uint8_t> mData;
};

#endif
