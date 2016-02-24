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

#ifndef MEDIAINFORMATIONBOX_HPP
#define MEDIAINFORMATIONBOX_HPP

#include "bitstream.hpp"
#include "datainformationbox.hpp"
#include "sampletablebox.hpp"
#include "videomediaheaderbox.hpp"

/** @brief Media Information Box class. Extends from Box.
 *  @details 'minf' box contains the information needed to access media samples and initialize decoders as defined in the ISOBMFF and HEIF standards. **/
class MediaInformationBox : public Box
{
public:
    MediaInformationBox();
    virtual ~MediaInformationBox() = default;

    /** @brief Get VideoMediaHeader box.
     *  @return Reference to VideoMediaHeaderBox data structure **/
    VideoMediaHeaderBox& getVideoMediaHeaderBox();

    /** @brief Get DataInformationBox.
     *  @return Reference to DataInformationBox data structure **/
    DataInformationBox& getDataInformationBox();

    /** @brief Get SampleTableBox.
     *  @return Reference to SampleTableBox data structure **/
    SampleTableBox& getSampleTableBox();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a MediaInformationBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    VideoMediaHeaderBox mVideoMediaHeaderBox; ///< Video media header box
    DataInformationBox mDataInformationBox; ///< Data information box
    SampleTableBox mSampleTableBox; ///< Sample Table box
};

#endif /* end of include guard: MEDIAINFORMATIONBOX_HPP */
