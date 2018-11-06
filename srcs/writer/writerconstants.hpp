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

#ifndef WRITERCONSTANTS_HPP
#define WRITERCONSTANTS_HPP

#include "customallocator.hpp"
#include "fourccint.hpp"

// Handler types
const FourCCInt PICT_HANDLER = "pict";  ///< 4CC identifier of 'pict' image handler
const FourCCInt AUXV_HANDLER = "auxv";  ///< 4CC identifier of 'auxv' auxiliary image sequence track
const FourCCInt VIDE_HANDLER = "vide";  ///< 4CC identifier of 'vide' video track handler
const FourCCInt SOUN_HANDLER = "soun";  ///< 4CC identifier of 'soun' audio track handler
const FourCCInt XML_HANDLER  = "xml1";  ///< 4CC identifier of 'xml1' metadata handler

// Codec types
const FourCCInt HVC1_CODEC = "hvc1";  ///< 4CC identifier of 'hvc1' codec type

// Item types in iinf box
const FourCCInt EXIF_ITEM_TYPE    = "Exif";  ///< 4CC identifier of' Exif' metadata item type in ItemInfoEntry
const FourCCInt GRID_ITEM_TYPE    = "grid";  ///< 4CC identifier of grid item type in ItemInfoEntry
const FourCCInt AVC1_ITEM_TYPE    = "avc1";  ///< 4CC identifier of 'avc1' image item type in ItemInfoEntry
const FourCCInt HVC1_ITEM_TYPE    = "hvc1";  ///< 4CC identifier of 'hvc1' image item type in ItemInfoEntry
const FourCCInt OVERLAY_ITEM_TYPE = "iovl";  ///< 4CC identifier of overlay item type in ItemInfoEntry
const FourCCInt XML_ITEM_TYPE     = "xml1";  ///< 4CC identifier of XML metadata item type in ItemInfoEntry

// Track References
const FourCCInt THUMB_TREF_TYPE = "thmb";  ///< 4CC identifier of thumbnail track reference
const FourCCInt AUXL_TREF_TYPE  = "auxl";  ///< 4CC identifier of auxiliry track reference

// Grouping Types
const FourCCInt STMI_GROUP_TYPE = "stmi";  ///< 4CC identifier of Sample To Metadata Item grouping

#endif
