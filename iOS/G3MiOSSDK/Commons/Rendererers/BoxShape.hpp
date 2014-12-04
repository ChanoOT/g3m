//
//  BoxShape.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 11/16/12.
//
//

#ifndef __G3MiOSSDK__BoxShape__
#define __G3MiOSSDK__BoxShape__

#include "AbstractMeshShape.hpp"
#include "Color.hpp"

class OrientedBox;


class BoxShape : public AbstractMeshShape {
private:
  double _extentX;
  double _extentY;
  double _extentZ;

//#ifdef C_CODE
//  const Quadric _frontQuadric;
//  const Quadric _backQuadric;
//  const Quadric _leftQuadric;
//  const Quadric _rightQuadric;
//  const Quadric _topQuadric;
//  const Quadric _bottomQuadric;
//#endif
//#ifdef JAVA_CODE
//  private final Quadric _frontQuadric;
//  private final Quadric _backQuadric;
//  private final Quadric _leftQuadric;
//  private final Quadric _rightQuadric;
//  private final Quadric _topQuadric;
//  private final Quadric _bottomQuadric;
//#endif
  
  
  OrientedBox* _boundingVolume;

  float _borderWidth;
  float _originalBorderWidth;
  
  bool _useNormals;

  Color* _surfaceColor;
  Color* _borderColor;
  Color* _originalBorderColor;

  Mesh* createBorderMesh(const G3MRenderContext* rc);
  Mesh* createSurfaceMesh(const G3MRenderContext* rc);

  Mesh* createSurfaceMeshWithNormals(const G3MRenderContext* rc);

protected:
  Mesh* createMesh(const G3MRenderContext* rc);
  BoundingVolume* getBoundingVolume(const G3MRenderContext *rc);

public:
  BoxShape(Geodetic3D* position,
           AltitudeMode altitudeMode,
           const Vector3D& extent,
           float borderWidth,
           const Color& surfaceColor,
           Color* borderColor = NULL,
           bool useNormals = true) :
  AbstractMeshShape(position, altitudeMode),
  _boundingVolume(NULL),
  _extentX(extent._x),
  _extentY(extent._y),
  _extentZ(extent._z),

//  _frontQuadric(Quadric::fromPlane(1, 0, 0, -extent._x/2)),
//  _backQuadric(Quadric::fromPlane(-1, 0, 0, -extent._x/2)),
//  _leftQuadric(Quadric::fromPlane(0, -1, 0, -extent._y/2)),
//  _rightQuadric(Quadric::fromPlane(0, 1, 0, -extent._y/2)),
//  _topQuadric(Quadric::fromPlane(0, 0, 1, -extent._z/2)),
//  _bottomQuadric(Quadric::fromPlane(0, 0, -1, -extent._z/2)),

  _borderWidth(borderWidth),
  _originalBorderWidth(borderWidth),
  _surfaceColor(new Color(surfaceColor)),
  _borderColor(borderColor),
  _originalBorderColor((borderColor!=NULL)? new Color(*borderColor) : NULL),
  _useNormals(useNormals)
  {
    
  }

  ~BoxShape();
  
  void setExtent(const Vector3D& extent) {
    if ((_extentX != extent._x) ||
        (_extentY != extent._y) ||
        (_extentZ != extent._z)) {
      _extentX = extent._x;
      _extentY = extent._y;
      _extentZ = extent._z;
      cleanMesh();
    }
  }

  Vector3D getExtent() const {
    return Vector3D(_extentX, _extentY, _extentZ);
  }

  void setSurfaceColor(Color* color) {
    delete _surfaceColor;
    _surfaceColor = color;
    cleanMesh();
  }

  void setBorderColor(Color* color) {
    delete _borderColor;
    _borderColor = color;
    cleanMesh();
  }

  void setBorderWidth(float borderWidth) {
    if (_borderWidth != borderWidth) {
      _borderWidth = borderWidth;
      cleanMesh();
    }
  }
  
  std::vector<double> intersectionsDistances(const Planet* planet,
                                             const Camera* camera,
                                             const Vector3D& origin,
                                             const Vector3D& direction);
  
  bool isVisible(const G3MRenderContext *rc);
  
  void setSelectedDrawMode(bool mode) {
    if (mode) {
      setBorderWidth(7);
      setBorderColor(Color::newFromRGBA(1, 1, 0, 1));
    } else {
      setBorderWidth(_originalBorderWidth);
      if (_originalBorderColor!=NULL)
        setBorderColor(new Color(*_originalBorderColor));
    }
  }

};

#endif
