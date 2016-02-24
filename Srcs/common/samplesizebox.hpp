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

#ifndef SAMPLESIZEBOX_HPP
#define SAMPLESIZEBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

#include <iostream>
#include <list>
#include <string>

/** @brief SampleSize Box. Extends FullBox.
 *  @details 'stsz' box provides the Sample size information as defined in the ISOBMFF standard.**/
class SampleSizeBox : public FullBox
{
public:
    SampleSizeBox();
    virtual ~SampleSizeBox() = default;

    /** @brief Set default sample size value.
     *  @param [in] sample_size a non-zero sample size means all samples have the same sample size as listed here. **/
    void setSampleSize(std::uint32_t sample_size);

    /** @brief Get default sample size value.
     *  @return [in] sample size value. 0 means all samples have different sizes**/
    std::uint32_t getSampleSize();

    /** @brief Set the number of samples to be listed in this box.
     *  @param [in] sample_count Number of samples. **/
    void setSampleCount(std::uint32_t sample_count);

    /** @brief Get the number of samples to be listed in this box.
     *  @return Number of samples. **/
    std::uint32_t getSampleCount();

    /** @brief Set the sample sizes of the entries as a vector of sizes.
     *  @param [in] sample_sizes vector containing sample sizes. **/
    void setEntrySize(std::vector<uint32_t> sample_sizes);

    /** @brief Get the sample sizes of the entries as a vector of sizes.
     *  @return vector containing sample sizes. **/
    std::vector<uint32_t> getEntrySize();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a SampleSizeBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::uint32_t mSampleSize; ///< Default sample size. Non-zero if all samples have the same sample size.
    std::uint32_t mSampleCount; ///< Number of samples to be listed
    std::vector<std::uint32_t> mEntrySize; ///< Sample sizes of each sample.
};

#endif /* end of include guard: SAMPLESIZEBOX_HPP */
