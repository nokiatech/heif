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

#ifndef MOVIEHEADERBOX_HPP
#define MOVIEHEADERBOX_HPP

#include "customallocator.hpp"
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

    void setCreationTime(std::uint64_t creationTime);
    std::uint64_t getCreationTime() const;

    void setModificationTime(std::uint64_t modificationTime);
    std::uint64_t getModificationTime() const;

    void setTimeScale(std::uint32_t timeScale);
    std::uint32_t getTimeScale() const;

    void setDuration(std::uint64_t duration);
    std::uint64_t getDuration() const;

    void setNextTrackID(std::uint32_t nextTrackId);
    std::uint32_t getNextTrackID() const;

    void setMatrix(const Vector<int32_t>& matrix);
    const Vector<int32_t>& getMatrix() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint64_t mCreationTime;
    std::uint64_t mModificationTime;
    std::uint32_t mTimeScale;
    std::uint64_t mDuration;
    Vector<std::int32_t> mMatrix;
    std::uint32_t mNextTrackID;
};

#endif /* end of include guard: MOVIEHEADERBOX_HPP */
