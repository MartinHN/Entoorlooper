//
//  ofxFFMPEGVideoWriter.cpp
//  ShapeDeform
//
//  Created by roy_shilkrot on 4/7/13.
//
//
// taken from ffmpeg's examples code: http://ffmpeg.org/doxygen/trunk/api-example_8c-source.html
// http://ffmpeg.org/doxygen/trunk/doc_2examples_2decoding_encoding_8c-example.html#a33
// http://ffmpeg.org/doxygen/trunk/doc_2examples_2muxing_8c-example.html#a75
//

#ifdef __cplusplus
extern "C" {
#include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"
#include "libavutil/opt.h"
}
#endif

#include "ofxFFMPEGVideoWriter.h"

void ofxFFMPEGVideoWriter::setup(const char* filename, int width, int height,int _targetFPS) {
  targetFPS = _targetFPS;
  printf("Video encoding: %s\n",filename);
  /* register all the formats and codecs */
  av_register_all();

  AVCodec * codecTest = av_codec_next(NULL);
  while(codecTest != NULL)
  {
    fprintf(stderr, "codecs : %s\n", codecTest->name);
    codecTest = av_codec_next(codecTest);
  }

  AVOutputFormat * outFormatTest = av_oformat_next(NULL);
  while(outFormatTest != NULL)
  {
    fprintf(stderr, "formats : %s\n", outFormatTest->name);
    outFormatTest = av_oformat_next(outFormatTest);
  }

  codec = avcodec_find_encoder(AV_CODEC_ID_HAP);



  /* allocate the output media context */
  avformat_alloc_output_context2(&oc, NULL, NULL, filename);
  if (!oc) {
    printf("Could not deduce output format from file extension: using MOV.\n");
    avformat_alloc_output_context2(&oc, NULL, "mov", filename);
  }
  if (!oc) {
    fprintf(stderr, "could not create AVFormat context\n");
    exit(1);
  }
  fmt = oc->oformat;
  if(codec){
    fmt->video_codec = codec->id;
  }

  /* Add the audio and video streams using the default format codecs
   * and initialize the codecs. */
  video_st = NULL;
  if (fmt->video_codec != AV_CODEC_ID_NONE) {
    /* find the video encoder */
    AVCodecID avcid = fmt->video_codec;
    if(codec==NULL)
      codec = avcodec_find_encoder(avcid);

    if (!codec) {
      fprintf(stderr, "codec not found: %s\n", avcodec_get_name(avcid));
      exit(1);
    } else {
      const AVPixelFormat* p = codec->pix_fmts;
      while (*p != AV_PIX_FMT_NONE) {
        printf("supported pix fmt: %s\n",av_get_pix_fmt_name(*p));
        ++p;
      }

    }

    video_st = avformat_new_stream(oc, codec);
    if (!video_st) {
      fprintf(stderr, "Could not allocate stream\n");
      exit(1);
    }
    video_st->id = oc->nb_streams-1;
    c = video_st->codec;

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
      c->flags |= CODEC_FLAG_GLOBAL_HEADER;
  }

  /* Now that all the parameters are set, we can open the audio and
   * video codecs and allocate the necessary encode buffers. */
  {


    /* put sample parameters */
    c->bit_rate = 400000;

    /* resolution must be a multiple of two */
    c->width = width;
    c->height = height;
    /* frames per second */
    c->time_base= (AVRational){1,targetFPS};
    video_st->time_base =(AVRational){1,targetFPS};
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames=1;
    c->pix_fmt = AV_PIX_FMT_RGBA;

    /* open it */

    AVDictionary* options = NULL;
    if(codec->id == AV_CODEC_ID_HAP){
      if(av_opt_set(c->priv_data,"format","hap_alpha",0) ||
         av_opt_set(c->priv_data,"compressor","snappy",0)){
        std::cout << "can't set hap options : " <<av_opt_set(c->priv_data,"format","hap_alpha",0) << "/" << av_opt_set(c->priv_data,"compressor","snappy",0) << std::endl;
        
        const AVOption * opt = av_opt_next(c->priv_data,NULL);
        while(opt){
          std::cout << opt->name << opt->type << std::endl;
          opt = av_opt_next(c->priv_data,opt);
        }
      }

    }
    if (avcodec_open2(c, codec, &options) < 0) {
      fprintf(stderr, "could not open codec\n");
      exit(1);
    }
    else {
      printf("opened %s\n", avcodec_get_name(codec->id));

    }

    /* alloc image and output buffer */
    int ret = -1;


    picture_rgba = av_frame_alloc();
    picture_rgba->format = AV_PIX_FMT_RGBA;
    picture_rgba->width = c->width;
    picture_rgba->height = c->height;
    picture_rgba->pts = 0;

    if((ret = av_image_alloc(picture_rgba->data, picture_rgba->linesize, c->width, c->height, (AVPixelFormat)picture_rgba->format, 24)) < 0) {
      fprintf(stderr,"cannot allocate RGB temp image\n");
      exit(1);
    } else
      printf("allocated picture of size %d (ptr %s), linesize %d %d %d %d\n",ret,picture_rgba->data[0],picture_rgba->linesize[0],picture_rgba->linesize[1],picture_rgba->linesize[2],picture_rgba->linesize[3]);


    size = ret;
  }

  av_dump_format(oc, 0, filename, 1);
  /* open the output file, if needed */
  if (!(fmt->flags & AVFMT_NOFILE)) {
    int ret;
    if ((ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE)) < 0) {
      fprintf(stderr, "Could not open '%s': %s\n", filename, av_err2str(ret));
      exit(1);
    }
  }
  /* Write the stream header, if any. */
  int ret = avformat_write_header(oc, NULL);
  if (ret < 0) {
    fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
    exit(1);
  }



  initialized = true;
  frame_count = 0;
  currentTime = 0;
  lastTimeFrameAdded - 100;
}

int ofxFFMPEGVideoWriter::getWidth(){
  if(c)return c->width;
  else  return 0;
}

int ofxFFMPEGVideoWriter::getHeight(){
  if(c)return c->height;
  else  return 0;
}
void ofxFFMPEGVideoWriter::update(float _currentTime){
  currentTime = _currentTime;
}
float ofxFFMPEGVideoWriter::getTime(){
  return currentTime;
}
bool ofxFFMPEGVideoWriter::needNewFrame() {
  return frame_count == 0 || (currentTime-lastTimeFrameAdded  > 1.0/targetFPS);
}
/* add a frame to the video file, RGB 24bpp format */
void ofxFFMPEGVideoWriter::addFrameInternal(){



  picture_rgba->pts += av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
  frame_count++;
}
void ofxFFMPEGVideoWriter::addFrame( uint8_t* pixels) {
  /* copy the buffer */
  picture_rgba->data[0]  = pixels;
//  memcpy(picture_rgba->data[0], pixels, size);

  /* convert RGB24 to YUV420 */
  //sws_scale(sws_ctx, picture_rgb24->data, picture_rgb24->linesize, 0, c->height, picture->data, picture->linesize);

  AVPacket pkt = { 0 };
  int got_packet;
  av_init_packet(&pkt);
  /* encode the image */
  int ret = avcodec_encode_video2(c, &pkt, picture_rgba, &got_packet);
  if (ret < 0) {
    fprintf(stderr, "Error encoding video frame: %s\n", av_err2str(ret));
    exit(1);
  }
  /* If size is zero, it means the image was buffered. */
  if (!ret && got_packet && pkt.size) {
    pkt.stream_index = video_st->index;
    /* Write the compressed frame to the media file. */
    ret = av_interleaved_write_frame(oc, &pkt);
  } else {
    ret = 0;
  }


  if(frame_count>0){
    float frameStep = 1.0/targetFPS;
  for(float t = lastTimeFrameAdded ; t < currentTime ; t+=frameStep){
    addFrameInternal();
  }
  }
  else{
    addFrameInternal();
  }

  lastTimeFrameAdded = currentTime;
}


void ofxFFMPEGVideoWriter::close() {
  /* Write the trailer, if any. The trailer must be written before you
   * close the CodecContexts open when you wrote the header; otherwise
   * av_write_trailer() may try to use memory that was freed on
   * av_codec_close(). */
  if(!initialized){return;}
    av_write_trailer(oc);
  /* Close each codec. */

    avcodec_close(video_st->codec);
  
  //    av_freep(&(picture->data[0]));
  //    av_free(picture);

  av_freep(picture_rgba->data[0]);
  av_free(picture_rgba);


  if (!(fmt->flags & AVFMT_NOFILE))
  /* Close the output file. */
    avio_close(oc->pb);

  /* free the stream */
  avformat_free_context(oc);

  printf("closed video file\n");

  initialized = false;
  frame_count = 0;
}
