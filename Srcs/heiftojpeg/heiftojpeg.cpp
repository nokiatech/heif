#include "hevcimagefilereader.hpp"
#include <iostream>
#include <fstream>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#define INBUF_SIZE 4096

using namespace std;

void getData(char *filename, ImageFileReaderInterface::DataVector& data)
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::IdVector itemIds;

    cout << "reading image data from " << filename << "...\n";
    reader.initialize(filename);
    const auto& properties = reader.getFileProperties();

    cout << "getting master image id...\n";
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
    reader.getItemListByType(contextId, "master", itemIds);
    cout << "items " << itemIds.size() << "\n";
    const uint32_t itemId = itemIds.at(0);
    cout << "item id " << itemId << " " << reader.getItemType(contextId, itemId) << "\n";
    const auto props = reader.getItemProperties(contextId, itemId);
    for (const auto& property : props)
    {

        cout << "prop ";
        if (property.type == ImageFileReaderInterface::ItemPropertyType::AUXC) {
            cout << "AUXC";
            cout << reader.getPropertyAuxc(contextId, property.index).auxType;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::AVCC) {
            cout << "AVCC";
            // cout << reader.getPropertyAvcc(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::CLAP) {
            cout << "CLAP";
            cout << reader.getPropertyClap(contextId, property.index).widthN;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::HVCC) {
            cout << "HVCC";
            // cout << reader.getPropertyHvcc(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::IMIR) {
            cout << "IMIR";
            cout << reader.getPropertyImir(contextId, property.index).horizontalAxis;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::IROT) {
            cout << "IROT";
            cout << reader.getPropertyIrot(contextId, property.index).rotation;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::ISPE) {
            cout << "ISPE";
            // cout << reader.getPropertyIspe(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::LHVC) {
            cout << "LHVC";
            // cout << reader.getPropertyLhvc(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::LSEL) {
            cout << "LSEL";
            cout << reader.getPropertyLsel(contextId, property.index).layerId;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::OINF) {
            cout << "OINF";
            // cout << reader.getPropertyOinf(contextId, property.index);
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::RLOC) {
            cout << "RLOC";
            cout << reader.getPropertyRloc(contextId, property.index).horizontalOffset;
        } else if (property.type == ImageFileReaderInterface::ItemPropertyType::TOLS) {
            cout << "TOLS";
            cout << reader.getPropertyTols(contextId, property.index).targetOlsIndex;
        } else {
            cout << "unknown";
        }
        cout << "\n";
    }
    // const uint32_t itemId = reader.getCoverImageItemId(contextId);
    // reader.getItemListByType(contextId, "iden", itemIds);
    // cout << "items " << itemIds.size() << "\n";
    // const uint32_t itemId = itemIds.at(0);
    // reader.getReferencedFromItemListByType(contextId, itemId, "dimg", itemIds);
    // const uint32_t sourceItemId = itemIds.at(0); // For demo purposes, assume there was one
    // reader.getItemListByType(contextId, "thmb", itemIds);
    // cout << "items " << itemIds.size() << "\n";
    // const uint32_t thumbnailId = itemIds.at(0);

    // cout << "thumbnail found with id " << thumbnailId << "\n";
    reader.getItemDataWithDecoderParameters(contextId, itemId, data);

    // cout << "cover image found with id " << itemId << " source " << sourceItemId << ", getting image data...\n";
    // reader.getItemDataWithDecoderParameters(contextId, sourceItemId, data);
}



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
        return len;
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
    return 0;
}

void decodeData(ImageFileReaderInterface::DataVector data, char *outfilename)
{
    cout << "A\n";
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int frame_count;
    // FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    AVPacket avpkt;
    cout << "B\n";
    av_init_packet(&avpkt);
    cout << "C\n";
    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);
    cout << "D\n";
    // printf("Decode video file %s to %s\n", filename, outfilename);
    /* find the mpeg1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    cout << "E\n";
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    // c->width = 4096;
    // c->height = 3072;
    // c.width =
    cout << "F\n";
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    if(codec->capabilities&CODEC_CAP_TRUNCATED)
        c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    /* open it */
    cout << "G\n";
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    cout << "H\n";
    frame_count = 0;
    // int copied = 0;
    // for (;;) {
    //     memcpy(inbuf, data.data(), INBUF_SIZE);
    //     vector<uint8_t> subdata(data.begin() + copied, data.begin() + INBUF_SIZE + copied);
    //     avpkt.size = subdata.size();
    //     copied += subdata.size();
    //     if (avpkt.size == 0)
    //         break;
    //     /* NOTE1: some codecs are stream based (mpegvideo, mpegaudio)
    //        and this is the only method to use them because you cannot
    //        know the compressed data size before analysing it.
    //        BUT some other codecs (msmpeg4, mpeg4) are inherently frame
    //        based, so you must call them with all the data for one
    //        frame exactly. You must also initialize 'width' and
    //        'height' before initializing them. */
    //      NOTE2: some codecs allow the raw parameters (frame size,
    //        sample rate) to be changed at any frame. We handle this, so
    //        you should also take care of it
    //     /* here, we use a stream based decoder (mpeg1video), so we
    //        feed decoder and see if it could decode a frame */
    //     memcpy(inbuf, subdata.data(), subdata.size());
    //     avpkt.data = inbuf;
    //     while (avpkt.size > 0) {
    //         cout << "I\n";
    //         if (decode_write_frame(outfilename, c, frame, &frame_count, &avpkt, 0) < 0) {
    //             avpkt.data = NULL;
    // avpkt.size = 0;
    // decode_write_frame(outfilename, c, frame, &frame_count, &avpkt, 1);
    //             exit(1);
    //         }
    //     }
    // }
    // avpkt.data = NULL;
    // avpkt.size = 0;
    // decode_write_frame(outfilename, c, frame, &frame_count, &avpkt, 1);

    avpkt.data = data.data();
    avpkt.size = data.size();
    decode_write_frame(outfilename, c, frame, &frame_count, &avpkt, 0);

    avcodec_close(c);
    av_free(c);
    av_frame_free(&frame);
    printf("\n");
}

int main(int argc, char *argv[])
{
    avcodec_register_all();
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

    // std::ofstream output_file(output_file_name);
    // std::ostream_iterator<std::uint8_t> output_iterator(output_file, "\n");
    // std::copy(data.begin(), data.end(), output_iterator);


    decodeData(data, output_file_name);
    return 0;
}

