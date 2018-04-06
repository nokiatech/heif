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
Grid::Grid(Heif* aHeif)
    : DerivedImageItem(aHeif, HEIF::FourCC("grid"))
    , mColumns(0)
    , mRows(0)
{
}

Grid::Grid(Heif* aHeif, uint32_t aColumns, uint32_t aRows)
    : DerivedImageItem(aHeif, HEIF::FourCC("grid"))
{
    HEIF_ASSERT(aColumns > 0);
    HEIF_ASSERT(aRows > 0);
    mColumns = aColumns;
    mRows    = aRows;
    reserveSourceImages(aColumns * aRows);
}

uint32_t Grid::columns() const
{
    return mColumns;
}
uint32_t Grid::rows() const
{
    return mRows;
}

void Grid::resize(uint32_t aColumns, uint32_t aRows)
{
    uint32_t mx, my;
    std::vector<ImageItem*> tmp;

    mx = aColumns > mColumns ? mColumns : aColumns;
    my = aRows > mRows ? mRows : aRows;

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

ImageItem* Grid::getImage(uint32_t aColumn, uint32_t aRow)
{
    if ((aColumn < mColumns) && (aRow < mRows))
    {
        return getSourceImage(aColumn + aRow * mColumns);
    }
    return nullptr;
}

const ImageItem* Grid::getImage(uint32_t aColumn, uint32_t aRow) const
{
    if ((aColumn < mColumns) && (aRow < mRows))
    {
        return getSourceImage(aColumn + aRow * mColumns);
    }
    return nullptr;
}

Result Grid::getImage(uint32_t aColumn, uint32_t aRow, ImageItem*& aImage)
{
    if (aColumn >= mColumns || aRow >= mRows)
    {
        return Result::INDEX_OUT_OF_BOUNDS;
    }
    aImage = getSourceImage(aColumn + aRow * mColumns);
    return Result::OK;
}

Result Grid::setImage(uint32_t aColumn, uint32_t aRow, ImageItem* aImage)
{
    if (aColumn >= mColumns || aRow >= mRows)
    {
        return Result::INDEX_OUT_OF_BOUNDS;
    }
    setSourceImage(aColumn + aRow * mColumns, aImage);
    return Result::OK;
}

Result Grid::removeImage(ImageItem* aImage)
{
    // removes ALL references to aImage.
    if (true == setSourceImage(aImage, nullptr))
    {
        return Result::OK;
    }
    return Result::INVALID_HANDLE;
}

HEIF::ErrorCode Grid::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = DerivedImageItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    HEIF::Grid grid;
    error = aReader->getItem(aId, grid);
    if (HEIF::ErrorCode::OK != error)
        return error;
    HEIF_ASSERT(grid.outputWidth == width());
    HEIF_ASSERT(grid.outputHeight == height());
    if ((grid.outputWidth != width()) || (grid.outputHeight != height()))
    {
        return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
    }
    mColumns = grid.columns;
    mRows    = grid.rows;
    return error;
}

HEIF::ErrorCode Grid::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    HEIF::Grid grid;
    grid.outputWidth  = width();
    grid.outputHeight = height();
    grid.columns      = mColumns;
    grid.rows         = mRows;
    uint32_t count    = mColumns * mRows;
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
    error         = aWriter->addDerivedImageItem(grid, mId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    return DerivedImageItem::save(aWriter);
}
