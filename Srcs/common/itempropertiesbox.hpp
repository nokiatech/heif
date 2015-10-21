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

#ifndef ITEMPROPERTIESBOX_HPP
#define ITEMPROPERTIESBOX_HPP

#include "fullbox.hpp"
#include "itempropertyassociation.hpp"
#include "itempropertycontainer.hpp"

#include <vector>

/** @brief Item Properties Box class.
 * @details 'iprp' box implementation as specified in the HEIF specification. Enum class PropertyType lists supported
 * Property types. */
class ItemPropertiesBox : public FullBox
{
public:
    ItemPropertiesBox();
    virtual ~ItemPropertiesBox() = default;

    /** Get an item property from the ItemPropertyContainer by index.
     *  @param [in] index 0-based index to to ItemPropertyContainerBox
     *  @return If found, pointer of type T pointing to the property, otherwise nullptr. */
    template<typename T> const T* getPropertyByIndex(std::uint32_t index) const;

    /**  Item Property and Item Full Property types recognized by ItemPropertiesBox */
    enum class PropertyType
    {
        UNKNOWN, ///< Not handled/recognized property which was read. Accessing it will cause failure.
        AUXC,    ///< Image properties for auxiliary images. @see AuxiliaryTypeProperty
        CLAP,    ///< Clean aperture. @see ImageCleanAperture
        HVCC,    ///< HEVC configuration item property. @see HevcConfigurationBox
        IROT,    ///< Image rotation. @see ImageRotation
        ISPE,    ///< Image spatial extents. @see ImageSpatialExtentsProperty
        RLOC     ///< Relative location. @see RelativeLocationProperty
    };

    /** Information about a property associated to an item. Information here comes from both contained boxes
     * ItemPropertyContainer and ItemPropertyAssociation. */
    struct PropertyInfo
    {
        PropertyType type;   ///< Type of the property
        std::uint32_t index; ///< Index of the property in the Item Property Container
        bool essential;      ///< True if this property is marked as 'essential' meaning the reader is required to process it.
    };

    typedef std::vector<PropertyInfo> PropertyInfos;

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
     * @param [in] essential Essential flag of the property. Reader is required to process essential properties. */
    void addProperty(std::shared_ptr<Box> box, const std::vector<std::uint32_t>& itemIds, bool essential);

    /** Associate an existing property to item ids.
     * @param [in] index Property index in the ItemPropertyContainer
     * @param [in] itemIds Item ids to associate
     * @param [in] essential Essential flag of the property. Reader is required to process essential properties. */
    void associateProperty(unsigned int index, const std::vector<std::uint32_t>& itemIds, bool essential);

    /** Write box to BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** Read box from BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    /** ItemPropertyContainer contains properties extending from Properties (Box) and FullProperties (FullBox). */
    ItemPropertyContainer mContainer;

    /** ItemPropertyAssociation contains information about property and item associations.
     * @todo This should be a vector, to support boxes with different flags and versions in headers. Currently only
     * one box is supported. */
    ItemPropertyAssociation mAssociations;

    PropertyType getPropertyType(const Box* property) const;
};

template<typename T>
const T* ItemPropertiesBox::getPropertyByIndex(const std::uint32_t index) const
{
    return dynamic_cast<const T*>(mContainer.getProperty(index));
}

#endif /* ITEMPROPERTIESBOX_HPP */
