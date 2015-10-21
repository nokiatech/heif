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

#include "editwriter.hpp"
#include "editbox.hpp"

EditWriter::EditWriter(const std::uint32_t clockTicks, const IsoMediaFile::EditList& config) :
    mEditList(config),
    mClockTicks(clockTicks)
{
}

void EditWriter::editWrite(EditBox& editBox)
{
    // If loop_flag is set then get a edit list box with flag=1
    auto editListBox = std::make_shared<EditListBox>();

    // Decide on which version of the box to use
    editListBox->setVersion(0);

    // Set the flags of the edit list box
    if (mEditList.numb_rept == -1 || mEditList.numb_rept > 0)
    {
        editListBox->setFlags(1);
    }

    for (const auto& editUnit : mEditList.edit_unit)
    {
        if(editUnit.edit_type == "empty")
        {
            writeEmpty(editListBox.get(), editUnit);
        }
        else if(editUnit.edit_type == "dwell")
        {
            writeDwell(editListBox.get(), editUnit);
        }
        else if(editUnit.edit_type == "shift")
        {
            writeShift(editListBox.get(), editUnit);
        }
    }
    editBox.setEditListBox(editListBox);
}

void EditWriter::writeEmpty(EditListBox* editListBox, const IsoMediaFile::EditUnit& editUnit) const
{
    EditListBox::EntryVersion0 editEntry;

    editEntry.mMediaTime = -1;
    editEntry.mSegmentDuration = (editUnit.time_span * mClockTicks) / 1000;
    editEntry.mMediaRateInteger = 1;
    editEntry.mMediaRateFraction = 0;

    editListBox->addEntry(editEntry);
}

void EditWriter::writeDwell(EditListBox* editListBox, const IsoMediaFile::EditUnit& editUnit) const
{
    EditListBox::EntryVersion0 editEntry;

    editEntry.mMediaTime = (editUnit.mdia_time * mClockTicks) / 1000;
    editEntry.mSegmentDuration = (editUnit.time_span * mClockTicks) / 1000;
    editEntry.mMediaRateInteger = 0;
    editEntry.mMediaRateFraction = 0;

    editListBox->addEntry(editEntry);
}

void EditWriter::writeShift(EditListBox* editListBox, const IsoMediaFile::EditUnit& editUnit) const
{
    EditListBox::EntryVersion0 editEntry;

    editEntry.mMediaTime = (editUnit.mdia_time * mClockTicks) / 1000;
    editEntry.mSegmentDuration = (editUnit.time_span * mClockTicks) / 1000;
    editEntry.mMediaRateInteger = 1;
    editEntry.mMediaRateFraction = 0;

    editListBox->addEntry(editEntry);
}
