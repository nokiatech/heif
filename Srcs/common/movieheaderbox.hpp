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

#ifndef MOVIEHEADERBOX_HPP
#define MOVIEHEADERBOX_HPP

#include "fullbox.hpp"

/**
 * Movie Header Box class
 * @details 'mvhd' box implementation as specified in the ISOBMFF specification.
 * @todo Currently only version zero box is supported. Extend to support version 1 box.
 */
class MovieHeaderBox : public FullBox
{
public:
    MovieHeaderBox();
    virtual ~MovieHeaderBox() = default;

    void setCreationTime(std::uint32_t creationTime);
    std::uint32_t getCreationTime() const;

    void setModificationTime(std::uint32_t modificationTime);
    std::uint32_t getModificationTime() const;

    void setTimeScale(std::uint32_t timeScale);
    std::uint32_t getTimeScale() const;

    void setDuration(std::uint32_t duration);
    std::uint32_t getDuration() const;

    void setNextTrackID(std::uint32_t nextTrackId);
    std::uint32_t getNextTrackID() const;

    std::vector<int32_t> getMatrix() const;

    /**
     * @brief Serialize box data to the BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(BitStream& bitstr);

    /**
     * @brief Deserialize box data from the BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(BitStream& bitstr);

private:
    std::uint32_t mCreationTime;
    std::uint32_t mModificationTime;
    std::uint32_t mTimeScale;
    std::uint32_t mDuration;
    std::vector<std::int32_t> mMatrix;
    std::uint32_t mNextTrackID;
};

#endif /* end of include guard: MOVIEHEADERBOX_HPP */
