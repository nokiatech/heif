#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>
#include <list>
#include <chrono>
#include "hevcimagefilereader.hpp"

using namespace std;

static int VERBOSE = 0;
static int MAX_SIZE = -1;

template<typename TimeT = std::chrono::milliseconds>
struct measure
{
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(F&& func, Args&&... args)
    {
        auto start = std::chrono::steady_clock::now();
        std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
        auto duration = std::chrono::duration_cast< TimeT>
                            (std::chrono::steady_clock::now() - start);
        return duration.count();
    }
};

static void decodeData(ImageFileReaderInterface::DataVector data, Magick::Image *image)
{
    char tmpTemplate[] = "/tmp/tmp-decodeData.XXXXXX";
    char *tmpDirName = mkdtemp(tmpTemplate);

    string hevcFileName = tmpDirName;
    hevcFileName += "/tmp.hevc";
    string bmpFileName = tmpDirName;
    bmpFileName += "/tmp.bmp";

    ofstream hevcFile(hevcFileName);
    if (!hevcFile.is_open()) {
        cerr << "could not open " << hevcFileName << " for writing HEVC\n";
        exit(1);
    }
    hevcFile.write((char*)&data[0], data.size());
    if (hevcFile.bad()) {
        cerr << "failed to write " << data.size() << " bytes of HEVC to " << hevcFileName << endl;
        exit(1);
    }
    hevcFile.close();
    if (VERBOSE) {
        cout << "wrote " << data.size() << " bytes of HEVC to " << hevcFileName << endl;
    }
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    int retval = system(("ffmpeg -i " + hevcFileName + " -loglevel panic -frames:v 1 -vsync vfr -q:v 1 -y -an " + bmpFileName).c_str());
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    remove(hevcFileName.c_str());
    if (retval != 0) {
        cerr << "ffmpeg failed with exit code " << retval << endl;
        string rm = "rmdir ";
        rm += tmpDirName;
        system(rm.c_str());
        exit(1);
    }
    if (VERBOSE) {
        cout << "[timing] ffmpeg " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
    }
    *image = Magick::Image(bmpFileName);
    remove(bmpFileName.c_str());
    string rm = "rmdir ";
    rm += tmpDirName;
    system(rm.c_str());
}

static void addExif(ImageFileReaderInterface::DataVector exifData, string fileName)
{
    char tmpTemplate[] = "/tmp/tmp-addExif.XXXXXX";
    char *tmpDirName = mkdtemp(tmpTemplate);

    string exifFileName = tmpDirName;
    exifFileName += "/tmp.exif";
    ofstream exifFile(exifFileName);
    if (!exifFile.is_open()) {
        cerr << "could not open " << exifFileName << " for writing EXIF\n";
        exit(1);
    }
    exifFile.write((char*)&exifData[0], exifData.size());
    if (exifFile.bad()) {
        cerr << "failed to write " << exifData.size() << " bytes of EXIF to " << exifFileName << endl;
        exit(1);
    }
    exifFile.close();
    if (VERBOSE) {
        cout << "wrote " << exifData.size() << " bytes of EXIF to " << exifFileName << endl;
    }
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    int retval = system(("exiftool -m -overwrite_original " + fileName + " -tagsFromFile " + exifFileName).c_str());
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    remove(exifFileName.c_str());
    string rm = "rmdir ";
    rm += tmpDirName;
    system(rm.c_str());
    if (retval != 0) {
        cerr << "exiftool failed with exit code " << retval << endl;
        exit(1);
    }
    if (VERBOSE) {
        cout << "[timing] exiftool " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
    }
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
        cout << "grid is " << gridItem.outputWidth << "x" << gridItem.outputHeight << " pixels in tiles " << gridItem.rowsMinusOne << "x" << gridItem.columnsMinusOne << endl;
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
        throw ImageFileReaderInterface::FileReaderException(ImageFileReaderInterface::FileReaderException::StatusCode::UNSUPPORTED_CODE_TYPE);
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

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    Magick::Montage montageOptions;
    montageOptions.tile("8x6");
    montageOptions.geometry("512x512");
    list<Magick::Image> montage;
    Magick::montageImages(&montage, tileImages.begin(), tileImages.end(), montageOptions);
    Magick::Image image = montage.front();
    image.magick("JPEG");
    image.crop(Magick::Geometry(gridItem.outputWidth, gridItem.outputHeight));
    image.quality(92);

    string timingName = "magick montage+crop";
    if (MAX_SIZE > 0) {
        double scaleFactor;
        if (gridItem.outputWidth > gridItem.outputHeight) {
            scaleFactor = (double)MAX_SIZE / (double)gridItem.outputWidth;
        } else {
            scaleFactor = (double)MAX_SIZE / (double)gridItem.outputHeight;
        }

        if (scaleFactor < 1) {
            image.zoom(Magick::Geometry(scaleFactor * gridItem.outputWidth, scaleFactor * gridItem.outputHeight));
            timingName += "+zoom";
        }
    }
    image.write(outputFileName);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    if (VERBOSE) {
        cout << "[timing] " << timingName << " " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
    }

    addExif(exifData, outputFileName);
}

int usage()
{
    cerr << "Usage: heiftojpeg [-v] [-s <max_dimension>] <input.heic> <output.jpg>\n";
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
        cout << "Converting HEIF image " << inputFileName << " to JPEG " << outputFileName << endl;
    }

    try {
        chrono::steady_clock::time_point begin = chrono::steady_clock::now();
        processFile(inputFileName, outputFileName);
        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        if (VERBOSE) {
            cout << "[timing] processFile " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
        }
    } catch (ImageFileReaderInterface::FileReaderException e) {
        cerr << "exception occurred while processing " << inputFileName << ": " << e.what() << endl;
    }

    return 0;
}

