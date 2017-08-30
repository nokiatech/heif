#include "hevcimagefilereader.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>
#include <list>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#define INBUF_SIZE 4096

using namespace std;


static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

static int decode_write_frame(const char *outfilename, AVCodecContext *avctx,
                              AVFrame *frame, int *frame_count, AVPacket *pkt, int last)
{
    int len, got_frame;
    char buf[1024];
    len = avcodec_decode_video2(avctx, frame, &got_frame, pkt);
    cout << "J\n";
    if (len < 0) {
        fprintf(stderr, "Error while decoding frame %d\n", *frame_count);
        return 0;
    }
    if (got_frame) {
        printf("Saving %sframe %3d\n", last ? "last " : "", *frame_count);
        fflush(stdout);
        /* the picture is allocated by the decoder, no need to free it */
        snprintf(buf, sizeof(buf), outfilename, *frame_count);
        pgm_save(frame->data[0], frame->linesize[0],
                 avctx->width, avctx->height, buf);
        (*frame_count)++;
    }
    if (pkt->data) {
        pkt->size -= len;
        pkt->data += len;
    }
    return 1;
}

static int decodeData(ImageFileReaderInterface::DataVector data, Magick::Image *image)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int frame_count;
    // FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    AVPacket avpkt;
    av_init_packet(&avpkt);
    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);
    // printf("Decode video file %s to %s\n", filename, outfilename);
    /* find the mpeg1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        return 0;
    }
    c = avcodec_alloc_context3(codec);
    c->width = 512;
    c->height = 512;
    // c.width =
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 0;
    }
    if(codec->capabilities&CODEC_CAP_TRUNCATED)
        c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return 0;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return 0;
    }

    frame_count = 0;

    avpkt.data = data.data();
    avpkt.size = data.size();
    std::string tmpfilename = "tmp.bmp";
    if (decode_write_frame(tmpfilename.c_str(), c, frame, &frame_count, &avpkt, 0)) {
        cout << "wrote frame bitmap to " << tmpfilename << "\n";
        *image = Magick::Image(tmpfilename);
    }

    avcodec_close(c);
    av_free(c);
    av_frame_free(&frame);

    return 1;
}
void processFile(char *filename)
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::GridItem gridItem;
    ImageFileReaderInterface::IdVector gridItemIds;

    cout << "reading image data from " << filename << "...\n";
    reader.initialize(filename);
    const auto& properties = reader.getFileProperties();
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    cout << "getting grid...\n";
    reader.getItemListByType(contextId, "grid", gridItemIds);
    cout << "found " << gridItemIds.size() << " grid items\n";
    const uint32_t itemId = gridItemIds.at(0);
    cout << "grid item 0 has id " << itemId << "\n";
    gridItem = reader.getItemGrid(contextId, itemId);

    uint32_t width = gridItem.outputWidth;
    uint32_t height = gridItem.outputHeight;
    uint8_t rows = gridItem.rowsMinusOne + 1;
    uint8_t cols = gridItem.columnsMinusOne + 1;
    cout << "grid is " << width << "x" << height << " pixels in tiles " << rows << "x" << cols << "\n";

    cout << "loading tiles...\n";
    ImageFileReaderInterface::IdVector tileItemIds;
    reader.getItemListByType(contextId, "master", tileItemIds);
    cout << "found " << tileItemIds.size() << " tile images\n";

    cout << "loading props...\n";
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

    std::vector<Magick::Image> tileImages;
    for (auto& tileItemId: tileItemIds) {
        ImageFileReaderInterface::DataVector data;
        reader.getItemDataWithDecoderParameters(contextId, tileItemId, data);
        Magick::Image image;
        decodeData(data, &image);
        tileImages.push_back(image);
    }

    // HevcImageFileReader::ParameterSetMap paramset;
    // reader.getDecoderParameterSets(contextId, tileItemIds.at(0), paramset);

    // std::ofstream ofs(dstfile, std::ios::binary);
    // for (const auto& key : {"VPS", "SPS", "PPS"}) {
    //     const auto& nalu = paramset[key];
    //     std::cout << key << " len=" << nalu.size() << std::endl;
    //     ofs.write((const char *)nalu.data(), nalu.size());
    // }
    // std::cout << "bitstream=" << bitstream.size() << std::endl;
    // ofs.write((const char *)bitstream.data(), bitstream.size());

    Magick::Montage montageOptions;
    montageOptions.tile("8x6");
    std::list<Magick::Image> montage;
    Magick::montageImages(&montage, tileImages.begin(), tileImages.end(), montageOptions);
    Magick::Image image = montage.front();
    image.magick("JPEG");
    image.write("out.jpg");
}

int main(int argc, char *argv[])
{
    Magick::InitializeMagick(*argv);
    avcodec_register_all();
    if ( argc != 3 ) {
        cout << "usage: heiftojpeg <input_file_name> <output_file_name>";
        return 1;
    }

    char *input_file_name = argv[1];
    char *output_file_name = argv[2];

    cout << "Converting HEIF image " << input_file_name << " to JPEG " << output_file_name << "\n";


    processFile(input_file_name);

    // cout << "item data received with size " << data.size() << "\n";

    // std::ofstream output_file(output_file_name);
    // std::ostream_iterator<std::uint8_t> output_iterator(output_file, "\n");
    // std::copy(data.begin(), data.end(), output_iterator);


    // decodeData(data, output_file_name);
    return 0;
}

