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

#ifndef ITEMPROPERTYCONTAINER_HPP
#define ITEMPROPERTYCONTAINER_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Item Property Container class
 * @details 'ipco' box implementation.
 * */
class ItemPropertyContainer : public Box
{
public:
    ItemPropertyContainer();
    virtual ~ItemPropertyContainer() = default;

    /**
     * Get Property or FullProperty (here Box or FullBox)
     * @param [in] index 0-based index of the item.
     * @return Base class pointer to the property. nullptr if index is invalid.
     */
    const Box* getProperty(size_t index) const;

    /**
     * Get Property or FullProperty (here Box or FullBox)
     * @param [in] index 0-based index of the item.
     * @return Base class pointer to the property. nullptr if index is invalid.
     */
    Box* getProperty(size_t index);

    /**
     * Add a Property or FullProperty
     * @param [in] box Pointer to the Box to add
     * @return Amount of properties after the operation. The new property is added as the last one, so the 0-based
     *         index of it is (return value - 1).
     */
    std::uint16_t addProperty(std::shared_ptr<Box> box);

    /** Serialize box data to the ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& bitstream) const;

    /** Deserialize box data from the ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& bitstream);

private:
    /**
     * Boxes derived from ItemProperty or ItemFullProperty
     * @todo Preferably unique_ptr should be used here.
     */
    Vector<std::shared_ptr<Box>> mProperties;
};

#endif /* ITEMPROPERTYCONTAINER_HPP */
