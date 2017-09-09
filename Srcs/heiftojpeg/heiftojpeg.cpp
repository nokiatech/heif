#include <stdio.h>
#include <stdlib.h>
#include "hevcimagefilereader.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>
#include <list>

using namespace std;

static void decodeData(ImageFileReaderInterface::DataVector data, Magick::Image *image)
{
    std::string hevcfilename = "tmp.hevc";
    std::string bmpfilename = "tmp.bmp";
    std::ofstream hevcfile(hevcfilename);
    hevcfile.write((char*)&data[0],data.size());
    hevcfile.close();
    system(("ffmpeg -i tmp.hevc -loglevel panic -frames:v 1 -vsync vfr -q:v 1 -y -an " + bmpfilename).c_str());
    *image = Magick::Image(bmpfilename);
}

static void addExif(ImageFileReaderInterface::DataVector exifData, std::string fileName)
{
    std::string exifFileName = "tmp.exif";
    std::ofstream exifFile(exifFileName);
    exifFile.write((char*)&exifData[0], exifData.size());
    exifFile.close();
    cout << "wrote exif to " << exifFileName << "\n";
    system(("exiftool " + fileName + " -tagsFromFile " + exifFileName).c_str());
}

static void processFile(char *filename, char *outputFileName)
{
    HevcImageFileReader reader;
    reader.initialize(filename);

    const auto& properties = reader.getFileProperties();
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    ImageFileReaderInterface::IdVector gridItemIds;
    reader.getItemListByType(contextId, "grid", gridItemIds);
    cout << "found " << gridItemIds.size() << " grid items\n";

    const uint32_t gridItemId = gridItemIds.at(0);
    ImageFileReaderInterface::GridItem gridItem;
    gridItem = reader.getItemGrid(contextId, gridItemId);

    cout << "grid is " << gridItem.outputWidth << "x" << gridItem.outputHeight << " pixels in tiles " << gridItem.rowsMinusOne << "x" << gridItem.columnsMinusOne << "\n";

    ImageFileReaderInterface::IdVector exifItemIds;
    ImageFileReaderInterface::DataVector exifData;
    reader.getReferencedToItemListByType(contextId, gridItemId, "cdsc", exifItemIds);
    cout << "found " << exifItemIds.size() << " cdsc items\n";
    reader.getItemData(contextId, exifItemIds.at(0), exifData);
    cout << "read " << exifData.size() << " bytes of exif data\n";

    ImageFileReaderInterface::IdVector tileItemIds;
    reader.getItemListByType(contextId, "master", tileItemIds);
    cout << "found " << tileItemIds.size() << " tile images\n";

    uint16_t rotation;
    const auto props = reader.getItemProperties(contextId, gridItemId);
    for (const auto& property : props)
    {
        cout << "prop ";
        if (property.type == ImageFileReaderInterface::ItemPropertyType::AUXC) {
            cout << "AUXC ";
            cout << reader.getPropertyAuxc(contextId, property.index).auxType;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::AVCC) {
            cout << "AVCC ";
            // cout << reader.getPropertyAvcc(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::CLAP) {
            cout << "CLAP ";
            cout << reader.getPropertyClap(contextId, property.index).widthN;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::HVCC) {
            cout << "HVCC ";
            // cout << reader.getPropertyHvcc(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::IMIR) {
            cout << "IMIR ";
            cout << reader.getPropertyImir(contextId, property.index).horizontalAxis;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::IROT) {
            cout << "IROT ";
            rotation = reader.getPropertyIrot(contextId, property.index).rotation;
            cout << rotation;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::ISPE) {
            cout << "ISPE ";
            // cout << reader.getPropertyIspe(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::LHVC) {
            cout << "LHVC ";
            // cout << reader.getPropertyLhvc(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::LSEL) {
            cout << "LSEL ";
            cout << reader.getPropertyLsel(contextId, property.index).layerId;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::OINF) {
            cout << "OINF ";
            // cout << reader.getPropertyOinf(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::RLOC) {
            cout << "RLOC ";
            cout << reader.getPropertyRloc(contextId, property.index).horizontalOffset;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::TOLS) {
            cout << "TOLS ";
            cout << reader.getPropertyTols(contextId, property.index).targetOlsIndex;
        } else {
            cout << "unknown ";
        }
        cout << "\n";
    }

    // Always reuse the parameter set from the first tile, sometimes tile 7 or 8 is corrupted
    HevcImageFileReader::ParameterSetMap parameterSet;
    reader.getDecoderParameterSets(contextId, tileItemIds.at(0), parameterSet);
    std::string codeType = reader.getDecoderCodeType(contextId, tileItemIds.at(0));
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

    std::vector<Magick::Image> tileImages;
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
    std::list<Magick::Image> montage;
    Magick::montageImages(&montage, tileImages.begin(), tileImages.end(), montageOptions);
    Magick::Image image = montage.front();
    image.magick("JPEG");
    image.crop(Magick::Geometry(gridItem.outputWidth,gridItem.outputHeight));
    image.rotate(-rotation);
    image.write(outputFileName);

    addExif(exifData, outputFileName);
}

int main(int argc, char *argv[])
{
    Magick::InitializeMagick(*argv);

    if ( argc != 3 ) {
        cout << "usage: heiftojpeg <input_file_name> <output_file_name>";
        return 1;
    }

    char *inputFileName = argv[1];
    char *outputFileName = argv[2];

    cout << "Converting HEIF image " << inputFileName << " to JPEG " << outputFileName << "\n";

    processFile(inputFileName, outputFileName);

    return 0;
}

