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

#ifndef SAMPLEDESCRIPTIONBOX_HPP
#define SAMPLEDESCRIPTIONBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"
#include "sampleentrybox.hpp"

#include <memory>
#include <vector>

/** @brief SampleDescriptionBox class. Extends from FullBox.
 *  @details This class makes use of templates in order to support different sample entry types. **/
class SampleDescriptionBox : public FullBox
{
public:
    SampleDescriptionBox();
    ~SampleDescriptionBox() = default;

    /** @brief Add a new sample entry.
     *  @param [in] sampleEntry Sample Entry of type SampleEntryBox **/
    void addSampleEntry(std::unique_ptr<SampleEntryBox> sampleEntry);

    /** @brief Get the list of sample entries.
     *  @returns Vector of sample entries of defined type **/
    template<typename T> std::vector<T*> getSampleEntries() const;

    /** @brief Get the sample entry at a particular index from the list.
     *  @param [in] index 1-based index of the sample entry
     *  @returns Sample Entry of defined type **/
    template<typename T> T* getSampleEntry(unsigned int index) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a SampleDescriptionBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    std::vector<std::unique_ptr<SampleEntryBox>> mIndex; ///< Vector of sample entries
};


template<typename T>
std::vector<T*> SampleDescriptionBox::getSampleEntries() const
{
    std::vector<T*> result;
    for (auto& entry : mIndex)
    {
        T* ptr = dynamic_cast<T*>(entry.get());
        if (ptr)
        {
            result.push_back(ptr);
        }
    }
    return result;
}

template<typename T>
T* SampleDescriptionBox::getSampleEntry(const unsigned int index) const
{
    if (mIndex.size() < index || index == 0)
    {
        throw std::runtime_error("SampleDescriptionBox::getSampleEntry invalid sample entry index.");
    }

    T* entryPtr = dynamic_cast<T*>(mIndex.at(index - 1).get());
    if (entryPtr == nullptr)
    {
        throw std::runtime_error("SampleDescriptionBox::getSampleEntry invalid entry type.");
    }
    return entryPtr;
}

#endif /* end of include guard: SAMPLEDESCRIPTIONBOX_HPP */
