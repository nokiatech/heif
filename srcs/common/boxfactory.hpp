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

#ifndef BOXFACTORY_HPP
#define BOXFACTORY_HPP

#include "bbox.hpp"

/**
 * The BoxFactory class can be used to create new box objects, based on the FourCC code
 * which is known (it was already read from input bitstream).
 *
 * Currently it supports only item property boxes.
 */
class BoxFactory
{
public:
    BoxFactory();

    /**
     * @brief makeNewBox Create a new box objects of wanted class.
     * @param boxType FourCC code of the new box.
     * @return New box of boxType type. Nullptr is returned if the box type was not recognized.
     */
    std::shared_ptr<Box> makeNewBox(FourCCInt boxType);

private:
    class BoxWrapperBase
    {
    public:
        BoxWrapperBase()                          = default;
        virtual FourCCInt getType() const         = 0;
        virtual std::shared_ptr<Box> makeNewBox() = 0;
        virtual ~BoxWrapperBase()                 = default;
    };

    template <class T>
    class BoxWrapper;

    std::map<FourCCInt, std::shared_ptr<BoxWrapperBase>> mBoxWrapperMap;
};

#endif
