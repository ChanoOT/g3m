package org.glob3.mobile.generated; 
//
//  TileTessellator.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 27/06/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//

//
//  TileTessellator.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 27/06/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//


//class G3MRenderContext;
//class Planet;
//class Mesh;
//class Tile;
//class MutableVector2D;
//class IFloatBuffer;
//class ElevationData;
//class Geodetic2D;
//class Sector;


public class TileTessellatorMeshData
{
  public double _minHeight;
  public double _maxHeight;
  public double _averageHeight;
  public double _deviation;
  public double _maxVerticesDistanceInLongitude;
  public double _maxVerticesDistanceInLatitude;
  public int _surfaceResolutionX;
  public int _surfaceResolutionY;
  public double _radius;

  public Vector3D _meshCenter;

  public TileTessellatorMeshData()
  {
     _meshCenter = null;

  }

  public void dispose()
  {
    if (_meshCenter != null)
       _meshCenter.dispose();
  }
}