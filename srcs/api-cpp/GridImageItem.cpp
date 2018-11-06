/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "GridImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>

using namespace HEIFPP;

GridImageItem::GridImageItem(Heif* aHeif)
    : DerivedImageItem(aHeif, HEIF::FourCC("grid"))
    , mColumns(0)
    , mRows(0)
{
}

GridImageItem::GridImageItem(Heif* aHeif, std::uint32_t aColumns, std::uint32_t aRows)
    : DerivedImageItem(aHeif, HEIF::FourCC("grid"))
{
    HEIF_ASSERT(aColumns > 0);
    HEIF_ASSERT(aRows > 0);
    mColumns = aColumns;
    mRows    = aRows;
    reserveSourceImages(aColumns * aRows);
}

std::uint32_t GridImageItem::columns() const
{
    return mColumns;
}
std::uint32_t GridImageItem::rows() const
{
    return mRows;
}

void GridImageItem::resize(uint32_t aColumns, std::uint32_t aRows)
{
    std::vector<ImageItem*> tmp;

    std::uint32_t mx = aColumns > mColumns ? mColumns : aColumns;
    std::uint32_t my = aRows > mRows ? mRows : aRows;

    tmp.resize(aColumns * aRows);
    for (uint32_t y = 0; y < my; ++y)
    {
        for (uint32_t x = 0; x < mx; ++x)
        {
            tmp[x + y * aColumns] = getImage(x, y);
        }
    }

    reserveSourceImages(aColumns * aRows);
    mColumns = aColumns;
    mRows    = aRows;
    for (uint32_t y = 0; y < my; ++y)
    {
        for (uint32_t x = 0; x < mx; ++x)
        {
            setImage(x, y, tmp[x + y * aColumns]);
        }
    }
}

ImageItem* GridImageItem::getImage(uint32_t aColumn, std::uint32_t aRow)
{
    if ((aColumn < mColumns) && (aRow < mRows))
    {
        return getSourceImage(aColumn + aRow * mColumns);
    }
    return nullptr;
}

const ImageItem* GridImageItem::getImage(uint32_t aColumn, std::uint32_t aRow) const
{
    if ((aColumn < mColumns) && (aRow < mRows))
    {
        return getSourceImage(aColumn + aRow * mColumns);
    }
    return nullptr;
}

Result GridImageItem::getImage(uint32_t aColumn, std::uint32_t aRow, ImageItem*& aImage)
{
    if (aColumn >= mColumns || aRow >= mRows)
    {
        return Result::INDEX_OUT_OF_BOUNDS;
    }
    aImage = getSourceImage(aColumn + aRow * mColumns);
    return Result::OK;
}

Result GridImageItem::setImage(uint32_t aColumn, std::uint32_t aRow, ImageItem* aImage)
{
    if (aColumn >= mColumns || aRow >= mRows)
    {
        return Result::INDEX_OUT_OF_BOUNDS;
    }
    setSourceImage(aColumn + aRow * mColumns, aImage);
    return Result::OK;
}

Result GridImageItem::removeImage(ImageItem* aImage)
{
    // removes ALL references to aImage.
    if (true == setSourceImage(aImage, nullptr))
    {
        return Result::OK;
    }
    return Result::INVALID_HANDLE;
}

HEIF::ErrorCode GridImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = DerivedImageItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    HEIF::Grid grid;
    error = aReader->getItem(aId, grid);
    if (HEIF::ErrorCode::OK != error)
        return error;
    if ((grid.outputWidth != width()) || (grid.outputHeight != height()))
    {
        return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
    }
    mColumns = grid.columns;
    mRows    = grid.rows;
    return error;
}

HEIF::ErrorCode GridImageItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    HEIF::Grid grid;
    grid.outputWidth    = width();
    grid.outputHeight   = height();
    grid.columns        = mColumns;
    grid.rows           = mRows;
    std::uint32_t count = mColumns * mRows;
    if (count != getSourceImageCount())
    {
        return HEIF::ErrorCode::INVALID_REFERENCE_COUNT;
    }
    HEIF::Array<HEIF::ImageId> ids(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        ImageItem* image = getSourceImage(i);
        if (image == nullptr)
        {
            return HEIF::ErrorCode::INVALID_ITEM_ID;
        }
        if (image->getId() == Heif::InvalidItem)
        {
            error = image->save(aWriter);
            if (HEIF::ErrorCode::OK != error)
                return error;
        }
        ids[i] = image->getId();
    }
    grid.imageIds = ids;
    HEIF::ImageId newId;
    error = aWriter->addDerivedImageItem(grid, newId);
    setId(newId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    return DerivedImageItem::save(aWriter);
}
