#include "hevcimagefilereader.hpp"
#include <iostream>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

using namespace std;

void getData(char *filename, ImageFileReaderInterface::DataVector& data)
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::IdVector itemIds;

    cout << "reading image data from " << filename << "...\n";
    reader.initialize(filename);
    const auto& properties = reader.getFileProperties();

    // Verify that the file has one or several images in the MetaBox
    if (not (properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasSingleImage) ||
        properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageCollection)))
    {
        cout << "No image property found.\n";
        return;
    }


    cout << "getting master image id...\n";
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
    reader.getItemListByType(contextId, "master", itemIds);
    const uint32_t masterId = itemIds.at(0);

    cout << "master image found with id " << masterId << ", getting image data...\n";
    reader.getItemDataWithDecoderParameters(contextId, masterId, data);
}

int main(int argc, char *argv[])
{

    if ( argc != 3 ) {
        cout << "usage: heiftojpeg <input_file_name> <output_file_name>";
        return 1;
    }

    char *input_file_name = argv[1];
    char *output_file_name = argv[2];

    cout << "Converting HEIF image " << input_file_name << " to JPEG " << output_file_name << "\n";

    ImageFileReaderInterface::DataVector data;
    getData(input_file_name, data);

    cout << "item data received with size " << data.size() << "\n";

    return 0;
}

