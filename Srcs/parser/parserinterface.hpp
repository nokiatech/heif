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

#ifndef PARSER_INTERFACE_HPP
#define PARSER_INTERFACE_HPP

#include <cstdint>
#include <string>
#include <vector>

/// Interface for video bitstream parsers.
class ParserInterface
{
public:
    struct PicOrder
    {
        unsigned int mDecodeIdx;
        int mDisplayIdx;
    };

    static bool compareDisplayOrder(const PicOrder& a, const PicOrder& b) { return a.mDisplayIdx < b.mDisplayIdx; }
    static bool compareDecodeOrder(const PicOrder& a, const PicOrder& b) { return a.mDecodeIdx < b.mDecodeIdx; }

    /// Information about a video bitstream AccessUnit.
    struct AccessUnit
    {
        std::vector<std::vector<std::uint8_t>> mVpsNalUnits;
        std::vector<std::vector<std::uint8_t>> mSpsNalUnits;
        std::vector<std::vector<std::uint8_t>> mPpsNalUnits;
        std::vector<std::vector<std::uint8_t>> mNalUnits;     ///< Non-parameter set NAL units
        std::vector<unsigned int> mRefPicIndices;
        unsigned int mPicIndex;      ///< Picture number in decoding order
        unsigned int mDisplayOrder;  ///< Picture number in display order
        int mPoc;                    ///< Picture Order Count
        unsigned int mPicWidth;
        unsigned int mPicHeight;
        bool mIsScalable;
        bool mIsIntra;
        bool mIsIdr;
        bool mIsCra;
        bool mIsBla;
        bool mPicOutputFlag;
    };

    ParserInterface() = default;
    virtual ~ParserInterface() = default;

    /** Open file
     *  @param [in] fileName Video bitstream file name
     *  @return True if opening file and initialization was successful */
    virtual bool openFile(const char* fileName) = 0;

    /** Open file @see openFile(const char* fileName) */
    virtual bool openFile(const std::string& fileName) = 0;

    /** Parse next access unit
     * @param [out] accessUnit
     * @return True if there is more accessUnits. */
    virtual bool parseNextAU(AccessUnit& accessUnit) = 0;

    /** @return Number of pictures in the bitstream */
    virtual unsigned int getNumPictures() = 0;
};

#endif
