//
//  TileTexturizer.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 12/07/12.
//

#ifndef G3MiOSSDK_TileTexturizer
#define G3MiOSSDK_TileTexturizer

#include "Renderer.hpp"

class Mesh;
class G3MRenderContext;
class Tile;
class TileTessellator;
class G3MContext;
class TilesRenderParameters;
class Geodetic3D;
class LayerSet;
class LayerTilesRenderParameters;
class G3MEventContext;
class PlanetRenderContext;


class TileTexturizer {
public:
  virtual ~TileTexturizer() {
  }

  virtual RenderState getRenderState(LayerSet* layerSet) = 0;

  virtual void initialize(const G3MContext* context,
                          const TilesRenderParameters* parameters) = 0;

  virtual Mesh* texturize(const G3MRenderContext*    rc,
                          const PlanetRenderContext* prc,
                          Tile* tile,
                          Mesh* tessellatorMesh,
                          Mesh* previousMesh) = 0;

  virtual void tileToBeDeleted(Tile* tile,
                               Mesh* mesh) = 0;

  virtual void tileMeshToBeDeleted(Tile* tile,
                                   Mesh* mesh) = 0;

  virtual void justCreatedTopTile(const G3MRenderContext* rc,
                                  Tile* tile,
                                  LayerSet* layerSet) = 0;

  virtual void ancestorTexturedSolvedChanged(Tile* tile,
                                             Tile* ancestorTile,
                                             bool textureSolved) = 0;

  virtual bool onTerrainTouchEvent(const G3MEventContext* ec,
                                   const Geodetic3D& position,
                                   const Tile* tile,
                                   LayerSet* layerSet) = 0;
  
};

#endif
