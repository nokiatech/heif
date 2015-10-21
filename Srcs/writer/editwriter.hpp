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

#ifndef EDITWRITER_HPP
#define EDITWRITER_HPP

#include "isomediafile.hpp"

class EditBox;
class EditListBox;

/**
 * @brief Write EditListBox based on writer input configuration
 */
class EditWriter
{
public:
    /**
     * @brief EditWriter constructor
     * @param clockTicks Timescale from the Movie Header Box, number of time units passing in one second.
     * @param config     EditList input configuration
     */
    EditWriter (std::uint32_t clockTicks, const IsoMediaFile::EditList& config);
    virtual ~EditWriter () = default;

    /**
     * @brief Write EditList based on the input configuration
     * @param [out] editBox EditBox where the EditListBox is set
     */
    void editWrite(EditBox& editBox);

private:
    IsoMediaFile::EditList mEditList; ///< Writer input configuration
    std::uint32_t mClockTicks;        ///< Timescale from the Movie Header Box

    /**
     * @brief Add edit list entry "empty"
     * @param [out] editListBox EditListBox where the unit is added
     * @param [in]  editUnit    Edit unit input configuration
     */
    void writeEmpty(EditListBox* editListBox, const IsoMediaFile::EditUnit& editUnit) const;

    /**
     * @brief Add edit list entry "dwell"
     * @param [out] editListBox EditListBox where the unit is added
     * @param [in]  editUnit    Edit unit input configuration
     */
    void writeDwell(EditListBox* editListBox, const IsoMediaFile::EditUnit& editUnit) const;

    /**
     * @brief Add edit list entry "shift"
     * @param [out] editListBox EditListBox where the unit is added
     * @param [in]  editUnit    Edit unit input configuration
     */
    void writeShift(EditListBox* editListBox, const IsoMediaFile::EditUnit& editUnit) const;
};

#endif /* end of include guard: EDITWRITER_HPP */
