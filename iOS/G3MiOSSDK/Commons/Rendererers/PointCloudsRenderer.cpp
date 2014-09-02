//
//  PointCloudsRenderer.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 8/19/14.
//
//

#include "PointCloudsRenderer.hpp"

#include "Context.hpp"
#include "IDownloader.hpp"
#include "Planet.hpp"
#include "DownloadPriority.hpp"
#include "Sector.hpp"
#include "ByteBufferIterator.hpp"
#include "ErrorHandling.hpp"

void PointCloudsRenderer::PointCloudMetadataDownloadListener::onDownload(const URL& url,
                                                                         IByteBuffer* buffer,
                                                                         bool expired) {
  ILogger::instance()->logInfo("Downloaded metadata for \"%s\" (bytes=%ld)",
                               _pointCloud->getCloudName().c_str(),
                               buffer->size());

  _threadUtils->invokeAsyncTask(new PointCloudMetadataParserAsyncTask(_pointCloud, buffer),
                                true);
}

void PointCloudsRenderer::PointCloudMetadataDownloadListener::onError(const URL& url) {
  _pointCloud->errorDownloadingMetadata();
}

void PointCloudsRenderer::PointCloudMetadataDownloadListener::onCancel(const URL& url) {
  // do nothing
}

void PointCloudsRenderer::PointCloudMetadataDownloadListener::onCanceledDownload(const URL& url,
                                                                                 IByteBuffer* buffer,
                                                                                 bool expired) {
  // do nothing
}


void PointCloudsRenderer::PointCloud::initialize(const G3MContext* context) {
  _downloadingMetadata = true;
  _errorDownloadingMetadata = false;
  _errorParsingMetadata = false;

  const std::string planetType = context->getPlanet()->getType();

  const URL metadataURL(_serverURL, _cloudName + "?planet=" + planetType + "&format=binary");

  ILogger::instance()->logInfo("Downloading metadata for \"%s\"", _cloudName.c_str());

  context->getDownloader()->requestBuffer(metadataURL,
                                          _downloadPriority,
                                          _timeToCache,
                                          _readExpired,
                                          new PointCloudsRenderer::PointCloudMetadataDownloadListener(this, context->getThreadUtils()),
                                          true);
}

PointCloudsRenderer::PointCloud::~PointCloud() {
  delete _octree;
  delete _sector;
}

void PointCloudsRenderer::PointCloud::errorDownloadingMetadata() {
  _downloadingMetadata = false;
  _errorDownloadingMetadata = true;
}

PointCloudsRenderer::PointCloudMetadataParserAsyncTask::~PointCloudMetadataParserAsyncTask() {
  delete _sector;
  delete _buffer;
  delete _octree;
}

void PointCloudsRenderer::PointCloudMetadataParserAsyncTask::runInBackground(const G3MContext* context) {
  ByteBufferIterator it(_buffer);

  _pointsCount = it.nextInt64();

  const double lowerLatitude  = it.nextDouble();
  const double lowerLongitude = it.nextDouble();
  const double upperLatitude  = it.nextDouble();
  const double upperLongitude = it.nextDouble();

  _sector = new Sector(Geodetic2D::fromRadians(lowerLatitude, lowerLongitude),
                       Geodetic2D::fromRadians(upperLatitude, upperLongitude));

  _minHeight = it.nextDouble();
  _maxHeight = it.nextDouble();

  const int leafNodesCount = it.nextInt32();
  std::vector<PointCloudLeafNode*> leafNodes;

  for (int i = 0; i < leafNodesCount; i++) {
    const int idLength = it.nextUInt8();
    unsigned char* id = new unsigned char[idLength];
    it.nextUInt8(idLength, id);

    const int byteLevelsCount = it.nextUInt8();
    const int shortLevelsCount = it.nextUInt8();
    const int intLevelsCount = it.nextUInt8();
    const int levelsCountLength = (int) byteLevelsCount + shortLevelsCount + intLevelsCount;

    int* levelsCount = new int[levelsCountLength];

    for (int j = 0; j < byteLevelsCount; j++) {
//      levelsCount.push_back( (int) it.nextUInt8() );
      levelsCount[j] = it.nextUInt8();
    }
    for (int j = 0; j < shortLevelsCount; j++) {
//      levelsCount.push_back( (int) it.nextInt16() );
      levelsCount[byteLevelsCount + j] = it.nextInt16();
    }
    for (int j = 0; j < intLevelsCount; j++) {
//      levelsCount.push_back( it.nextInt32() );
      levelsCount[byteLevelsCount + shortLevelsCount + j] =  it.nextInt32();
    }

    leafNodes.push_back( new PointCloudLeafNode(idLength,
                                                id,
                                                levelsCountLength,
                                                levelsCount) );
  }

  if (it.hasNext()) {
    THROW_EXCEPTION("Logic error");
  }

  if (leafNodesCount != leafNodes.size()) {
    THROW_EXCEPTION("Logic error");
  }

  delete _buffer;
  _buffer = NULL;

  _octree = new PointCloudOctreeInnerNode(0, new unsigned char[0]);
  for (int i = 0; i < leafNodesCount; i++) {
    _octree->addLeafNode( leafNodes[i] );
  }
}

PointCloudsRenderer::PointCloudOctreeInnerNode::~PointCloudOctreeInnerNode() {
  delete _children[0];
  delete _children[1];
  delete _children[2];
  delete _children[3];
}

void PointCloudsRenderer::PointCloudOctreeInnerNode::addLeafNode(PointCloudLeafNode* leafNode) {
  if ((_idLenght + 1) == leafNode->getIDLenght()) {
    const unsigned char childIndex = leafNode->getID()[_idLenght];
    if (_children[childIndex] != NULL) {
      THROW_EXCEPTION("Logic error!");
    }
    _children[childIndex] = leafNode;
  }
  else {
#warning DGD at work!

  }
}

void PointCloudsRenderer::PointCloudMetadataParserAsyncTask::onPostExecute(const G3MContext* context) {
  _pointCloud->parsedMetadata(_pointsCount, _sector, _minHeight, _maxHeight, _octree);
  _sector = NULL; // moves ownership to pointCloud
  _octree = NULL;  // moves ownership to pointCloud
}

void PointCloudsRenderer::PointCloud::parsedMetadata(long long pointsCount,
                                                     Sector* sector,
                                                     double minHeight,
                                                     double maxHeight,
                                                     PointCloudOctreeInnerNode* octree) {
  _pointsCount = pointsCount;
  _sector = sector;
  _minHeight = minHeight;
  _maxHeight = maxHeight;

  _downloadingMetadata = false;
  _octree = octree;

  ILogger::instance()->logInfo("Parsed metadata for \"%s\"", _cloudName.c_str());

  if (_metadataListener != NULL) {
    _metadataListener->onMetadata(pointsCount,
                                  *sector,
                                  minHeight,
                                  maxHeight);
    if (_deleteListener) {
      delete _metadataListener;
    }
    _metadataListener = NULL;
  }

}

//void PointCloudsRenderer::PointCloud::downloadedMetadata(IByteBuffer* buffer) {
//  ILogger::instance()->logInfo("Downloaded metadata for \"%s\" (bytes=%ld)", _cloudName.c_str(), buffer->size());
//
//  _threadUtils->invokeAsyncTask(new PointCloudMetadataParserAsyncTask(this, buffer),
//                                true);
//
//  //  _downloadingMetadata = false;
//  //
//  //
//  //#warning DGD at work!
//  ////  _errorParsingMetadata = true;
//  //
//  //  delete buffer;
//}

RenderState PointCloudsRenderer::PointCloud::getRenderState(const G3MRenderContext* rc) {
  if (_downloadingMetadata) {
    return RenderState::busy();
  }

  if (_errorDownloadingMetadata) {
    return RenderState::error("Error downloading metadata of \"" + _cloudName + "\" from \"" + _serverURL.getPath() + "\"");
  }

  if (_errorParsingMetadata) {
    return RenderState::error("Error parsing metadata of \"" + _cloudName + "\" from \"" + _serverURL.getPath() + "\"");
  }

  return RenderState::ready();
}

void PointCloudsRenderer::PointCloud::render(const G3MRenderContext* rc,
                                             GLState* glState) {


}

PointCloudsRenderer::~PointCloudsRenderer() {
  for (int i = 0; i < _cloudsSize; i++) {
    PointCloud* cloud = _clouds[i];
    delete cloud;
  }
#ifdef JAVA_CODE
  super.dispose();
#endif
}

void PointCloudsRenderer::onResizeViewportEvent(const G3MEventContext* ec,
                                                int width, int height) {

}

void PointCloudsRenderer::onChangedContext() {
  for (int i = 0; i < _cloudsSize; i++) {
    PointCloud* cloud = _clouds[i];
    cloud->initialize(_context);
  }
}

RenderState PointCloudsRenderer::getRenderState(const G3MRenderContext* rc) {
  _errors.clear();
  bool busyFlag  = false;
  bool errorFlag = false;

  for (int i = 0; i < _cloudsSize; i++) {
    PointCloud* cloud = _clouds[i];
    const RenderState childRenderState = cloud->getRenderState(rc);

    const RenderState_Type childRenderStateType = childRenderState._type;

    if (childRenderStateType == RENDER_ERROR) {
      errorFlag = true;

      const std::vector<std::string> childErrors = childRenderState.getErrors();
#ifdef C_CODE
      _errors.insert(_errors.end(),
                     childErrors.begin(),
                     childErrors.end());
#endif
#ifdef JAVA_CODE
      _errors.addAll(childErrors);
#endif
    }
    else if (childRenderStateType == RENDER_BUSY) {
      busyFlag = true;
    }
  }

  if (errorFlag) {
    return RenderState::error(_errors);
  }
  else if (busyFlag) {
    return RenderState::busy();
  }
  else {
    return RenderState::ready();
  }
}

void PointCloudsRenderer::addPointCloud(const URL& serverURL,
                                        const std::string& cloudName,
                                        PointCloudMetadataListener* metadataListener,
                                        bool deleteListener) {
  addPointCloud(serverURL,
                cloudName,
                DownloadPriority::MEDIUM,
                TimeInterval::fromDays(30),
                true,
                metadataListener,
                deleteListener);
}

void PointCloudsRenderer::addPointCloud(const URL& serverURL,
                                        const std::string& cloudName,
                                        long long downloadPriority,
                                        const TimeInterval& timeToCache,
                                        bool readExpired,
                                        PointCloudMetadataListener* metadataListener,
                                        bool deleteListener) {
  PointCloud* pointCloud = new PointCloud(serverURL, cloudName, downloadPriority, timeToCache, readExpired, metadataListener, deleteListener);
  if (_context != NULL) {
    pointCloud->initialize(_context);
  }
  _clouds.push_back(pointCloud);
  _cloudsSize = _clouds.size();
}

void PointCloudsRenderer::removeAllPointClouds() {
  for (int i = 0; i < _cloudsSize; i++) {
    PointCloud* cloud = _clouds[i];
    delete cloud;
  }
  _clouds.clear();
  _cloudsSize = _clouds.size();
}

void PointCloudsRenderer::render(const G3MRenderContext* rc,
                                 GLState* glState) {
  for (int i = 0; i < _cloudsSize; i++) {
    PointCloud* cloud = _clouds[i];
    cloud->render(rc, glState);
  }
}
