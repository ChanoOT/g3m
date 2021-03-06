//
//  Tile.cpp
//  G3MiOSSDK
//
//  Created by Agustin Trujillo Pino on 12/06/12.
//

#include "Tile.hpp"

#include "Mesh.hpp"
#include "ITexturizerData.hpp"
#include "ElevationData.hpp"
#include "TileElevationDataRequest.hpp"
#include "PlanetTileTessellator.hpp"
#include "TileData.hpp"
#include "TileTexturizer.hpp"
#include "PlanetRenderContext.hpp"
#include "PlanetRenderer.hpp"
#include "MeshHolder.hpp"
#include "TileVisibilityTester.hpp"
#include "TileLODTester.hpp"
#include "TilesRenderParameters.hpp"
#include "MercatorUtils.hpp"
#include "LayerTilesRenderParameters.hpp"
#include "DecimatedSubviewElevationData.hpp"
#include "DEMProvider.hpp"
#include "DEMSubscription.hpp"
#include "IStringBuilder.hpp"
#include "ErrorHandling.hpp"
#include "IMathUtils.hpp"
#include "Geodetic3D.hpp"
#include "ITimer.hpp"
#include "Vector2D.hpp"
#include "DEMGrid.hpp"

//#include "BoundingVolume.hpp"
//#include "Color.hpp"


const std::string Tile::createTileID(int level,
                                     int row,
                                     int column) {
#ifdef C_CODE
  IStringBuilder* isb = IStringBuilder::newStringBuilder();
  isb->addInt(level);
  isb->addString("/");
  isb->addInt(row);
  isb->addString("/");
  isb->addInt(column);
  const std::string s = isb->getString();
  delete isb;
  return s;
#endif
#ifdef JAVA_CODE
  return level + "/" + row + "/" + column;
#endif
}

Tile::Tile(TileTexturizer* texturizer,
           Tile* parent,
           const Sector& sector,
           const bool mercator,
           int level,
           int row,
           int column,
           const PlanetRenderer* planetRenderer):
_texturizer(texturizer),
_parent(parent),
_sector(sector),
_mercator(mercator),
_level(level),
_row(row),
_column(column),
_tessellatorMesh(NULL),
_debugMesh(NULL),
_texturizedMesh(NULL),
_textureSolved(false),
_texturizerDirty(true),
_subtiles(NULL),
_justCreatedSubtiles(false),
_isVisible(false),
_texturizerData(NULL),
_elevationData(NULL),
_elevationDataLevel(-1),
_elevationDataRequest(NULL),
_grid(NULL),
_demSubscription(NULL),
_mustActualizeMeshDueToNewElevationData(false),
_lastTileMeshResolutionX(-1),
_lastTileMeshResolutionY(-1),
_planetRenderer(planetRenderer),
_planetTileTessellatorData(NULL),
_id( createTileID(level, row, column) ),
_data(NULL),
_dataSize(0)
{
}

Tile::~Tile() {
  //  prune(NULL, NULL);

  delete _debugMesh;

  delete _tessellatorMesh;

  delete _texturizerData;

  delete _texturizedMesh;

  delete _elevationData;

  if (_grid != NULL) {
    _grid->_release();
  }

  if (_elevationDataRequest != NULL) {
    _elevationDataRequest->cancelRequest(); //The listener will auto delete
    delete _elevationDataRequest;
  }

  if (_demSubscription != NULL) {
    _demSubscription->cancel();
    _demSubscription->_release();
    _demSubscription = NULL;
  }

  delete _planetTileTessellatorData;

  for (size_t i = 0; i < _dataSize; i++) {
    TileData* data = _data[i];
    delete data;
  }
  delete [] _data;
}

void Tile::setTexturizerData(ITexturizerData* texturizerData) {
  if (texturizerData != _texturizerData) {
    delete _texturizerData;
    _texturizerData = texturizerData;
  }
}


void Tile::ancestorTexturedSolvedChanged(Tile* ancestor,
                                         bool textureSolved) {
  if (textureSolved && isTextureSolved()) {
    return;
  }

  if (_texturizer != NULL) {
    _texturizer->ancestorTexturedSolvedChanged(this, ancestor, textureSolved);
  }

  if (_subtiles != NULL) {
    const size_t subtilesSize = _subtiles->size();
    for (size_t i = 0; i < subtilesSize; i++) {
      Tile* subtile = _subtiles->at(i);
      subtile->ancestorTexturedSolvedChanged(ancestor, textureSolved);
    }
  }
}

void Tile::setTextureSolved(bool textureSolved) {
  if (textureSolved != _textureSolved) {
    _textureSolved = textureSolved;

    if (_textureSolved) {
      delete _texturizerData;
      _texturizerData = NULL;
    }

    if (_subtiles != NULL) {
      const size_t subtilesSize = _subtiles->size();
      for (size_t i = 0; i < subtilesSize; i++) {
        Tile* subtile = _subtiles->at(i);
        subtile->ancestorTexturedSolvedChanged(this, _textureSolved);
      }
    }
  }
}

Tile::TerrainListener::TerrainListener(Tile* tile) :
_tile(tile)
{
}

Tile::TerrainListener::~TerrainListener() {
}

void Tile::TerrainListener::onGrid(DEMGrid* grid) {
  _tile->onGrid(grid);
}

Mesh* Tile::getTessellatorMesh(const G3MRenderContext* rc,
                               const PlanetRenderContext* prc) {

  ElevationDataProvider* elevationDataProvider = prc->_elevationDataProvider;

  if ((_elevationData == NULL) &&
      (elevationDataProvider != NULL) &&
      (elevationDataProvider->isEnabled())) {
    initializeElevationData(rc, prc);
  }

  DEMProvider* demProvider = prc->_demProvider;
  if (demProvider != NULL) {
    if (_demSubscription == NULL) {
      const Vector2S tileMeshResolution = prc->_layerTilesRenderParameters->_tileMeshResolution;

      _demSubscription = demProvider->subscribe(_sector,
                                                tileMeshResolution,
                                                new TerrainListener(this),
                                                true);
    }
  }

  if ( (_tessellatorMesh == NULL) || _mustActualizeMeshDueToNewElevationData ) {
    _mustActualizeMeshDueToNewElevationData = false;

    _planetRenderer->onTileHasChangedMesh(this);

    if (_debugMesh != NULL) {
      delete _debugMesh;
      _debugMesh = NULL;
    }

//    if (elevationDataProvider == NULL) {
//      // no elevation data provider, just create a simple mesh without elevation
//      _tessellatorMesh = prc->_tessellator->createTileMesh(rc,
//                                                           prc,
//                                                           this,
//                                                           NULL,
//                                                           _tileTessellatorMeshData);
//    }
//    else {
    Mesh* tessellatorMesh = prc->_tessellator->createTileMesh(rc,
                                                              prc,
                                                              this,
                                                              _elevationData,
                                                              _grid,
                                                              _tileTessellatorMeshData);

    MeshHolder* meshHolder = (MeshHolder*) _tessellatorMesh;
    if (meshHolder == NULL) {
      meshHolder = new MeshHolder(tessellatorMesh);
      _tessellatorMesh = meshHolder;
    }
    else {
      meshHolder->setMesh(tessellatorMesh);
    }
//    }

    //Notifying when the tile is first created and every time the elevation data changes
    _planetRenderer->sectorElevationChanged(_elevationData);
  }

  return _tessellatorMesh;
}

Mesh* Tile::getDebugMesh(const G3MRenderContext* rc,
                         const PlanetRenderContext* prc) {
  if (_debugMesh == NULL) {
    _debugMesh = prc->_tessellator->createTileDebugMesh(rc, prc, this);
  }
  return _debugMesh;
}

void Tile::prepareForFullRendering(const G3MRenderContext*    rc,
                                   const PlanetRenderContext* prc) {

  Mesh* tessellatorMesh = getTessellatorMesh(rc, prc);
  if (tessellatorMesh == NULL) {
    return;
  }

  if (prc->_texturizer != NULL) {
    const bool needsToCallTexturizer = (_texturizedMesh == NULL) || isTexturizerDirty();

    if (needsToCallTexturizer) {
      _texturizedMesh = prc->_texturizer->texturize(rc,
                                                    prc,
                                                    this,
                                                    tessellatorMesh,
                                                    _texturizedMesh);
    }
  }
}

void Tile::rawRender(const G3MRenderContext*    rc,
                     const PlanetRenderContext* prc,
                     const GLState*             glState) {

  Mesh* tessellatorMesh = getTessellatorMesh(rc, prc);
  if (tessellatorMesh == NULL) {
    return;
  }

//  tessellatorMesh->getBoundingVolume()->render(rc,
//                                               glState,
//                                               Color::white());

  if (prc->_texturizer == NULL) {
    tessellatorMesh->render(rc, glState);
  }
  else {
    const bool needsToCallTexturizer = (_texturizedMesh == NULL) || isTexturizerDirty();

    if (needsToCallTexturizer) {
      _texturizedMesh = prc->_texturizer->texturize(rc,
                                                    prc,
                                                    this,
                                                    tessellatorMesh,
                                                    _texturizedMesh);
    }

    if (_texturizedMesh == NULL) {
      tessellatorMesh->render(rc, glState);
    }
    else {
      _texturizedMesh->render(rc, glState);
    }
  }
}

void Tile::debugRender(const G3MRenderContext*    rc,
                       const PlanetRenderContext* prc,
                       const GLState*             glState) {
  Mesh* debugMesh = getDebugMesh(rc, prc);
  if (debugMesh != NULL) {
    debugMesh->render(rc, glState);
  }
}

std::vector<Tile*>* Tile::getSubTiles() {
  if (_subtiles == NULL) {
    _subtiles = createSubTiles(true);
  }
  return _subtiles;
}

void Tile::toBeDeleted(TileTexturizer*        texturizer,
                       ElevationDataProvider* elevationDataProvider) {
  prune(texturizer, elevationDataProvider);

  if (texturizer != NULL) {
    texturizer->tileToBeDeleted(this, _texturizedMesh);
  }

  if (elevationDataProvider != NULL) {
    if (_elevationDataRequest != NULL) {
      _elevationDataRequest->cancelRequest();
    }
  }

  if (_demSubscription != NULL) {
    _demSubscription->cancel();
    _demSubscription->_release();
    _demSubscription = NULL;
  }
}

void Tile::prune(TileTexturizer*        texturizer,
                 ElevationDataProvider* elevationDataProvider) {

  if (_subtiles != NULL) {
    //Notifying elevation event when LOD decreases
    _planetRenderer->sectorElevationChanged(_elevationData);

    const size_t subtilesSize = _subtiles->size();
    for (size_t i = 0; i < subtilesSize; i++) {
      Tile* subtile = _subtiles->at(i);

      subtile->setIsVisible(false, texturizer);

      subtile->prune(texturizer, elevationDataProvider);
      if (texturizer != NULL) {
        texturizer->tileToBeDeleted(subtile, subtile->_texturizedMesh);
      }

      delete subtile;
    }

    delete _subtiles;
    _subtiles = NULL;
  }
}

void Tile::setIsVisible(bool isVisible,
                        TileTexturizer* texturizer) {
  if (_isVisible != isVisible) {
    _isVisible = isVisible;

    if (!_isVisible) {
      deleteTexturizedMesh(texturizer);
    }
  }
}

void Tile::deleteTexturizedMesh(TileTexturizer* texturizer) {
  // check for (_parent != NULL) to avoid deleting the firstLevel tiles.
  // in this case, the mesh is always loaded (as well as its texture) to be the last option
  // falback texture for any tile
  if ((_parent != NULL) && (_texturizedMesh != NULL)) {
    if (texturizer != NULL) {
      texturizer->tileMeshToBeDeleted(this, _texturizedMesh);
    }

    delete _texturizedMesh;
    _texturizedMesh = NULL;

    delete _texturizerData;
    _texturizerData = NULL;

    setTexturizerDirty(true);
    setTextureSolved(false);
  }
}

void Tile::render(const G3MRenderContext*    rc,
                  const PlanetRenderContext* prc,
                  const GLState*             parentState,
                  TilesStatistics*           tilesStatistics,
                  std::vector<Tile*>*        toVisitInNextIteration) {

  const bool visible = prc->_tileVisibilityTester->isVisible(rc, prc, this);
  bool rendered = false;
  if (visible) {
    setIsVisible(true, prc->_texturizer);

    rendered = (
                (toVisitInNextIteration == NULL)                           ||
                prc->_tileLODTester->meetsRenderCriteria(rc, prc, this)    ||
                (prc->_tilesRenderParameters->_incrementalTileQuality && !_textureSolved)
                );

    if (rendered) {
      if (prc->_renderTileMeshes) {
        rawRender(rc, prc, parentState);
      }
      if (prc->_tilesRenderParameters->_renderDebug) {
        debugRender(rc, prc, parentState);
      }

      prune(prc->_texturizer, prc->_elevationDataProvider);
      //TODO: AVISAR CAMBIO DE TERRENO
    }
    else {
      std::vector<Tile*>* subTiles = getSubTiles();
      if (_justCreatedSubtiles) {
        prc->_lastSplitTimer->start();
        _justCreatedSubtiles = false;
      }

      if (toVisitInNextIteration != NULL) {
        const size_t subTilesSize = subTiles->size();
        for (size_t i = 0; i < subTilesSize; i++) {
          Tile* subTile = subTiles->at(i);
          toVisitInNextIteration->push_back(subTile);
        }
      }
    }
  }
  else {
    setIsVisible(false, prc->_texturizer);

    prune(prc->_texturizer, prc->_elevationDataProvider);
    //TODO: AVISAR CAMBIO DE TERRENO
  }

  tilesStatistics->computeTileProcessed(this, visible, rendered);
}

Tile* Tile::createSubTile(const Sector& sector,
                          const int level,
                          const int row,
                          const int column,
                          bool setParent) {
  Tile* parent = setParent ? this : NULL;
  return new Tile(_texturizer,
                  parent,
                  sector,
                  _mercator,
                  level,
                  row, column,
                  _planetRenderer);
}

std::vector<Tile*>* Tile::createSubTiles(bool setParent) {
  _justCreatedSubtiles = true;

  const Geodetic2D lower = _sector._lower;
  const Geodetic2D upper = _sector._upper;
  const Angle splitLongitude = Angle::midAngle(lower._longitude,
                                               upper._longitude);

  const Angle splitLatitude = _mercator
  /*                               */ ? MercatorUtils::calculateSplitLatitude(lower._latitude,
                                                                              upper._latitude)
  /*                               */ : Angle::midAngle(lower._latitude,
                                                        upper._latitude);

  const int nextLevel = _level + 1;

  const int row2    = 2 * _row;
  const int column2 = 2 * _column;

  std::vector<Tile*>* subTiles = new std::vector<Tile*>();

  const Sector* renderedSector = _planetRenderer->getRenderedSector();

  const Sector s0(Geodetic2D(lower._latitude, lower._longitude),
                  Geodetic2D(splitLatitude,   splitLongitude));
  if (renderedSector == NULL || renderedSector->touchesWith(s0)) {
    subTiles->push_back( createSubTile(s0,
                                       nextLevel,
                                       row2,
                                       column2,
                                       setParent) );
  }

  const Sector s1(Geodetic2D(lower._latitude, splitLongitude),
                  Geodetic2D(splitLatitude,   upper._longitude));
  if (renderedSector == NULL || renderedSector->touchesWith(s1)) {
    subTiles->push_back( createSubTile(s1,
                                       nextLevel,
                                       row2,
                                       column2 + 1,
                                       setParent) );
  }

  const Sector s2(Geodetic2D(splitLatitude,   lower._longitude),
                  Geodetic2D(upper._latitude, splitLongitude));
  if (renderedSector == NULL || renderedSector->touchesWith(s2)) {
    subTiles->push_back( createSubTile(s2,
                                       nextLevel,
                                       row2 + 1,
                                       column2,
                                       setParent) );
  }

  const Sector s3(Geodetic2D(splitLatitude,   splitLongitude),
                  Geodetic2D(upper._latitude, upper._longitude));
  if (renderedSector == NULL || renderedSector->touchesWith(s3)) {
    subTiles->push_back( createSubTile(s3,
                                       nextLevel,
                                       row2 + 1,
                                       column2 + 1,
                                       setParent) );
  }

#ifdef JAVA_CODE
  subTiles.trimToSize();
#endif

  return subTiles;
}

const Tile* Tile::getDeepestTileContaining(const Geodetic3D& position) const {
  if (_sector.contains(position._latitude, position._longitude)) {
    if (_subtiles == NULL) {
      return this;
    }

    const size_t subtilesSize = _subtiles->size();
    for (size_t i = 0; i < subtilesSize; i++) {
      const Tile* subtile = _subtiles->at(i);
      const Tile* subtileResult = subtile->getDeepestTileContaining(position);
      if (subtileResult != NULL) {
        return subtileResult;
      }
    }
  }

  return NULL;
}

const std::string Tile::description() const {
  IStringBuilder* isb = IStringBuilder::newStringBuilder();
  isb->addString("(Tile");
  isb->addString(" level=");
  isb->addInt(_level);
  isb->addString(", row=");
  isb->addInt(_row);
  isb->addString(", column=");
  isb->addInt(_column);
  isb->addString(", sector=");
  isb->addString(_sector.description());
  isb->addString(")");
  const std::string s = isb->getString();
  delete isb;
  return s;
}

#pragma mark ElevationData methods

void Tile::onGrid(DEMGrid* grid) {
  if (grid != _grid) {
    if (_grid != NULL) {
      _grid->_release();
    }
    _grid = grid;
    _mustActualizeMeshDueToNewElevationData = true;
  }
}

void Tile::setElevationData(ElevationData* ed, int level) {
  if (_elevationDataLevel < level) {

    if (_elevationData != NULL) {
      delete _elevationData;
    }

    _elevationData = ed;
    _elevationDataLevel = level;
    _mustActualizeMeshDueToNewElevationData = true;

    //If the elevation belongs to tile's level, we notify the sub-tree
    if (isElevationDataSolved()) {
      if (_subtiles != NULL) {
        const size_t subtilesSize = _subtiles->size();
        for (size_t i = 0; i < subtilesSize; i++) {
          Tile* subtile = _subtiles->at(i);
          subtile->ancestorChangedElevationData(this);
        }
      }
    }

  }
}

void Tile::getElevationDataFromAncestor(const Vector2I& extent) {
  if (_elevationData == NULL) {
    Tile* ancestor = getParent();
    while ((ancestor != NULL) &&
           !ancestor->isElevationDataSolved()) {
      ancestor = ancestor->getParent();
    }

    if (ancestor != NULL) {
      ElevationData* subView = createElevationDataSubviewFromAncestor(ancestor);
      setElevationData(subView, ancestor->_level);
    }
  }
}

void Tile::initializeElevationData(const G3MRenderContext* rc,
                                   const PlanetRenderContext* prc) {

  const Vector2S tileMeshResolution = prc->_layerTilesRenderParameters->_tileMeshResolution;

  //Storing for subviewing
  _lastElevationDataProvider = prc->_elevationDataProvider;
  _lastTileMeshResolutionX = tileMeshResolution._x;
  _lastTileMeshResolutionY = tileMeshResolution._y;
  if (_elevationDataRequest == NULL) {

    const Vector2S res = prc->_tessellator->getTileMeshResolution(rc, prc, this);
#warning should ElevationData res should be short?
    _elevationDataRequest = new TileElevationDataRequest(this, res.asVector2I(), prc->_elevationDataProvider);
    _elevationDataRequest->sendRequest();
  }

  //If after petition we still have no data we request from ancestor (provider asynchronous)
  if (_elevationData == NULL) {
    getElevationDataFromAncestor(tileMeshResolution.asVector2I());
  }

}

void Tile::ancestorChangedElevationData(Tile* ancestor) {

  if (ancestor->_level > _elevationDataLevel) {
    ElevationData* subView = createElevationDataSubviewFromAncestor(ancestor);
    if (subView != NULL) {
      setElevationData(subView, ancestor->_level);
    }
  }

  if (_subtiles != NULL) {
    const size_t subtilesSize = _subtiles->size();
    for (size_t i = 0; i < subtilesSize; i++) {
      Tile* subtile = _subtiles->at(i);
      subtile->ancestorChangedElevationData(this);
    }
  }
}

ElevationData* Tile::createElevationDataSubviewFromAncestor(Tile* ancestor) const {
  ElevationData* ed = ancestor->getElevationData();

  if (ed == NULL) {
    ILogger::instance()->logError("Ancestor can't have undefined Elevation Data.");
    return NULL;
  }

  if (ed->getExtentWidth() < 1 || ed->getExtentHeight() < 1) {
    ILogger::instance()->logWarning("Tile too small for ancestor elevation data.");
    return NULL;
  }

  if ((_lastElevationDataProvider != NULL) &&
      (_lastTileMeshResolutionX > 0) &&
      (_lastTileMeshResolutionY > 0)) {
    return new DecimatedSubviewElevationData(ed,
                                             _sector,
                                             Vector2I(_lastTileMeshResolutionX, _lastTileMeshResolutionY));
  }

  ILogger::instance()->logError("Can't create subview of elevation data from ancestor");
  return NULL;

}

void Tile::setPlanetTileTessellatorData(PlanetTileTessellatorData* planetTileTessellatorData) {
  if (planetTileTessellatorData != _planetTileTessellatorData) {
    delete _planetTileTessellatorData;
    _planetTileTessellatorData = planetTileTessellatorData;
  }
}

Vector2I Tile::getNormalizedPixelFromPosition(const Geodetic2D& position,
                                              const Vector2I& tileDimension) const {
  const IMathUtils* math = IMathUtils::instance();
  const Vector2D uv = _sector.getUVCoordinates(position);
  return Vector2I(math->toInt(tileDimension._x * uv._x),
                  math->toInt(tileDimension._y * uv._y));
}

void Tile::setData(TileData* data) const {
  const int id = data->_id;
  const size_t requiredSize = id+1;
  if (_dataSize < requiredSize) {
    if (_dataSize == 0) {
      _data = new TileData*[requiredSize];
      _dataSize = requiredSize;
#ifdef C_CODE
      for (size_t i = 0; i < _dataSize; i++) {
        _data[i] = NULL;
      }
#endif
    }
    else {
      TileData** oldData = _data;
      const size_t oldDataSize = _dataSize;
      _data = new TileData*[requiredSize];
      _dataSize = requiredSize;
#ifdef C_CODE
      for (size_t i = 0; i < oldDataSize; i++) {
        _data[i] = oldData[i];
      }
      for (size_t i = oldDataSize; i < _dataSize; i++) {
        _data[i] = NULL;
      }
      delete [] oldData;
#endif
#ifdef JAVA_CODE
      System.arraycopy(oldData, 0, _data, 0, oldDataSize);
#endif
    }
  }

  TileData* current = _data[id];
  if (current != data) {
    delete current;
    _data[id] = data;
  }
}

void Tile::clearDataWithID(int id) const {
  if (_dataSize > id) {
    //Assuming we won't reduce _data capacity by erasing items
    delete _data[id];
    _data[id] = NULL;
  }
}

TileData* Tile::getData(int id) const {
  return (id >= _dataSize) ? NULL : _data[id];
}

const TileTessellatorMeshData* Tile::getTileTessellatorMeshData() const {
#warning ask JM
  return &_tileTessellatorMeshData;
}
