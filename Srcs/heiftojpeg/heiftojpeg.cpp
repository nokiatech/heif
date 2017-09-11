#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>
#include "hevcimagefilereader.hpp"
#include <list>

using namespace std;

static int VERBOSE = 0;
static int MAX_SIZE = -1;

static void decodeData(ImageFileReaderInterface::DataVector data, Magick::Image *image)
{
    string hevcFileName = "tmp.hevc";
    string bmpFileName = "tmp.bmp";
    ofstream hevcFile(hevcFileName);
    hevcFile.write((char*)&data[0],data.size());
    hevcFile.close();
    system(("ffmpeg -i " + hevcFileName + " -loglevel panic -frames:v 1 -vsync vfr -q:v 1 -y -an " + bmpFileName).c_str());
    *image = Magick::Image(bmpFileName);
    remove(hevcFileName.c_str());
    remove(bmpFileName.c_str());
}

static void addExif(ImageFileReaderInterface::DataVector exifData, string fileName)
{
    string exifFileName = "tmp.exif";
    ofstream exifFile(exifFileName);
    exifFile.write((char*)&exifData[0], exifData.size());
    exifFile.close();
    if (VERBOSE) {
        cout << "wrote exif to " << exifFileName << "\n";
    }
    system(("exiftool -m -overwrite_original " + fileName + " -tagsFromFile " + exifFileName).c_str());
    remove(exifFileName.c_str());
}

static void processFile(char *filename, char *outputFileName)
{
    HevcImageFileReader reader;
    reader.initialize(filename);

    const auto& properties = reader.getFileProperties();
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    ImageFileReaderInterface::IdVector gridItemIds;
    reader.getItemListByType(contextId, "grid", gridItemIds);
    if (VERBOSE) {
        cout << "found " << gridItemIds.size() << " grid items\n";
    }

    const uint32_t gridItemId = gridItemIds.at(0);
    ImageFileReaderInterface::GridItem gridItem;
    gridItem = reader.getItemGrid(contextId, gridItemId);

    if (VERBOSE) {
        cout << "grid is " << gridItem.outputWidth << "x" << gridItem.outputHeight << " pixels in tiles " << gridItem.rowsMinusOne << "x" << gridItem.columnsMinusOne << "\n";
    }

    ImageFileReaderInterface::IdVector exifItemIds;
    ImageFileReaderInterface::DataVector exifData;
    reader.getReferencedToItemListByType(contextId, gridItemId, "cdsc", exifItemIds);
    if (VERBOSE) {
        cout << "found " << exifItemIds.size() << " cdsc items\n";
    }
    reader.getItemData(contextId, exifItemIds.at(0), exifData);
    if (VERBOSE) {
        cout << "read " << exifData.size() << " bytes of exif data\n";
    }

    ImageFileReaderInterface::IdVector tileItemIds;
    reader.getItemListByType(contextId, "master", tileItemIds);
    if (VERBOSE) {
        cout << "found " << tileItemIds.size() << " tile images\n";
    }

    uint16_t rotation;
    const auto props = reader.getItemProperties(contextId, gridItemId);
    for (const auto& property : props)
    {
        if (property.type == ImageFileReaderInterface::ItemPropertyType::IROT) {
            rotation = reader.getPropertyIrot(contextId, property.index).rotation;
            if (VERBOSE) {
                cout << "IROT " << rotation << "\n";
            }
        }
    }

    // Always reuse the parameter set from the first tile, sometimes tile 7 or 8 is corrupted
    HevcImageFileReader::ParameterSetMap parameterSet;
    reader.getDecoderParameterSets(contextId, tileItemIds.at(0), parameterSet);
    string codeType = reader.getDecoderCodeType(contextId, tileItemIds.at(0));
    ImageFileReaderInterface::DataVector parametersData;
    if ((codeType == "hvc1") || (codeType == "lhv1")) {
        // VPS (HEVC specific)
        parametersData.insert(parametersData.end(), parameterSet.at("VPS").begin(), parameterSet.at("VPS").end());
    }

    if ((codeType == "avc1") || (codeType == "hvc1") || (codeType == "lhv1")) {
        // SPS and PPS
        parametersData.insert(parametersData.end(), parameterSet.at("SPS").begin(), parameterSet.at("SPS").end());
        parametersData.insert(parametersData.end(), parameterSet.at("PPS").begin(), parameterSet.at("PPS").end());
    } else {
        // No other code types supported
        // throw FileReaderException(FileReaderException::StatusCode::UNSUPPORTED_CODE_TYPE);
    }

    ImageFileReaderInterface::DataVector itemDataWithDecoderParameters;
    ImageFileReaderInterface::DataVector itemData;

    vector<Magick::Image> tileImages;
    for (auto& tileItemId: tileItemIds) {
        itemDataWithDecoderParameters.clear();
        itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), parametersData.begin(), parametersData.end());

        itemData.clear();
        reader.getItemData(contextId, tileItemId, itemData);

        // +1 comes from skipping first zero after decoder parameters
        itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), itemData.begin() + 1, itemData.end());

        Magick::Image image;
        decodeData(itemDataWithDecoderParameters, &image);
        tileImages.push_back(image);
    }

    Magick::Montage montageOptions;
    montageOptions.tile("8x6");
    montageOptions.geometry("512x512");
    list<Magick::Image> montage;
    Magick::montageImages(&montage, tileImages.begin(), tileImages.end(), montageOptions);
    Magick::Image image = montage.front();
    image.magick("JPEG");
    image.crop(Magick::Geometry(gridItem.outputWidth, gridItem.outputHeight));

    if (MAX_SIZE > 0) {
        double scaleFactor;
        if (gridItem.outputWidth > gridItem.outputHeight) {
            scaleFactor = (double)MAX_SIZE / (double)gridItem.outputWidth;
        } else {
            scaleFactor = (double)MAX_SIZE / (double)gridItem.outputHeight;
        }

        if (scaleFactor < 1) {
            image.zoom(Magick::Geometry(scaleFactor * gridItem.outputWidth, scaleFactor * gridItem.outputHeight));
        }
    }
    image.rotate(-rotation);
    image.write(outputFileName);

    addExif(exifData, outputFileName);
}

int usage()
{
    fprintf(stderr, "Usage: heiftojpeg [-v] [-s <max_dimension>] <input.heic> <output.jpg>\n");
    return 1;
}

int main(int argc, char *argv[])
{
    Magick::InitializeMagick(*argv);

    char *inputFileName = NULL;
    char *outputFileName = NULL;
    int index;
    int c;

    opterr = 0;
    while ((c = getopt (argc, argv, "vs:")) != -1) {
        switch (c) {
            case 'v':
                VERBOSE = 1;
                break;
            case 's':
                MAX_SIZE = atoi(optarg);
                break;
            case '?':
                return usage();
            default:
                abort ();
        }
    }

    for (index = optind; index < argc; index++) {
        if (inputFileName == NULL) {
            inputFileName = argv[index];
        } else if (outputFileName == NULL) {
            outputFileName = argv[index];
        } else {
            return usage();
        }
    }

    if (VERBOSE) {
        cout << "Converting HEIF image " << inputFileName << " to JPEG " << outputFileName << "\n";
    }

    processFile(inputFileName, outputFileName);

    return 0;
}

