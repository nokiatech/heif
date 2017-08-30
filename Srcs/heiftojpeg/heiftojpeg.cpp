#include <stdio.h>
#include <stdlib.h>
#include "hevcimagefilereader.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>
#include <list>

using namespace std;

static int decodeData(ImageFileReaderInterface::DataVector data, Magick::Image *image)
{
    std::string hevcfilename = "tmp.hevc";
    std::string bmpfilename = "tmp.bmp";
    std::ofstream hevcfile(hevcfilename);
    hevcfile.write((char*)&data[0],data.size());
    hevcfile.close();
    system(("ffmpeg -i tmp.hevc -loglevel panic -frames:v 1 -vsync vfr -q:v 1 -y -an " + bmpfilename).c_str());
    *image = Magick::Image(bmpfilename);
    return 1;
}

void processFile(char *filename)
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::GridItem gridItem;
    ImageFileReaderInterface::IdVector gridItemIds;

    reader.initialize(filename);
    const auto& properties = reader.getFileProperties();
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    reader.getItemListByType(contextId, "grid", gridItemIds);
    cout << "found " << gridItemIds.size() << " grid items\n";
    const uint32_t itemId = gridItemIds.at(0);
    cout << "grid item 0 has id " << itemId << "\n";
    gridItem = reader.getItemGrid(contextId, itemId);

    uint8_t rows = gridItem.rowsMinusOne + 1;
    uint8_t cols = gridItem.columnsMinusOne + 1;
    cout << "grid is " << gridItem.outputWidth << "x" << gridItem.outputHeight << " pixels in tiles " << rows << "x" << cols << "\n";

    ImageFileReaderInterface::IdVector tileItemIds;
    reader.getItemListByType(contextId, "master", tileItemIds);
    cout << "found " << tileItemIds.size() << " tile images\n";

    uint16_t rotation;
    const auto props = reader.getItemProperties(contextId, itemId);
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


    HevcImageFileReader::ParameterSetMap parameterSet;
    reader.getDecoderParameterSets(contextId, tileItemIds.at(0), parameterSet);
    std::string codeType = reader.getDecoderCodeType(contextId, tileItemIds.at(0));
    ImageFileReaderInterface::DataVector itemDataWithDecoderParameters;
    ImageFileReaderInterface::DataVector itemData;

    std::vector<Magick::Image> tileImages;
    for (auto& tileItemId: tileItemIds) {
        itemDataWithDecoderParameters.clear();
        itemData.clear();

        if ((codeType == "hvc1") || (codeType == "lhv1")) {
            // VPS (HEVC specific)
            itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), parameterSet.at("VPS").begin(), parameterSet.at("VPS").end());
        }

        if ((codeType == "avc1") || (codeType == "hvc1") || (codeType == "lhv1")) {
            // SPS and PPS
            itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), parameterSet.at("SPS").begin(), parameterSet.at("SPS").end());
            itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), parameterSet.at("PPS").begin(), parameterSet.at("PPS").end());
        } else {
            // No other code types supported
            // throw FileReaderException(FileReaderException::StatusCode::UNSUPPORTED_CODE_TYPE);
        }

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
    image.write("out.jpg");
}

int main(int argc, char *argv[])
{
    Magick::InitializeMagick(*argv);

    if ( argc != 3 ) {
        cout << "usage: heiftojpeg <input_file_name> <output_file_name>";
        return 1;
    }

    char *input_file_name = argv[1];
    char *output_file_name = argv[2];

    cout << "Converting HEIF image " << input_file_name << " to JPEG " << output_file_name << "\n";

    processFile(input_file_name);

    return 0;
}

