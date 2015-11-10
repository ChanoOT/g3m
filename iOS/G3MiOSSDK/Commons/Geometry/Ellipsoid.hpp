//
//  Ellipsoid.h
//  G3MiOSSDK
//
//  Created by Jose Miguel SN on 04/06/13.
//
//

#ifndef __G3MiOSSDK__Ellipsoid__
#define __G3MiOSSDK__Ellipsoid__

#include <vector>

#include "Vector3D.hpp"


class Ellipsoid {
public:
  const Vector3D _center;
  const Vector3D _radii;

  const Vector3D _radiiSquared;
  const Vector3D _radiiToTheFourth;
  const Vector3D _oneOverRadiiSquared;


  Ellipsoid(const Vector3D& center,
            const Vector3D& radii):
  _center(center),
  _radii(radii),
  _radiiSquared(Vector3D(radii._x * radii._x ,
                         radii._y * radii._y,
                         radii._z * radii._z)),
  _radiiToTheFourth(Vector3D(_radiiSquared._x * _radiiSquared._x ,
                             _radiiSquared._y * _radiiSquared._y,
                             _radiiSquared._z * _radiiSquared._z)),
  _oneOverRadiiSquared(Vector3D(1.0 / (radii._x * radii._x ),
                                1.0 / (radii._y * radii._y),
                                1.0 / (radii._z * radii._z))) {}

  Vector3D getCenter() const {
    return _center;
  }

  Vector3D getRadii() const {
    return _radii;
  }

  Vector3D getRadiiSquared() const {
    return _radiiSquared;
  }

  Vector3D getRadiiToTheFourth() const {
    return _radiiToTheFourth;
  }

  Vector3D getOneOverRadiiSquared() const {
    return _oneOverRadiiSquared;
  }

  double getMeanRadius() const {
    return (_radii._x + _radii._y + _radii._y) / 3;
  }
  
  static std::vector<double> intersectionCenteredEllipsoidWithRay(const Vector3D& origin,
                                                                  const Vector3D& direction,
                                                                  const Vector3D& oneOverRadiiSquared);
  
  static Vector3D closestIntersectionCenteredEllipsoidWithRay(const Vector3D& origin,
                                                              const Vector3D& direction,
                                                              const Vector3D& oneOverRadiiSquared);

  std::vector<double> intersectionsDistances(const Vector3D& origin,
                                             const Vector3D& direction) const {
    return intersectionsDistances(origin._x,
                                  origin._y,
                                  origin._z,
                                  direction._x,
                                  direction._y,
                                  direction._z,
                                  _oneOverRadiiSquared._x,
                                  _oneOverRadiiSquared._y,
                                  _oneOverRadiiSquared._z);
  }

  static std::vector<double> intersectionsDistances(double originX,
                                             double originY,
                                             double originZ,
                                             double directionX,
                                             double directionY,
                                             double directionZ,
                                             double oneOverRadiiSquaredX,
                                             double oneOverRadiiSquaredY,
                                             double oneOverRadiiSquaredZ);
};

#endif
