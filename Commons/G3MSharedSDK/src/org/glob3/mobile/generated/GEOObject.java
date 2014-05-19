package org.glob3.mobile.generated; 
//
//  GEOObject.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 11/29/12.
//
//


//class G3MRenderContext;
//class GEOSymbolizer;
//class MeshRenderer;
//class ShapesRenderer;
//class MarksRenderer;
//class GEOTileRasterizer;
//class GEORasterSymbolizer;
//class ICanvas;
//class GEORasterProjection;


public abstract class GEOObject
{
  public void dispose()
  {
  }

  public abstract void rasterize(GEORasterSymbolizer symbolizer, ICanvas canvas, GEORasterProjection projection, int tileLevel);

  public abstract void symbolize(G3MRenderContext rc, GEOSymbolizer symbolizer, MeshRenderer meshRenderer, ShapesRenderer shapesRenderer, MarksRenderer marksRenderer, GEOTileRasterizer geoTileRasterizer);

  public abstract long getCoordinatesCount();

}