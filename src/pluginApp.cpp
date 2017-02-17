// **************************************************************************
//
// ofxFFGLPlugin for OpenFrameworks 080 - experimental version, jan 15 2014
// Modified by Davide Maniˆ for OpenFrameworks 080
// email: software@cogitamus.it
// based on ofxFFGLPlugin by Daniel Berio
//
// Tested on OSX (XC 5.02) only!
//
// Write user code here
//
// setup() update() and draw() work just like the old ones in testApp()
//
// event handlers have been removed, they were not being used anyway
// Handle parameter changes in onParameterChanged()
//
// **************************************************************************

#include "pluginApp.h"
#include "shaders.h"

#include <dlfcn.h> // for getExecutableFile() and related

extern CFFGLPluginInfo* g_CurrPluginInfo;
static int pIcount = 0;
pluginApp::pluginApp(int minInputs, int maxInputs)
{
  // add needed FFGL parameters here


  ofLogError("pluginApp") << "creating " << pIcount++;

  speed = 1;
  addFloatParameter("speed",	// name of parameter ( as it will appear in host )
                    &speed,      // address of the float this parameter will point to
                    -2,				// minimum value
                    2				// maximum value
                    );
  rec = false;
  addBoolParameter("Rec", &rec);

  play = false;
  addBoolParameter("Play",&play);


  showDebug = true;
  addBoolParameter("ShowDebug",&showDebug);

  bankParam = 0;
  addFloatParameter("bank", &bankParam,0,10);
  
  //  addStringParameter("videoCacheDirectory", &videoCacheDirectory);



  //  SetParamInfo(parameters.size(), "videoCachDir", FF_TYPE_TEXT, &videoCacheDirectory[0]);

  initParameters(); // Setup parameters with host


  ofSetLogLevel(ofLogLevel::OF_LOG_VERBOSE);
  ofSetLogLevel("ofShader",ofLogLevel::OF_LOG_VERBOSE);
  lastLoadedFile = "";




}

// **************************************************************************

pluginApp::~pluginApp()
{

  ofLogError("pluginApp") << "deleting " << pIcount--;

  //    videoRecorder->close(true);
}



const char* getExecutableFile()
{
  Dl_info exeInfo;
  dladdr((void*) getExecutableFile, &exeInfo); // quite a weird construct but totally legit :)
  return exeInfo.dli_fname;
}

// **************************************************************************
// Called by instantiateGL
// **************************************************************************

void pluginApp::setup()
{
  // Get the absolute location of the executable file in the bundle.
  string strExecFile(getExecutableFile());

  // Strip out the filename to just get the path
  int found = strExecFile.find_last_of("/");
  string strPath = strExecFile.substr(0, found) + "/data/";

  // Change the working directory to that of the data folder
  //chdir(strPath.c_str());
  ofSetDataPathRoot(strPath.c_str());


  //    fbo.allocate(ofGetWidth(), ofGetHeight());

  initVideoRecorder();


  videoPlayer.setLoopState(ofLoopType::OF_LOOP_NORMAL);
}

void pluginApp::initVideoRecorder(){
  ofFile tmpCacheF(ofFilePath::getUserHomeDir()+"/Documents/Entoorlooper/");
  if(!tmpCacheF.exists())tmpCacheF.create();
  videoCacheDirectory = tmpCacheF.getAbsolutePath().c_str();
  folderName = videoCacheDirectory;
  folderName = ofFilePath::addTrailingSlash(folderName);

  fileExt = ".mov"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats

  // override the default codecs if you like
  // run 'ffmpeg -codecs' to find out what your implementation supports (or -formats on some older versions)
  //    videoRecorder->setVideoCodec("hap");
  //    videoRecorder.setVideoFormat("hap_q");
  //    videoRecorder.setVideoBitrate("800k");

  // 4. Register for events so we'll know when videos finish saving.
  //  ofAddListener(videoRecorder->videoSavedEvent, this, &ofApp::videoSaved);

  videoRecorder = make_shared<ofxFFMPEGVideoWriter>();
  //  videoRecorder->initRecording();



  _pix.setImageType(ofImageType::OF_IMAGE_COLOR_ALPHA);




}

// **************************************************************************
// Gets called immediately after setup()
// **************************************************************************

void pluginApp::setupShaders()
{
}

void pluginApp::update()
{
  videoRecorder->update(timeNow);
}

// **************************************************************************
void pluginApp::addFrame(ofTexture * tex){
#if 0


#define useBufQ 0

  //  bool vFlip = false;
  int w = tex->getWidth();
  int h=tex->getHeight();
  //  int x= 0;
  //  int y= 0;
  //  int sh = h;
  _pix.allocate(w, h, OF_PIXELS_RGBA);
  //
  if(bufferObj[0].size()!=_pix.size()){
    bufferObj[0].allocate(_pix.size(),GL_STREAM_READ);
#if useBufQ
    bufferObj[1].allocate(_pix.size(),GL_STREAM_READ);
    tex->copyTo(bufferObj[1]);
#endif
    idxBuf = 0;
  }
  //  if(vFlip){
  //    y = sh - y;
  //    y -= h; // top, bottom issues
  //  }

  //  bufferObj.bind(GL_PIXEL_PACK_BUFFER);
  //  glReadPixels(x, y, w, h, ofGetGlFormat(_pix), GL_UNSIGNED_BYTE, 0); // read the memory....
  //  bufferObj.unbind(GL_PIXEL_PACK_BUFFER);
  int wI = (idxBuf)%2;

  int rI = (idxBuf+useBufQ)%2;


  tex->copyTo(bufferObj[wI]);
  unsigned char * p = bufferObj[rI].map<unsigned char>(GL_READ_ONLY);
  _pix.setFromExternalPixels(p,w,h,OF_PIXELS_RGBA);
  //  _pix.mirrorTo(_pix,true,false);
  bufferObj[rI].unmap();


  idxBuf = (idxBuf+useBufQ)%2;


#else
  tex->readToPixels(_pix);
#endif

#if 0

  int numTab = 8;
  int expectedTabHeight = ceil(tex->getHeight() *1.0/ numTab);
  int expectedTabWidth = tex->getWidth();
  //  if(pixTab.size()!=numTab){
  //    pixTab.resize(numTab);
  //    for(int  i = 0 ; i < pixTab.size() ; i++){
  //      pixTab[i]. allocate(tex->getWidth(), expectedTabHeight,OF_IMAGE_COLOR_ALPHA );
  //    }
  //  }


  int format = ofGetGlFormat(_pix);

  _pix.allocate(tex->getWidth(),tex->getHeight(),ofGetImageTypeFromGLType(GL_RGBA));
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  tex->bind();
  int x = 0;
  for(int  y = 0 ; y < tex->getHeight() ; y+=expectedTabHeight){
    if(y+expectedTabHeight > tex->getHeight()){
      expectedTabHeight = tex->getHeight() - y;
    }
    //    glGetTextureSubImage(tex->getTextureData().textureID,0,
    //                         x,y,0,
    //                         expectedTabWidth,expectedTabHeight,0,
    //                         format,
    //                         GL_UNSIGNED_BYTE,
    //                        _pix.getNumChannels()*expectedTabWidth * expectedTabHeight,
    //                         _pix.getData() + y*_pix.getNumChannels()*expectedTabWidth
    //                         );
    //    glReadPixels(0,0,
    //                 expectedTabWidth, expectedTabHeight,
    //                 format, GL_UNSIGNED_BYTE, _pix.getData() + y*_pix.getNumChannels()*expectedTabWidth);


    y+=expectedTabHeight;

  }
  tex->unbind();
#endif


  videoRecorder->addFrame(_pix.getData());


}

void pluginApp::draw()
{
  // input textures from host are stored here
  ofTexture * tex = inputTextures[0];
  int b= 0;
  int a = 0;
  glClearColor(b / 255., b / 255., b / 255., a / 255.);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if( !tex)
    return;

  if(tex->getWidth() != ofGetWidth()){
    ofLogError()<< "wrong size " << tex->getWidth() << "," << ofGetWidth();
    ofSetWindowShape(tex->getWidth(),tex->getHeight());
  }

  // check time interval
  if( rec && videoRecorder->isInitialized()){

    if(videoRecorder->needNewFrame()){
      addFrame(tex);
    }
    else{
      cout << "droping frame" << endl;
    }
  }


  if(!videoPlayer.isLoaded()){
    if(play && lastLoadedFile!=""){
      if(!videoPlayer.load(lastLoadedFile)){
        ofLogVerbose("VideoPlayer") << " can't play";
      }
      else{
        ofLogVerbose("videoPlayer") << "loaded " << lastLoadedFile << ",duration : " << videoPlayer.getDuration();
        videoPlayer.play();
      }
    }

  }

  if( videoPlayer.isLoaded()){
    if(play){

      videoPlayer.update();
      ofSetColor(255,255,255,255);
      ofFill();
      // fuckin vflip somwhere
      videoPlayer.draw(0,ofGetHeight(),ofGetWidth(),-ofGetHeight());
      //          ofLogVerbose("videoPlayer") <<"playPos: "<< videoPlayer.getPosition();
    }


  }



  ofSetColor(255,255,255,255);
  if(!play){
    tex->draw(0,0);


  }

  ofFill();

  //    stringstream ss;
  //    ss << "video queue size: " << videoRecorder.getVideoQueueSize() << endl
  //    << "audio queue size: " << videoRecorder.getAudioQueueSize() << endl
  //    << "FPS: " << ofGetFrameRate() << endl
  //    << (rec?"pause":"start") << " recording: r" << endl
  //    << (rec?"close current video file: c":"") << endl;


  //    ofDrawBitmapString(ss.str(),15,15);
  // Draw a simple geometric shape


  if(showDebug){

  ofSetColor(255,0,0);
  int pad = 20;
  if(rec){
    ofSetColor(255,0,0);
    if(videoRecorder)
    ofDrawRectangle(pad, pad, 2*pad, 2*pad);
  }
  if(play){
    ofSetColor(videoPlayer.isLoaded()?0:255,255,0);
    ofDrawRectangle(3*pad, 3*pad, 4*pad, 4*pad);
  }

  }

  ofSetColor( 255, 255, 255 );



}

string pluginApp::getCurrentFilePath(){
  return (folderName+ofToString(currentBank)+fileExt);
  
}
// **************************************************************************

void pluginApp::onParameterChanged(ofPtr<ofFFGLParameter> param)
{
  // Do something if you wish
  if(((bool*)param->getAddress()) == &rec && rec!=lastRec){
    string fileName =getCurrentFilePath();
    if(rec ) {

      if(videoPlayer.getMoviePath()==fileName){
        videoPlayer.close();
      }
      lastLoadedFile = fileName;

      videoRecorder->setup(fileName.c_str(), ofGetWidth(), ofGetHeight());

    }
    else if(!rec && videoRecorder->isInitialized()) {
      //            videoRecorder.setPaused(true);
      videoRecorder->close();
//      videoPlayer.load(fileName);
    }

    lastRec = rec;
  }
  else   if(((bool*)param->getAddress()) == &play && play!=lastPlay){

    if(play ) {

      //            videoRecorder.commonPipeFolder = folderName;
      //          ofGetTimestampString()
      string fileName =getCurrentFilePath();
      if(videoPlayer.getMoviePath()!=fileName){
        videoPlayer.close();
        videoPlayer.load(fileName);
      }
      else{
        videoPlayer.setPosition(0);
      }
      videoPlayer.play();

    }
    else if(!play && videoPlayer.isInitialized()) {
      videoPlayer.stop();
    }

    lastPlay = play;
  }
  else if(((float*)param->getAddress()) == &speed ){
    videoPlayer.setSpeed(speed);


  }
  else if(((float*)param->getAddress()) == &bankParam){

    int cB =   floor(bankParam);
    if(cB!=currentBank){

      currentBank = cB;
      string curPath = getCurrentFilePath();
      if(!play &&ofFile(curPath).exists() && videoPlayer.getMoviePath()!=curPath){
        videoPlayer.load(curPath);
      }
    }
  }
  else{
    int dbg;
    dbg++;
  }
}

// **************************************************************************
// **************************************************************************



