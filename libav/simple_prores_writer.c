/* Code to write a simple prores 4444 file.
   Took me a while to figure out how to do it so thought I'd share it.
   Don't treat this as "reference code", because I don't really know what I'm
   doing. However it does seem to work, here anyway.
*/

#include <stdio.h>
#include <stdlib.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

static void encode(AVCodecContext *codec_ctx, AVFormatContext *fmt_ctx, AVFrame *frame, AVPacket *pkt) {
    int ret;

    /* send the frame to the encoder */
    // if (frame)
    //     printf("Send frame %3"PRId64"\n", frame->pts);

    ret = avcodec_send_frame(codec_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        ret = av_interleaved_write_frame(fmt_ctx, pkt);
        if (ret < 0) {
            fprintf(stderr, "av_interleaved_write_frame failed\n");
            exit(1);
        }
        // printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        av_packet_unref(pkt);
    }
}


int main(int argc, char **argv) {
    const char *filename;
    const AVCodec *codec;
    AVCodecContext *codec_ctx = NULL;
    AVFormatContext *format_ctx = NULL;
    AVFrame *av_frame;
    AVFrame *src_frame;
    AVPacket *av_packet;
    AVStream *av_video_stream;
    uint32_t framerate = 25;
    uint32_t width = 720;
    uint32_t height = 480;

    if (argc <= 1) {
        fprintf(stderr, "Usage: %s <output file>\n", argv[0]);
        exit(0);
    }
    filename = argv[1];

    codec = avcodec_find_encoder(AV_CODEC_ID_PRORES);
    if (codec == NULL) {
        fprintf(stderr, "Could not find codec for id %d\n", AV_CODEC_ID_PRORES);
        exit(1);
    }

    // For prores 4444, the codec gets three pix_fmts (here anyway):
    // pix_fmts[0] seems to be 64, aka AV_PIX_FMT_YUV422P10LE ///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    // pix_fmts[1] seems to be 68, aka AV_PIX_FMT_YUV444P10LE ///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    // pix_fmts[2] seems to be 91, aka AV_PIX_FMT_YUVA444P10LE ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    int pix_fmt = codec->pix_fmts[1];
    if (pix_fmt != AV_PIX_FMT_YUV444P10LE) {
        fprintf(stderr, "Codec pix_fmt list was not as expected\n");
        exit(1);
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    format_ctx = avformat_alloc_context();
    int r = avformat_alloc_output_context2(&format_ctx, NULL, NULL, filename);
    if (r < 0) {
        fprintf(stderr, "Could not open format context for %s (%d)\n", filename, r);
        exit(1);
    }

    av_video_stream = avformat_new_stream(format_ctx, codec);
    if (!av_video_stream) {
        fprintf(stderr, "fail\n");
        exit(1);
    }

    codec_ctx->codec_id = codec->id;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->sample_aspect_ratio = (AVRational){ 1, 1 };
    codec_ctx->pix_fmt = pix_fmt;
    codec_ctx->time_base = (AVRational){1, 1};  // AFAICT this is ignored and the default stream time_base is used instead?
    codec_ctx->framerate = (AVRational){framerate, 1};
    codec_ctx->profile = FF_PROFILE_PRORES_4444;
    codec_ctx->thread_count = 0;
    codec_ctx->thread_type = FF_THREAD_FRAME;

    r = avcodec_open2(codec_ctx, codec, NULL);
    if (r < 0) {
        fprintf(stderr, "avcodec_open2 failed: %d\n", r);
        exit(1);
    }

    r = avcodec_parameters_from_context(av_video_stream->codecpar, codec_ctx);
    if (r < 0) {
        fprintf(stderr, "avcodec_parameters_from_context failed: %d\n", r);
        exit(1);
    }
//    av_video_stream->time_base = codec_ctx->framerate;  // AFAICT it's OK to just use the default?
    av_video_stream->avg_frame_rate = codec_ctx->framerate;
    av_video_stream->r_frame_rate = codec_ctx->framerate;

    r = avio_open(&format_ctx->pb, filename, AVIO_FLAG_WRITE);
    if (r < 0) {
        fprintf(stderr, "avio_open failed: %d\n", r);
        exit(1);
    }

    r = avformat_write_header(format_ctx, NULL);
    if (r < 0) {
        fprintf(stderr, "avformat_write_header failed: %d\n", r);
        exit(1);
    }

    av_packet = av_packet_alloc();
    if (!av_packet) {
        fprintf(stderr, "av_packet_alloc failed: %d\n", r);
        exit(1);
    }

    av_frame = av_frame_alloc();
    if (!av_frame) {
        fprintf(stderr, "av_frame_alloc failed for av_frame (%d)\n", r);
        exit(1);
    }
    av_frame->format = pix_fmt;
    av_frame->width = width;
    av_frame->height = height;

    r = av_frame_get_buffer(av_frame, 0);
    if (r < 0) {
        fprintf(stderr, "av_frame_get_buffer failed for av_frame (%d)\n", r);
        exit(1);
    }

    // Create a source frame we can draw on in simple RGB24 format
    src_frame = av_frame_alloc();
    if (!src_frame) {
        fprintf(stderr, "av_frame_alloc failed for src_frame (%d)\n", r);
        exit(1);
    }
    src_frame->width = width;
    src_frame->height = height;
    src_frame->format = AV_PIX_FMT_RGB24;
    r = av_frame_get_buffer(src_frame, 0);
    if (r < 0) {
        fprintf(stderr, "av_frame_get_buffer failed for src_frame (%d)\n", r);
        exit(1);
    }

    struct SwsContext *sws_context = sws_getContext(
        // src:
        width,
        height,
        AV_PIX_FMT_RGB24,
        // dst:
        width,
        height,
        pix_fmt,
        SWS_BILINEAR,
        0,
        0,
        0);
    if (!sws_context) {
        fprintf(stderr, "sws_getContext failed\n");
        exit(1);
    }

    // Write the actual video frames
    for (uint32_t i = 0; i < framerate * 5; i++) {

        // Write test image data into the src_frame, in straightforward RGB format
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                int index = (y * width + x) * 3; // 3 bytes per pixel (RGB)
                src_frame->data[0][index] = 255 * ((x + i * 10) % width) / width;
                src_frame->data[0][index + 1] = 255 * (float)x / width;
                src_frame->data[0][index + 2] = 255*(1 - (float)((x + i * 10) % width) / width);
            }
        }

        // Copy the image data into the video frame, in the correct planar format
        sws_scale(
            sws_context,
            (uint8_t const* const*)src_frame->data,
            src_frame->linesize,
            0,
            av_video_stream->codecpar->height,
            av_frame->data,
            av_frame->linesize
        );

        av_frame->pts = av_rescale_q(
            i,
            (AVRational){ 1, framerate },
            av_video_stream->time_base
        );
        encode(codec_ctx, format_ctx, av_frame, av_packet);
    }

    // flush the encoder
    encode(codec_ctx, format_ctx, NULL, av_packet);
    // write the trailer
    av_write_trailer(format_ctx);

    // tidy up
    avformat_close_input(&format_ctx);
    avformat_free_context(format_ctx);
    avcodec_free_context(&codec_ctx);
    av_frame_free(&av_frame);
    av_frame_free(&src_frame);
    av_packet_free(&av_packet);

    printf("Wrote file %s\n", filename);

    return 0;
}
