#ifndef FFGL_PLUGIN_APP
#define FFGL_PLUGIN_APP

#include "ofxFFGLPlugin.h"
#include "ofxFFMPEGVideoWriter.h"
#if ! __LP64__
#include "ofxHapPlayer.h"
#endif

//
//class BufferList{
//public:
//  void allocate(int num, int size,int format){
//    bufs.resize(num);
//    for(auto & b :bufs){
//      b.allocate(size,GL_STATIC_READ);
//    }
//  }
//
//  void readOne(int w,int h){
//    bufs[readPtr].bind(GL_PIXEL_PACK_BUFFER);
//    glReadPixels(0, 0 , w, h, GL_RGBA, GL_UNSIGNED_BYTE, 0);
//    readPtr++;
//    readPtr%=bufs.size();
//  }
//
//  void writeOne(){
//    bufs[readPtr].map();
//    glReadPixels(x, y, w, h, ofGetGlFormat(_pix), GL_UNSIGNED_BYTE, 0);
//    bufs[readPtr].unbind(GL_PIXEL_PACK_BUFFER);
//    readPtr++;
//    readPtr%=bufs.size();
//  }
//  vector<ofBufferObject> bufs;
//  int readPtr;
//  int writePtr;
//  
//};



class pluginApp : public ofFFGLPlugin
{
public:
    pluginApp(int minInputs = 1, int maxInputs = 1);
	virtual ~pluginApp();
    
    // do something when a parameter has changed
    virtual void onParameterChanged(ofPtr<ofFFGLParameter>);
    
    void setup();
    void initVideoRecorder();
    void update();
    void draw();
    void setupShaders();
    

//    ofFbo fbo;
  
		ofPtr<ofxFFMPEGVideoWriter>	vidRecorder;

  ofVideoPlayer recordedVideoPlayback;

		void videoSaved(ofVideoSavedEventArgs& e);

  vector<string> videoDevices;
  vector<string> audioDevices;
  #if ! __LP64__
    ofxHapPlayer
#else
  ofVideoPlayer
#endif
  videoPlayer;


private: // Insert here declarations needed in your implementation


  void addFrame(ofTexture * tex);

    string debugText;
    string lastLoadedFile;


  const char * videoCacheDirectory;

    float numRepeats;
    bool playPause;
    bool rec,lastRec;
    bool play,lastPlay;
    float speed;
    string folderName;
    string fileExt;
//    void recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args);
            ofPixels _pix;
  vector<ofPixels> pixTab;
  ofBufferObject bufferObj[2];
  int idxBuf;
//  BufferList ofBuf;
};




#endif // FFGL_PLUGIN_APP

	
