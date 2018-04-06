package com.nokia.heif.example;

import com.nokia.heif.ErrorHandler;
import com.nokia.heif.Exception;
import com.nokia.heif.GridImageItem;
import com.nokia.heif.HEIF;
import com.nokia.heif.HEVCImageItem;
import com.nokia.heif.ImageItem;
import com.nokia.heif.Size;

class HEIFExample
{
    static void loadSingleImage()
    {
        String filename = "something/something.heic";
        // Create an instance of the HEIF library,
        HEIF heif = new HEIF();
        try
        {
            // Load the file
            heif.load(filename);

            // Get the primary image
            ImageItem primaryImage = heif.getPrimaryImage();

            // Check the type, assuming that it's a HEVC image
            if (primaryImage instanceof HEVCImageItem)
            {
                HEVCImageItem hevcImageItem = (HEVCImageItem)primaryImage;
                byte[] decoderConfig = hevcImageItem.getDecoderConfig().getConfig();
                byte[] imageData = hevcImageItem.getItemDataAsArray();
                // Feed the data to a decoder
            }

        }
        // All exceptions thrown by the HEIF library are of the same type
        // Check the error code to see what happened
        catch (Exception e)
        {
            e.printStackTrace();
        }

    }

    static void createAndSaveAFile()
    {
        // These should contain the encoded image data and the corresponding decoder config data
        int width = 640;
        int height = 480;
        // Sizes are just placeholders
        byte[] decoderConfig = new byte[1024];
        byte[] imageData = new byte[50000];

        // Filename should have the full path
        String filename = "something/something.heic";

        // Create an instance of the HEIF library,
        HEIF heif = new HEIF();
        try
        {
            // This example assumes that the data is HEVC
            // The constructor requires the HEIF instance, the size of the image,
            // the decoder config data and the image data
            HEVCImageItem imageItem = new HEVCImageItem(heif, new Size(width, height),
                                                        decoderConfig, imageData);
            // Every HEIF image should have a primary image
            heif.setPrimaryImage(imageItem);

            // The brands need to be set
            heif.setMajorBrand(HEIF.BRAND_MIF1);
            heif.addCompatibleBrand(HEIF.BRAND_HEIC);

            // And we save the file
            heif.save(filename);

        }
        // All exceptions thrown by the HEIF library are of the same type
        // Check the error code to see what happened
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    static void loadGridImage()
    {
        String filename = "something/grid_something.heic";
        // Create an instance of the HEIF library,
        HEIF heif = new HEIF();
        try
        {
            // Load the file
            heif.load(filename);

            // Get the primary image
            ImageItem primaryImage = heif.getPrimaryImage();

            // Check the type, assuming that it's a Grid image
            if (primaryImage instanceof GridImageItem)
            {
                GridImageItem gridImageItem = (GridImageItem) primaryImage;
                // Go through the grid
                for (int rowIndex = 0; rowIndex < gridImageItem.getRowCount(); rowIndex++)
                {
                    for (int columnIndex = 0; columnIndex < gridImageItem.getColumnCount(); columnIndex++)
                    {
                        // We assume that the image items are HEVC
                        HEVCImageItem hevcImageItem = (HEVCImageItem) gridImageItem.getImage(columnIndex, rowIndex);
                        byte[] decoderConfig = hevcImageItem.getDecoderConfig().getConfig();
                        byte[] imageData = hevcImageItem.getItemDataAsArray();
                        // Feed the data to a decoder
                    }
                }
            }
        }
        // All exceptions thrown by the HEIF library are of the same type
        // Check the error code to see what happened
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    static void saveGridImage()
    {
        // These should contain the encoded image data and the corresponding decoder config data
        int width = 640;
        int height = 480;
        // Sizes are just placeholders
        byte[] decoderConfig = new byte[1024];
        byte[] imageData = new byte[50000];

        String filename = "something/grid_something.heic";
        // Create an instance of the HEIF library,
        HEIF heif = new HEIF();
        try
        {
            final int columnCount = 3;
            final int rowCount = 2;
            // As an example, create a 3 x 2 grid
            GridImageItem gridImageItem = new GridImageItem(heif, columnCount, rowCount,
                                                            new Size (columnCount * width,
                                                                      rowCount * height));
            for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
            {
                for (int columnIndex = 0; columnIndex < columnCount; columnIndex++)
                {
                    // Create image items for each grid, as an example we're using the same
                    // data for each image
                    HEVCImageItem hevcImageItem = new HEVCImageItem(heif,
                                                                    new Size(640, 480),
                                                                    decoderConfig, imageData);
                    // Set the image to the correct location
                    gridImageItem.setImage(columnIndex, rowIndex, hevcImageItem);
                }
            }

            // Set the grid as a primary image
            heif.setPrimaryImage(gridImageItem);

            // The brands need to be set
            heif.setMajorBrand(HEIF.BRAND_MIF1);
            heif.addCompatibleBrand(HEIF.BRAND_HEIC);

            // And we save the file
            heif.save(filename);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

    }
}