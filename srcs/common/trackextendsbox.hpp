/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef TRACKEXTENDSBOX_HPP
#define TRACKEXTENDSBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"
#include "moviefragmentsdatatypes.hpp"

/**
 * @brief  Track Extends Box class
 * @details 'trex' box implementation as specified in the ISOBMFF specification.
 */
class TrackExtendsBox : public FullBox
{
public:
    TrackExtendsBox();
    ~TrackExtendsBox() override = default;

    /** @brief Set sample defaults to TrackExtendsBox.
     *  @param [in] FragmentSampleDefaults fragmentSampleDefaults */
    void setFragmentSampleDefaults(const MOVIEFRAGMENTS::SampleDefaults& fragmentSampleDefaults);

    /** @brief Get sample defaults to TrackExtendsBox.
     *  @return FragmentSampleDefaults struct as specified in 8.8.3.1 of ISO/IEC 14496-12:2015(E) */
    const MOVIEFRAGMENTS::SampleDefaults& getFragmentSampleDefaults() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    MOVIEFRAGMENTS::SampleDefaults mSampleDefaults;
};

#endif /* end of include guard: TRACKEXTENDSBOX_HPP */
