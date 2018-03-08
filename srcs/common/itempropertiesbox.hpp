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

#ifndef ITEMPROPERTIESBOX_HPP
#define ITEMPROPERTIESBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"
#include "itempropertyassociation.hpp"
#include "itempropertycontainer.hpp"


/** @brief Item Properties Box class.
 * @details 'iprp' box implementation. Enum class PropertyType lists supported
 * Property types. */
class ItemPropertiesBox : public Box
{
public:
    ItemPropertiesBox();
    virtual ~ItemPropertiesBox() = default;

    /** Get an item property from the ItemPropertyContainer by index.
     *  @param [in] index 0-based index to to ItemPropertyContainerBox
     *  @return If found, pointer of type T pointing to the property, otherwise nullptr. */
    const Box* getPropertyByIndex(std::uint32_t index) const;

    /** Get an item property raw box from the ItemPropertyContainer by index.
     *  @param [in] index 0-based index to to ItemPropertyContainerBox
     *  @return Data of the property, including header. An empty vector if property in index was not found. */
    Vector<std::uint8_t> getPropertyDataByIndex(std::uint32_t index) const;

    /**  Item Property and Item Full Property types recognized by ItemPropertiesBox */
    enum class PropertyType
    {
        RAW,   ///< Property of an unrecognized type. It is accessible only as raw data. @see CustomPropertyBox.
        AUXC,  ///< Image properties for auxiliary images. @see AuxiliaryTypeProperty
        AVCC,  ///< AVC configuration item property. @see AvcConfigurationBox
        CLAP,  ///< Clean aperture. @see ImageCleanAperture
        COLR,  ///< Colour information. @see ColourInformationBox
        FREE,  ///< FreeSpaceBox. An empty property which should be ignored.
        HVCC,  ///< HEVC configuration item property. @see HevcConfigurationBox
        IMIR,  ///< Image mirror. @see ImageMirror
        IROT,  ///< Image rotation. @see ImageRotation
        ISPE,  ///< Image spatial extents. @see ImageSpatialExtentsProperty
        JPGC,  ///< JPEG configuration item property. @see JpegConfigurationBox
        PASP,  ///< Pixel aspect ratio. @see PixelAspectRatioBox
        PIXI,  ///< Pixel information. @see PixelInformationProperty
        RLOC   ///< Relative location. @see RelativeLocationProperty
    };

    /** Information about a property associated to an item. Information here comes from both contained boxes
     * ItemPropertyContainer and ItemPropertyAssociation. */
    struct PropertyInfo
    {
        PropertyType type;    ///< Type of the property.
        std::uint32_t index;  ///< Index of the property in the Item Property Container (0-based).
        bool essential;       ///< True if this property is marked as 'essential' meaning the reader is required to process it.
    };

    typedef Vector<PropertyInfo> PropertyInfos;

    /** Get all PropertyInfos for an item.
     * @param [in] itemId Item ID of the item
     * @return PropertyInfo structs for this this itemId. An empty vector if the item has no properties, or if the
     *         item ID does not exist). */
    PropertyInfos getItemProperties(std::uint32_t itemId) const;

    /** Find property index based on item id and property type.
     * @param [in] type Type of the property to find
     * @param [in] itemId Item id of the associated item.
     * @return 0 if not found, 1-based index otherwise */
    std::uint32_t findPropertyIndex(PropertyType type, std::uint32_t itemId) const;

    /** Add a new property
     * @param [in] box Property or FullProperty to add.
     * @param [in] itemIds Item ids to which associate the new property.
     * @param [in] essential Essential flag of the property. Reader is required to process essential properties.
     * @return 1-based index of the added property in the ItemPropertyContainer. */
    std::uint16_t addProperty(std::shared_ptr<Box> box, const Vector<std::uint32_t>& itemIds, bool essential);

    /** Associate an existing property to item ids.
     * @param [in] index Property index in the ItemPropertyContainer
     * @param [in] itemIds Item ids to associate
     * @param [in] essential Essential flag of the property. Reader is required to process essential properties. */
    void associateProperty(std::uint16_t index, const Vector<std::uint32_t>& itemIds, bool essential);

    /** Write box to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** Read box from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    /** ItemPropertyContainer contains properties extending from Properties (Box) and FullProperties (FullBox). */
    ItemPropertyContainer mContainer;

    /** ItemPropertyAssociation boxes contain information about property and item associations. */
    Vector<ItemPropertyAssociation> mAssociationBoxes;

    PropertyType getPropertyType(const Box* property) const;
};

#endif /* ITEMPROPERTIESBOX_HPP */
