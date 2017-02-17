//
//  ofxFFMPEGVideoWriter.h
//  ShapeDeform
//
//  Created by roy_shilkrot on 4/7/13.
//
//

#ifndef __ofxFFMPEGVideoWriter__
#define __ofxFFMPEGVideoWriter__

#include <iostream>
#ifdef __cplusplus
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#endif

class ofxFFMPEGVideoWriter {
    //instance variables
    AVCodec *codec;
    int size, frame_count;
//  AVFrame *picture;
  AVFrame *picture_rgba;
    struct SwsContext *sws_ctx;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *video_st;
    AVCodecContext* c;

    bool initialized;

public:
    ofxFFMPEGVideoWriter():oc(NULL),codec(NULL),initialized(false),frame_count(1) {}
    ~ofxFFMPEGVideoWriter(){close();}
    
    /**
     * setup the video writer
     * @param output filename, the codec and format will be determined by it. (e.g. "xxx.mpg" will create an MPEG1 file
     * @param width of the frame
     * @param height of the frame
     **/
    int setup(const char* filename, int width, int height,int targetFPS = 25);

  void update(float currentTime);
  bool needNewFrame();
    /**
     * add a frame to the video file
     * @param the pixels packed in RGB (24-bit RGBRGBRGB...)
     **/
    void addFrame( uint8_t* pixels);
  void addFrameInternal();
    /**
     * close the video file and release all datastructs
     **/
    void close();
    /**
     * is the videowriter initialized?
     **/
    bool isInitialized() const { return initialized; }
  int getWidth();
  int getHeight();
  int targetFPS;


  float getTime();

  float lastTimeFrameAdded ,currentTime;
};

#endif /* defined(__ofxFFMPEGVideoWriter__) */
