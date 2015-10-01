//
//  G3MWidget.cpp
//  G3MiOSSDK
//
//  Created by José Miguel S N on 31/05/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "G3MWidget.hpp"

#include "ProtoRenderer.hpp"
#include "Camera.hpp"
#include "GL.hpp"
#include "TexturesHandler.hpp"
#include "IDownloader.hpp"
#include "Effects.hpp"
#include "Context.hpp"
#include "ICameraConstrainer.hpp"
#include "FrameTasksExecutor.hpp"
#include "IStringUtils.hpp"
#include "IThreadUtils.hpp"
#include "IStringBuilder.hpp"
#include "IJSONParser.hpp"
#include "GLConstants.hpp"
#include "PeriodicalTask.hpp"
#include "CameraGoToPositionEffect.hpp"
#include "CameraRenderer.hpp"
#include "IStorage.hpp"
#include "OrderedRenderable.hpp"
#include <math.h>
#include "GInitializationTask.hpp"
#include "ITextUtils.hpp"
#include "TouchEvent.hpp"
#include "GPUProgramManager.hpp"
#include "GLGlobalState.hpp"
#include "ICameraActivityListener.hpp"
#include "SceneLighting.hpp"
#include "PlanetRenderer.hpp"
#include "ErrorRenderer.hpp"
#include "IDeviceAttitude.hpp"
#include "IDeviceLocation.hpp"
#include "IDeviceInfo.hpp"

#warning TO BE REMOVED
#include <OpenGLES/ES2/gl.h>


void G3MWidget::initSingletons(ILogger*            logger,
                               IFactory*           factory,
                               const IStringUtils* stringUtils,
                               IStringBuilder*     stringBuilder,
                               IMathUtils*         mathUtils,
                               IJSONParser*        jsonParser,
                               ITextUtils*         textUtils,
                               IDeviceAttitude*    devAttitude,
                               IDeviceLocation*    devLocation) {
  if (ILogger::instance() == NULL) {
    ILogger::setInstance(logger);
    IFactory::setInstance(factory);
    IStringUtils::setInstance(stringUtils);
    IStringBuilder::setInstance(stringBuilder);
    IMathUtils::setInstance(mathUtils);
    IJSONParser::setInstance(jsonParser);
    ITextUtils::setInstance(textUtils);
    IDeviceAttitude::setInstance(devAttitude);
    IDeviceLocation::setInstance(devLocation);
  }
  else {
    ILogger::instance()->logWarning("Singletons already set");
  }
}

G3MWidget::G3MWidget(GL*                                  gl,
                     IStorage*                            storage,
                     IDownloader*                         downloader,
                     IThreadUtils*                        threadUtils,
                     ICameraActivityListener*             cameraActivityListener,
                     const Planet*                        planet,
                     std::vector<ICameraConstrainer*>     cameraConstrainers,
                     CameraRenderer*                      cameraRenderer,
                     Renderer*                            mainRenderer,
                     ProtoRenderer*                       busyRenderer,
                     ErrorRenderer*                       errorRenderer,
                     Renderer*                            hudRenderer,
                     const Color&                         backgroundColor,
                     const bool                           logFPS,
                     const bool                           logDownloaderStatistics,
                     GInitializationTask*                 initializationTask,
                     bool                                 autoDeleteInitializationTask,
                     std::vector<PeriodicalTask*>         periodicalTasks,
                     GPUProgramManager*                   gpuProgramManager,
                     SceneLighting*                       sceneLighting,
                     const InitialCameraPositionProvider* initialCameraPositionProvider,
                     InfoDisplay* infoDisplay):
_frameTasksExecutor( new FrameTasksExecutor() ),
_effectsScheduler( new EffectsScheduler() ),
_gl(gl),
_downloader(downloader),
_storage(storage),
_threadUtils(threadUtils),
_cameraActivityListener(cameraActivityListener),
_texturesHandler( new TexturesHandler(_gl, false) ),
_planet(planet),
_cameraConstrainers(cameraConstrainers),
_cameraRenderer(cameraRenderer),
_mainRenderer(mainRenderer),
_busyRenderer(busyRenderer),
_errorRenderer(errorRenderer),
_hudRenderer(hudRenderer),
_width(1),
_height(1),
_currentCamera(new Camera(1, this)),
_nextCamera(new Camera(2, this)),
_backgroundColor( new Color(backgroundColor) ),
_timer(IFactory::instance()->createTimer()),
_renderCounter(0),
_totalRenderTime(0),
_logFPS(logFPS),
_rendererState( new RenderState( RenderState::busy() ) ),
_selectedRenderer(NULL),
_renderStatisticsTimer(NULL),
_logDownloaderStatistics(logDownloaderStatistics),
_userData(NULL),
_initializationTask(initializationTask),
_autoDeleteInitializationTask(autoDeleteInitializationTask),
_surfaceElevationProvider( mainRenderer->getSurfaceElevationProvider() ),
_context(new G3MContext(IFactory::instance(),
                        IStringUtils::instance(),
                        threadUtils,
                        ILogger::instance(),
                        IMathUtils::instance(),
                        IJSONParser::instance(),
                        _planet,
                        downloader,
                        _effectsScheduler,
                        storage,
                        mainRenderer->getSurfaceElevationProvider()
                        )),
_paused(false),
_initializationTaskWasRun(false),
_initializationTaskReady(true),
_clickOnProcess(false),
_gpuProgramManager(gpuProgramManager),
_sceneLighting(sceneLighting),
_rootState(NULL),
_initialCameraPositionProvider(initialCameraPositionProvider),
_initialCameraPositionHasBeenSet(false),
_forceBusyRenderer(false),
_nFramesBeetweenProgramsCleanUp(500),
_touchDownPositionX(0),
_touchDownPositionY(0),
_frameBufferContent(EMPTY_FRAMEBUFFER),
_infoDisplay(infoDisplay)
{
  _effectsScheduler->initialize(_context);
  _cameraRenderer->initialize(_context);
  _mainRenderer->initialize(_context);
  _busyRenderer->initialize(_context);
  _errorRenderer->initialize(_context);
  if (_hudRenderer != NULL) {
    _hudRenderer->initialize(_context);
  }
  _currentCamera->initialize(_context);
  _nextCamera->initialize(_context);

  if (_threadUtils != NULL) {
    _threadUtils->initialize(_context);
  }

  if (_storage != NULL) {
    _storage->initialize(_context);
  }

  if (_downloader != NULL) {
    _downloader->initialize(_context, _frameTasksExecutor);
    _downloader->start();
  }

  for (int i = 0; i < periodicalTasks.size(); i++) {
    addPeriodicalTask(periodicalTasks[i]);
  }
  _mainRenderer->setChangedRendererInfoListener((ChangedRendererInfoListener*)this, -1);


  _renderContext = new G3MRenderContext(_frameTasksExecutor,
                                        IFactory::instance(),
                                        IStringUtils::instance(),
                                        _threadUtils,
                                        ILogger::instance(),
                                        IMathUtils::instance(),
                                        IJSONParser::instance(),
                                        _planet,
                                        _gl,
                                        _currentCamera,
                                        _nextCamera,
                                        _texturesHandler,
                                        _downloader,
                                        _effectsScheduler,
                                        IFactory::instance()->createTimer(),
                                        _storage,
                                        _gpuProgramManager,
                                        _surfaceElevationProvider);


  //#ifdef C_CODE
  //  delete _rendererState;
  //  _rendererState = new RenderState( calculateRendererState() );
  //#endif
  //#ifdef JAVA_CODE
  //  _rendererState = calculateRendererState();
  //#endif
}


G3MWidget* G3MWidget::create(GL*                                  gl,
                             IStorage*                            storage,
                             IDownloader*                         downloader,
                             IThreadUtils*                        threadUtils,
                             ICameraActivityListener*             cameraActivityListener,
                             const Planet*                        planet,
                             std::vector<ICameraConstrainer*>     cameraConstrainers,
                             CameraRenderer*                      cameraRenderer,
                             Renderer*                            mainRenderer,
                             ProtoRenderer*                       busyRenderer,
                             ErrorRenderer*                       errorRenderer,
                             Renderer*                            hudRenderer,
                             const Color&                         backgroundColor,
                             const bool                           logFPS,
                             const bool                           logDownloaderStatistics,
                             GInitializationTask*                 initializationTask,
                             bool                                 autoDeleteInitializationTask,
                             std::vector<PeriodicalTask*>         periodicalTasks,
                             GPUProgramManager*                   gpuProgramManager,
                             SceneLighting*                       sceneLighting,
                             const InitialCameraPositionProvider* initialCameraPositionProvider,
                             InfoDisplay* infoDisplay) {

  return new G3MWidget(gl,
                       storage,
                       downloader,
                       threadUtils,
                       cameraActivityListener,
                       planet,
                       cameraConstrainers,
                       cameraRenderer,
                       mainRenderer,
                       busyRenderer,
                       errorRenderer,
                       hudRenderer,
                       backgroundColor,
                       logFPS,
                       logDownloaderStatistics,
                       initializationTask,
                       autoDeleteInitializationTask,
                       periodicalTasks,
                       gpuProgramManager,
                       sceneLighting,
                       initialCameraPositionProvider,
                       infoDisplay);
}

G3MWidget::~G3MWidget() {
  delete _rendererState;
  delete _renderContext;

  delete _userData;

  delete _planet;
  delete _cameraRenderer;
  delete _mainRenderer;
  delete _busyRenderer;
  delete _errorRenderer;
  delete _hudRenderer;
  delete _gl;
  delete _effectsScheduler;
  delete _currentCamera;
  delete _nextCamera;
  delete _texturesHandler;
  delete _timer;

  if (_downloader != NULL) {
    _downloader->stop();
    delete _downloader;
  }

  delete _storage;
  delete _threadUtils;
  delete _cameraActivityListener;

  for (unsigned int n=0; n < _cameraConstrainers.size(); n++) {
    delete _cameraConstrainers[n];
  }
  delete _frameTasksExecutor;

  for (int i = 0; i < _periodicalTasks.size(); i++) {
    PeriodicalTask* periodicalTask =  _periodicalTasks[i];
    delete periodicalTask;
  }

  delete _context;

  if (_rootState != NULL) {
    _rootState->_release();
  }
  delete _initialCameraPositionProvider;

  if(_infoDisplay != NULL) {
    delete _infoDisplay;
  }
}

void G3MWidget::removeAllPeriodicalTasks() {
  for (int i = 0; i < _periodicalTasks.size(); i++) {
    PeriodicalTask* periodicalTask =  _periodicalTasks[i];
    delete periodicalTask;
  }
  _periodicalTasks.clear();
}

void G3MWidget::notifyTouchEvent(const G3MEventContext &ec,
                                 const TouchEvent* touchEvent) const {
  const RenderState_Type renderStateType = _rendererState->_type;
  switch (renderStateType) {
    case RENDER_READY: {
      bool handled = false;

      if (_hudRenderer != NULL) {
        if (_hudRenderer->isEnable()) {
          handled = _hudRenderer->onTouchEvent(&ec, touchEvent);
        }
      }

      if (!handled && _mainRenderer->isEnable()) {
        handled = _mainRenderer->onTouchEvent(&ec, touchEvent);
      }

      if (!handled) {
        handled = _cameraRenderer->onTouchEvent(&ec, touchEvent);
        if (handled) {
          if (_cameraActivityListener != NULL) {
            _cameraActivityListener->touchEventHandled();
          }
        }
      }
      break;
    }
    case RENDER_BUSY: {
      break;
    }
    default: {
      break;
    }
  }
}

double G3MWidget::getDepthForPixel(float x, float y){
  zRender();
  
  const IMathUtils* mu = IMathUtils::instance();
  
  const int ix = mu->round(x);
  const int iy = mu->round(y);
  
  const double z = _gl->readPixelAsDouble(ix, iy, _width, _height);
  
  return z;
}

void G3MWidget::onTouchEvent(const TouchEvent* touchEvent) {

  G3MEventContext ec(IFactory::instance(),
                     IStringUtils::instance(),
                     _threadUtils,
                     ILogger::instance(),
                     IMathUtils::instance(),
                     IJSONParser::instance(),
                     _planet,
                     _downloader,
                     _effectsScheduler,
                     _storage,
                     _surfaceElevationProvider);

  // notify the original event
  notifyTouchEvent(ec, touchEvent);

  // creates DownUp event when a Down is immediately followed by an Up
  if (touchEvent->getTouchCount() == 1) {
    const TouchEventType eventType = touchEvent->getType();
    if (eventType == Down) {
      _clickOnProcess = true;
      const Vector2F pos = touchEvent->getTouch(0)->getPos();
      _touchDownPositionX = pos._x;
      _touchDownPositionY = pos._y;
    }
    else {
      if (eventType == Up) {
        if (_clickOnProcess) {
          const Touch* touch = touchEvent->getTouch(0);
          const TouchEvent* downUpEvent = TouchEvent::create(DownUp, new Touch(*touch));
          notifyTouchEvent(ec, downUpEvent);
          delete downUpEvent;
        }
      }
      if (_clickOnProcess) {
        if (eventType == Move) {
          const Vector2F movePosition = touchEvent->getTouch(0)->getPos();
          const double sd = movePosition.squaredDistanceTo(_touchDownPositionX, _touchDownPositionY);
          const float thresholdInPixels = _context->getFactory()->getDeviceInfo()->getPixelsInMM(1);
          if (sd > (thresholdInPixels * thresholdInPixels)) {
            _clickOnProcess = false;
          }
        }
        else {
          _clickOnProcess = false;
        }
      }
    }
  }
  else {
    _clickOnProcess = false;
  }

}



void G3MWidget::zRender(){
  
  if (_frameBufferContent == DEPTH_IMAGE){
    return; //It means no regular frame has been generated since last ZRender
  }

  if (_mainRenderer->isEnable()){
    GLState* zRenderGLState = new GLState();
    _gl->clearScreen(Color::black());
    _mainRenderer->zRender(_renderContext, zRenderGLState);
    zRenderGLState->_release();
    
    std::vector<OrderedRenderable*>* orderedRenderables = _renderContext->getSortedOrderedRenderables();
    if (orderedRenderables != NULL) {
      if (orderedRenderables->size() > 0){
        ILogger::instance()->logError("Some component is altering the OrderedRenderables list during Depth Rendering.");
      }
    }
    
    _frameBufferContent = DEPTH_IMAGE;
  }

}

void G3MWidget::onResizeViewportEvent(int width, int height) {
  G3MEventContext ec(IFactory::instance(),
                     IStringUtils::instance(),
                     _threadUtils,
                     ILogger::instance(),
                     IMathUtils::instance(),
                     IJSONParser::instance(),
                     _planet,
                     _downloader,
                     _effectsScheduler,
                     _storage,
                     _surfaceElevationProvider);

  _nextCamera->resizeViewport(width, height);
  _currentCamera->resizeViewport(width, height);
  _cameraRenderer->onResizeViewportEvent(&ec, width, height);
  _mainRenderer->onResizeViewportEvent(&ec, width, height);
  _busyRenderer->onResizeViewportEvent(&ec, width, height);
  _errorRenderer->onResizeViewportEvent(&ec, width, height);
  if (_hudRenderer != NULL) {
    _hudRenderer->onResizeViewportEvent(&ec, width, height);
  }
}


void G3MWidget::resetPeriodicalTasksTimeouts() {
  const size_t periodicalTasksCount = _periodicalTasks.size();
  for (size_t i = 0; i < periodicalTasksCount; i++) {
    PeriodicalTask* pt = _periodicalTasks[i];
    pt->resetTimeout();
  }
}

RenderState G3MWidget::calculateRendererState() {
  if (_forceBusyRenderer) {
    return RenderState::busy();
  }

  if (!_initializationTaskReady) {
    return RenderState::busy();
  }

  bool busyFlag = false;

  RenderState cameraRendererRenderState = _cameraRenderer->getRenderState(_renderContext);
  if (cameraRendererRenderState._type == RENDER_ERROR) {
    return cameraRendererRenderState;
  }
  else if (cameraRendererRenderState._type == RENDER_BUSY) {
    busyFlag = true;
  }

  if (_hudRenderer != NULL) {
    RenderState hudRendererRenderState = _hudRenderer->getRenderState(_renderContext);
    if (hudRendererRenderState._type == RENDER_ERROR) {
      return hudRendererRenderState;
    }
    else if (hudRendererRenderState._type == RENDER_BUSY) {
      busyFlag = true;
    }
  }

  RenderState mainRendererRenderState = _mainRenderer->getRenderState(_renderContext);
  if (mainRendererRenderState._type == RENDER_ERROR) {
    return mainRendererRenderState;
  }
  else if (mainRendererRenderState._type == RENDER_BUSY) {
    busyFlag = true;
  }

  return busyFlag ? RenderState::busy() : RenderState::ready();
}

void G3MWidget::rawRender(const RenderState_Type renderStateType) {
  
  if (_rootState == NULL) {
    _rootState = new GLState();
  }
  
  switch (renderStateType) {
    case RENDER_READY:
      setSelectedRenderer(_mainRenderer);
      _cameraRenderer->render(_renderContext, _rootState);
      
      _sceneLighting->modifyGLState(_rootState, _renderContext);  //Applying ilumination to rootState
      
      if (_mainRenderer->isEnable()) {
        _mainRenderer->render(_renderContext, _rootState);
      }
      
      break;
      
    case RENDER_BUSY:
      setSelectedRenderer(_busyRenderer);
      _busyRenderer->render(_renderContext, _rootState);
      break;
      
    default:
      _errorRenderer->setErrors( _rendererState->getErrors() );
      setSelectedRenderer(_errorRenderer);
      _errorRenderer->render(_renderContext, _rootState);
      break;
      
  }
  
  std::vector<OrderedRenderable*>* orderedRenderables = _renderContext->getSortedOrderedRenderables();
  if (orderedRenderables != NULL) {
    const size_t orderedRenderablesCount = orderedRenderables->size();
    for (size_t i = 0; i < orderedRenderablesCount; i++) {
      OrderedRenderable* orderedRenderable = orderedRenderables->at(i);
      orderedRenderable->render(_renderContext);
      delete orderedRenderable;
    }
    
    orderedRenderables->clear();
  }
  
  if (_hudRenderer != NULL) {
    if (renderStateType == RENDER_READY) {
      if (_hudRenderer->isEnable()) {
        _hudRenderer->render(_renderContext, _rootState);
      }
    }
  }


}

void G3MWidget::rawRenderStereoToedIn(const RenderState_Type renderStateType){
  Vector3D camPos = _currentCamera->getCartesianPosition();
  Vector3D camCenter = _currentCamera->getCenter();
  Vector3D eyesDirection = _currentCamera->getUp().cross(_currentCamera->getViewDirection()).normalized();
  
  //INTEROCULAR DISTANCE
  const double eyesSeparation = 20;// 0.03;
  
  _gl->clearScreen(*_backgroundColor);
  
  Camera centralCamera(1000, this);
  centralCamera.copyFrom(*_currentCamera);
  //centralCamera.initialize(_renderContext);
  
  _currentCamera->setFOV(Angle::fromDegrees(60), Angle::fromDegrees(60));
  
 // Vector3D center = centralCamera.pixel2PlanetPoint(Vector2F(_width/2, _height/2));
  
  Vector3D up = _currentCamera->getUp();
  
  //Left
  glViewport(0, 0, _width / 2, _height);
  Vector3D leftEyePosition = camPos.add(eyesDirection.times(-eyesSeparation));
  Vector3D leftEyeCenter = camCenter.add(eyesDirection.times(-eyesSeparation));
  
  //_currentCamera->setCartesianPosition(camPos.add(eyesDirection.times(-eyesSeparation)));
  _currentCamera->setLookAtParams(leftEyePosition.asMutableVector3D(), leftEyeCenter.asMutableVector3D(), up.asMutableVector3D());
  
  rawRender(renderStateType);

  
  //Right
  
  glViewport(_width / 2, 0, _width / 2, _height);
  Vector3D rightEyePosition = camPos.add(eyesDirection.times(eyesSeparation));
  Vector3D rightEyeCenter = camCenter.add(eyesDirection.times(eyesSeparation));
  
  //_currentCamera->setCartesianPosition(camPos.add(eyesDirection.times(eyesSeparation)));
  _currentCamera->setLookAtParams(rightEyePosition.asMutableVector3D(), rightEyeCenter.asMutableVector3D(), up.asMutableVector3D());
  
  
  rawRender(renderStateType);
  
//  _currentCamera->setCartesianPosition(camPos);
  
  _currentCamera->setLookAtParams(centralCamera.getCartesianPosition().asMutableVector3D(),
                                  centralCamera.getCenter().asMutableVector3D(),
                                  up.asMutableVector3D());
}

void G3MWidget::rawRenderStereoParallelAxis(const RenderState_Type renderStateType){
  Vector3D camPos = _currentCamera->getCartesianPosition();
  Vector3D camCenter = _currentCamera->getCenter();
  Vector3D eyesDirection = _currentCamera->getUp().cross(_currentCamera->getViewDirection()).normalized();
  const double eyesSeparation = 200;// 0.03;
  
  _gl->clearScreen(*_backgroundColor);
  
  Camera centralCamera(1000, this);
  centralCamera.copyFrom(*_currentCamera);
  
  Vector3D center = centralCamera.pixel2PlanetPoint(Vector2F(_width/2, _height/2));
  
  Vector3D up = _currentCamera->getUp();
  
  //Left
  glViewport(0, 0, _width / 2, _height);
  Vector3D leftEyePosition = camPos.add(eyesDirection.times(-eyesSeparation));
  Vector3D leftEyeCenter = camCenter.add(eyesDirection.times(-eyesSeparation));
  
  //_currentCamera->setCartesianPosition(camPos.add(eyesDirection.times(-eyesSeparation)));
  _currentCamera->setLookAtParams(leftEyePosition.asMutableVector3D(), leftEyeCenter.asMutableVector3D(), up.asMutableVector3D());
  
  rawRender(renderStateType);
  
  
  //Right
  
  glViewport(_width / 2, 0, _width / 2, _height);
  Vector3D rightEyePosition = camPos.add(eyesDirection.times(eyesSeparation));
  Vector3D rightEyeCenter = camCenter.add(eyesDirection.times(eyesSeparation));
  
  //_currentCamera->setCartesianPosition(camPos.add(eyesDirection.times(eyesSeparation)));
  _currentCamera->setLookAtParams(rightEyePosition.asMutableVector3D(), rightEyeCenter.asMutableVector3D(), up.asMutableVector3D());
  
  
  rawRender(renderStateType);
  
  //  _currentCamera->setCartesianPosition(camPos);
  
  _currentCamera->copyFrom(centralCamera);
}

void G3MWidget::render(int width, int height) {
  if (_paused) {
    return;
  }

  if (_width != width || _height != height) {
    _width = width;
    _height = height;

    onResizeViewportEvent(_width, _height);
  }

  if (!_initialCameraPositionHasBeenSet) {
    _initialCameraPositionHasBeenSet = true;

    const Geodetic3D position = _initialCameraPositionProvider->getCameraPosition(_planet,
                                                                                  _mainRenderer->getPlanetRenderer());

    _currentCamera->setGeodeticPosition(position);
    _currentCamera->setHeading(Angle::zero());
    _currentCamera->setPitch(Angle::fromDegrees(-90));
    _currentCamera->setRoll(Angle::zero());

    _nextCamera->setGeodeticPosition(position);
    _nextCamera->setHeading(Angle::zero());
    _nextCamera->setPitch(Angle::fromDegrees(-90));
    _nextCamera->setRoll(Angle::zero());
  }

  _timer->start();
  _renderCounter++;



  if (_initializationTask != NULL) {
    if (!_initializationTaskWasRun) {
      _initializationTask->run(_context);
      _initializationTaskWasRun = true;
    }

    _initializationTaskReady = _initializationTask->isDone(_context);
    if (_initializationTaskReady) {
      if (_autoDeleteInitializationTask) {
        delete _initializationTask;
      }
      _initializationTask = NULL;
    }
  }

  // Start periodical tasks
  const size_t periodicalTasksCount = _periodicalTasks.size();
  for (size_t i = 0; i < periodicalTasksCount; i++) {
    PeriodicalTask* pt = _periodicalTasks[i];
    pt->executeIfNecessary(_context);
  }

  // give to the CameraContrainers the opportunity to change the nextCamera
  const size_t cameraConstrainersCount = _cameraConstrainers.size();
  for (size_t i = 0; i< cameraConstrainersCount; i++) {
    ICameraConstrainer* constrainer = _cameraConstrainers[i];
    constrainer->onCameraChange(_planet,
                                _currentCamera,
                                _nextCamera);
  }
  _planet->applyCameraConstrainers(_currentCamera, _nextCamera);

  _currentCamera->copyFrom(*_nextCamera);



#ifdef C_CODE
  delete _rendererState;
  _rendererState = new RenderState( calculateRendererState() );
#endif
#ifdef JAVA_CODE
  _rendererState = calculateRendererState();
#endif
  const RenderState_Type renderStateType = _rendererState->_type;

  _renderContext->clearForNewFrame();

  _effectsScheduler->doOneCyle(_renderContext);

  _frameTasksExecutor->doPreRenderCycle(_renderContext);
  
#warning AT WORK JM
  rawRenderStereoToedIn(renderStateType);
  
  //Removing unused programs
  if (_renderCounter % _nFramesBeetweenProgramsCleanUp == 0) {
    _gpuProgramManager->removeUnused();
  }
  
  _frameBufferContent = REGULAR_FRAME; //FrameBuffer has been filled with a regular frame

  const long long elapsedTimeMS = _timer->elapsedTimeInMilliseconds();
  //  if (elapsedTimeMS > 100) {
  //    ILogger::instance()->logWarning("Frame took too much time: %dms", elapsedTimeMS);
  //  }
//#warning REMOVE
//  if (RENDER_READY == renderStateType){
//    zRender();
//  }
  
  if (_logFPS) {
    _totalRenderTime += elapsedTimeMS;

    if ((_renderStatisticsTimer == NULL) ||
        (_renderStatisticsTimer->elapsedTimeInMilliseconds() > 2000)) {
      const double averageTimePerRender = (double) _totalRenderTime / _renderCounter;
      const double fps = 1000.0 / averageTimePerRender;
      ILogger::instance()->logInfo("FPS=%f" , fps);

      _renderCounter = 0;
      _totalRenderTime = 0;

      if (_renderStatisticsTimer == NULL) {
        _renderStatisticsTimer = IFactory::instance()->createTimer();
      }
      else {
        _renderStatisticsTimer->start();
      }
    }
  }

  if (_logDownloaderStatistics) {
    std::string cacheStatistics = "";

    if (_downloader != NULL) {
      cacheStatistics = _downloader->statistics();
    }

    if (cacheStatistics != _lastCacheStatistics) {
      ILogger::instance()->logInfo("%s" , cacheStatistics.c_str());
      _lastCacheStatistics = cacheStatistics;
    }
  }
}

void G3MWidget::setSelectedRenderer(ProtoRenderer* selectedRenderer) {
  if (selectedRenderer != _selectedRenderer) {
    if (_selectedRenderer != NULL) {
      _selectedRenderer->stop(_renderContext);
    }
    _selectedRenderer = selectedRenderer;
    _selectedRenderer->start(_renderContext);
  }
}

void G3MWidget::onPause() {
  _paused = true;

  _threadUtils->onPause(_context);

  _effectsScheduler->onPause(_context);

  _mainRenderer->onPause(_context);
  _busyRenderer->onPause(_context);
  _errorRenderer->onPause(_context);
  if (_hudRenderer != NULL) {
    _hudRenderer->onPause(_context);
  }

  _downloader->onPause(_context);
  _storage->onPause(_context);
}

void G3MWidget::onResume() {
  _paused = false;

  _storage->onResume(_context);

  _downloader->onResume(_context);

  _mainRenderer->onResume(_context);
  _busyRenderer->onResume(_context);
  _errorRenderer->onResume(_context);
  if (_hudRenderer != NULL) {
    _hudRenderer->onResume(_context);
  }

  _effectsScheduler->onResume(_context);

  _threadUtils->onResume(_context);
}

void G3MWidget::onDestroy() {
  _threadUtils->onDestroy(_context);

  _effectsScheduler->onDestroy(_context);

  _mainRenderer->onDestroy(_context);
  _busyRenderer->onDestroy(_context);
  _errorRenderer->onDestroy(_context);
  if (_hudRenderer != NULL) {
    _hudRenderer->onDestroy(_context);
  }

  _downloader->onDestroy(_context);
  _storage->onDestroy(_context);
}

void G3MWidget::addPeriodicalTask(PeriodicalTask* periodicalTask) {
  _periodicalTasks.push_back(periodicalTask);
}

void G3MWidget::addPeriodicalTask(const TimeInterval& interval,
                                  GTask* task) {
  addPeriodicalTask( new PeriodicalTask(interval, task) );
}

void G3MWidget::setCameraHeading(const Angle& heading) {
  getNextCamera()->setHeading(heading);
}

void G3MWidget::setCameraPitch(const Angle& pitch) {
  getNextCamera()->setPitch(pitch);
}

void G3MWidget::setCameraRoll(const Angle& roll) {
  getNextCamera()->setRoll(roll);
}

void G3MWidget::setCameraHeadingPitchRoll(const Angle& heading,
                                          const Angle& pitch,
                                          const Angle& roll) {
  getNextCamera()->setHeadingPitchRoll(heading, pitch, roll);
}

void G3MWidget::setCameraPosition(const Geodetic3D& position) {
  getNextCamera()->setGeodeticPosition(position);
  _initialCameraPositionHasBeenSet = true;
}

void G3MWidget::setAnimatedCameraPosition(const Geodetic3D& position,
                                          const Angle& heading,
                                          const Angle& pitch) {
  setAnimatedCameraPosition(TimeInterval::fromSeconds(3), position, heading, pitch);
}

void G3MWidget::setAnimatedCameraPosition(const TimeInterval& interval,
                                          const Geodetic3D& position,
                                          const Angle& heading,
                                          const Angle& pitch,
                                          const bool linearTiming,
                                          const bool linearHeight) {
  const Geodetic3D fromPosition = _nextCamera->getGeodeticPosition();
  const Angle fromHeading = _nextCamera->getHeading();
  const Angle fromPitch   = _nextCamera->getPitch();

  setAnimatedCameraPosition(interval,
                            fromPosition, position,
                            fromHeading,  heading,
                            fromPitch,    pitch,
                            linearTiming,
                            linearHeight);
}

void G3MWidget::setAnimatedCameraPosition(const TimeInterval& interval,
                                          const Geodetic3D& fromPosition,
                                          const Geodetic3D& toPosition,
                                          const Angle& fromHeading,
                                          const Angle& toHeading,
                                          const Angle& fromPitch,
                                          const Angle& toPitch,
                                          const bool linearTiming,
                                          const bool linearHeight) {

  if (fromPosition.isEquals(toPosition) &&
      fromHeading.isEquals(toHeading) &&
      fromPitch.isEquals(toPitch)) {
    return;
  }

  double finalLatInDegrees = toPosition._latitude._degrees;
  double finalLonInDegrees = toPosition._longitude._degrees;

  //Fixing final latitude
  while (finalLatInDegrees > 90) {
    finalLatInDegrees -= 360;
  }
  while (finalLatInDegrees < -90) {
    finalLatInDegrees += 360;
  }

  //Fixing final longitude
  while (finalLonInDegrees > 360) {
    finalLonInDegrees -= 360;
  }
  while (finalLonInDegrees < 0) {
    finalLonInDegrees += 360;
  }
  if (fabs(finalLonInDegrees - fromPosition._longitude._degrees) > 180) {
    finalLonInDegrees -= 360;
  }

  const Geodetic3D finalToPosition = Geodetic3D::fromDegrees(finalLatInDegrees,
                                                             finalLonInDegrees,
                                                             toPosition._height);

  cancelCameraAnimation();

  _effectsScheduler->startEffect(new CameraGoToPositionEffect(interval,
                                                              fromPosition, finalToPosition,
                                                              fromHeading,  toHeading,
                                                              fromPitch,    toPitch,
                                                              linearTiming,
                                                              linearHeight),
                                 _nextCamera->getEffectTarget());
}

void G3MWidget::cancelAllEffects() {
  _effectsScheduler->cancelAllEffects();
}

void G3MWidget::cancelCameraAnimation() {
  EffectTarget* target = _nextCamera->getEffectTarget();
  _effectsScheduler->cancelAllEffectsFor(target);
}

void G3MWidget::setBackgroundColor(const Color& backgroundColor) {
  delete _backgroundColor;

  _backgroundColor = new Color(backgroundColor);
}

PlanetRenderer* G3MWidget::getPlanetRenderer() {
  return _mainRenderer->getPlanetRenderer();
}

bool G3MWidget::setRenderedSector(const Sector& sector) {
  const bool changed = getPlanetRenderer()->setRenderedSector(sector);
  if (changed) {
    _initialCameraPositionHasBeenSet = false;
  }
  return changed;
}

//void G3MWidget::notifyChangedInfo() const {
//  if(_hudRenderer != NULL){
//    const RenderState_Type renderStateType = _rendererState->_type;
//    switch (renderStateType) {
//      case RENDER_READY:
//      //_hudRenderer->setInfo(_mainRenderer->getInfo());
//      break;
//
//      case RENDER_BUSY:
//      break;
//
//      default:
//      break;
//
//    }
//  }
//}

void G3MWidget::changedRendererInfo(const int rendererIdentifier,
                                    const std::vector<const Info*>& info) {
  if(_infoDisplay != NULL){
    _infoDisplay->changedInfo(info);
  }
  //  else {
  //    ILogger::instance()->logWarning("Render Infos are changing and InfoDisplay is NULL");
  //  }
}


void G3MWidget::addCameraConstrainer(ICameraConstrainer* cc){
  _cameraConstrainers.push_back(cc);
}

void G3MWidget::removeCameraConstrainer(ICameraConstrainer* cc){
  size_t size = _cameraConstrainers.size();
  for (size_t i = 0; i < size; i++) {
    if (_cameraConstrainers[i] == cc){
#ifdef C_CODE
      _cameraConstrainers.erase(_cameraConstrainers.begin() + i);
#endif
#ifdef JAVA_CODE
      _cameraConstrainers.remove(i);
#endif
      return;
    }
  }
  ILogger::instance()->logError("Could not remove camera constrainer.");
}


