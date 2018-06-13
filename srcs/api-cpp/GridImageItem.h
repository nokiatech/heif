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

#pragma once

#include "DerivedImageItem.h"

namespace HEIFPP
{
    class Grid : public HEIFPP::DerivedImageItem
    {
    public:
        Grid(Heif* aHeif);
        Grid(Heif* aHeif, std::uint32_t aColumns, std::uint32_t aRows);
        ~Grid() = default;

        /** Sets the dimensions for the grid
         * @param [in] aColums: The column count for the grid
         * @param [in] aRows: The row count for the grid */
        void resize(std::uint32_t aColumns, std::uint32_t aRows);

        /** Returns the column count of the grid*/
        std::uint32_t columns() const;
        /** Returns the row count of the grid*/
        std::uint32_t rows() const;

        /** Returns the image from the given index
         * @param [in] aColumn: The column index of the image
         * @param [in] aRow: The row index of the image
         * @param [out] aImage: The image from the given index
         * @return Result: Possible error code */
        Result getImage(std::uint32_t aColumn, std::uint32_t aRow, ImageItem*& aImage);

        /** Sets an image to the given index
         * @param [in] aColumn: The column index of the image
         * @param [in] aRow: The row index of the image
         * @param [in] aImage: The image for the given index
         * @return Result: Possible error code */
        Result setImage(std::uint32_t column, std::uint32_t aRow, ImageItem* aImage);

        /** Removes an image from the grid
         * @param [in] aImage: Image to be removed */
        Result removeImage(ImageItem* aImage) override;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        ImageItem* getImage(std::uint32_t aColumn, std::uint32_t aRow);
        const ImageItem* getImage(std::uint32_t aColumn, std::uint32_t aRow) const;

        std::uint32_t mColumns;
        std::uint32_t mRows;

    private:
        Grid& operator=(const Grid&) = delete;
        Grid(const Grid&)            = delete;
        Grid(Grid&&)                 = delete;
        Grid()                       = delete;
    };
}  // namespace HEIFPP
