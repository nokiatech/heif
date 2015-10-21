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

#ifndef WRITERCONSTANTS_HPP
#define WRITERCONSTANTS_HPP

#include <string>

// Handler types
const std::string EXIF_HANDLER = "exif"; ///< 4CC identifier of 'Exif' metadata handler
const std::string PICT_HANDLER = "pict"; ///< 4CC identifier of 'pict' image handler
const std::string VIDE_HANDLER = "vide"; ///< 4CC identifier of 'vide' video track handler
const std::string XML_HANDLER = "xml1"; ///< 4CC identifier of 'xml1' metadata handler

// Codec types
const std::string HVC1_CODEC = "hvc1"; ///< 4CC identifier of 'hvc1' codec type

// Encapsulation types
const std::string META_ENCAPSULATION = "meta"; ///< 4CC identifier of MetaBox encapsulation
const std::string TRAK_ENCAPSULATION = "trak"; ///< 4CC identifier of TrackBox encapsulation

// Item types in iinf box
const std::string EXIF_ITEM_TYPE = "Exif"; ///< 4CC identifier of' Exif' metadata item type in ItemInfoEntry
const std::string GRID_ITEM_TYPE = "grid"; ///< 4CC identifier of grid item type in ItemInfoEntry
const std::string HVC1_ITEM_TYPE = "hvc1"; ///< 4CC identifier of 'hvc1' image item type in ItemInfoEntry
const std::string OVERLAY_ITEM_TYPE = "iovl"; ///< 4CC identifier of overlay item type in ItemInfoEntry
const std::string XML_ITEM_TYPE = "xml1"; ///< 4CC identifier of XML metadata item type in ItemInfoEntry

#endif
