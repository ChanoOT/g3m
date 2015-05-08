//
//  SphericalPlanet.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 31/05/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//

#include "SphericalPlanet.hpp"
#include "CameraEffects.hpp"

#include "Camera.hpp"


SphericalPlanet::SphericalPlanet(const Sphere& sphere):
_sphere(sphere),
_radii( Vector3D(sphere._radius, sphere._radius, sphere._radius) ),
_lastCorrectingRollAngle(0),
_prevFactor(0)
{
}

Vector3D SphericalPlanet::geodeticSurfaceNormal(const Angle& latitude,
                                                const Angle& longitude) const {
  const double cosLatitude = COS(latitude._radians);
  
  return Vector3D(cosLatitude * COS(longitude._radians),
                  cosLatitude * SIN(longitude._radians),
                  SIN(latitude._radians));
}

void SphericalPlanet::geodeticSurfaceNormal(const Angle& latitude,
                                            const Angle& longitude,
                                            MutableVector3D& result) const {
  const double cosLatitude = COS(latitude._radians);
  
  result.set(cosLatitude * COS(longitude._radians),
             cosLatitude * SIN(longitude._radians),
             SIN(latitude._radians));
}

std::vector<double> SphericalPlanet::intersectionsDistances(double originX,
                                                            double originY,
                                                            double originZ,
                                                            double directionX,
                                                            double directionY,
                                                            double directionZ) const {
  std::vector<double> intersections;
  
  // By laborious algebraic manipulation....
  const double a = directionX * directionX  + directionY * directionY + directionZ * directionZ;
  
  const double b = 2.0 * (originX * directionX + originY * directionY + originZ * directionZ);
  
  const double c = originX * originX + originY * originY + originZ * originZ - _sphere._radiusSquared;
  
  // Solve the quadratic equation: ax^2 + bx + c = 0.
  // Algorithm is from Wikipedia's "Quadratic equation" topic, and Wikipedia credits
  // Numerical Recipes in C, section 5.6: "Quadratic and Cubic Equations"
  const double discriminant = b * b - 4 * a * c;
  if (discriminant < 0.0) {
    // no intersections
    return intersections;
  }
  else if (discriminant == 0.0) {
    // one intersection at a tangent point
    //return new double[1] { -0.5 * b / a };
    intersections.push_back(-0.5 * b / a);
    return intersections;
  }
  
  const double rootDiscriminant = IMathUtils::instance()->sqrt(discriminant);
  const double root1 = (-b + rootDiscriminant) / (2*a);
  const double root2 = (-b - rootDiscriminant) / (2*a);
  
  // Two intersections - return the smallest first.
  if (root1 < root2) {
    intersections.push_back(root1);
    intersections.push_back(root2);
  }
  else {
    intersections.push_back(root2);
    intersections.push_back(root1);
  }
  return intersections;
}

Vector3D SphericalPlanet::toCartesian(const Angle& latitude,
                                      const Angle& longitude,
                                      const double height) const {
  return geodeticSurfaceNormal(latitude, longitude).times( _sphere._radius + height);
}

void SphericalPlanet::toCartesian(const Angle& latitude,
                                  const Angle& longitude,
                                  const double height,
                                  MutableVector3D& result) const {
  geodeticSurfaceNormal(latitude, longitude, result);
  const double nX = result.x();
  const double nY = result.y();
  const double nZ = result.z();
  
  const double K = _sphere._radius + height;
  result.set(nX * K,
             nY * K,
             nZ * K);
}

Geodetic2D SphericalPlanet::toGeodetic2D(const Vector3D& position) const {
  const Vector3D n = geodeticSurfaceNormal(position);
  
  const IMathUtils* mu = IMathUtils::instance();
  return Geodetic2D(Angle::fromRadians(mu->asin(n._z)),
                    Angle::fromRadians(mu->atan2(n._y, n._x)));
}


Geodetic3D SphericalPlanet::toGeodetic3D(const Vector3D& position) const {
  const Vector3D p = scaleToGeodeticSurface(position);
  const Vector3D h = position.sub(p);
  
  const double height = (h.dot(position) < 0) ? -1 * h.length() : h.length();
  
  return Geodetic3D(toGeodetic2D(p), height);
}


Vector3D SphericalPlanet::scaleToGeodeticSurface(const Vector3D& position) const {
  return geodeticSurfaceNormal(position).times( _sphere._radius );
}


Vector3D SphericalPlanet::scaleToGeocentricSurface(const Vector3D& position) const {
  return scaleToGeodeticSurface(position);
}


Geodetic2D SphericalPlanet::getMidPoint (const Geodetic2D& P0, const Geodetic2D& P1) const
{
  const Vector3D v0 = toCartesian(P0);
  const Vector3D v1 = toCartesian(P1);
  const Vector3D normal = v0.cross(v1).normalized();
  const Angle theta = v0.angleBetween(v1);
  const Vector3D midPoint = scaleToGeocentricSurface(v0.rotateAroundAxis(normal, theta.times(0.5)));
  return toGeodetic2D(midPoint);
}


std::list<Vector3D> SphericalPlanet::computeCurve(const Vector3D& start,
                                                  const Vector3D& stop,
                                                  double granularity) const {
  if (granularity <= 0.0) {
    //throw new ArgumentOutOfRangeException("granularity", "Granularity must be greater than zero.");
    return std::list<Vector3D>();
  }
  
  const Vector3D normal = start.cross(stop).normalized();
  const double theta = start.angleBetween(stop)._radians;
  
  //int n = max((int)(theta / granularity) - 1, 0);
  int n = ((int) (theta / granularity) - 1) > 0 ? (int) (theta / granularity) - 1 : 0;
  
  std::list<Vector3D> positions;
  
  positions.push_back(start);
  
  for (int i = 1; i <= n; ++i) {
    double phi = (i * granularity);
    
    positions.push_back(scaleToGeocentricSurface(start.rotateAroundAxis(normal, Angle::fromRadians(phi))));
  }
  
  positions.push_back(stop);
  
  return positions;
}


// compute distance from two points
double SphericalPlanet::computePreciseLatLonDistance(const Geodetic2D& g1,
                                                     const Geodetic2D& g2) const {
  const IMathUtils* mu = IMathUtils::instance();
  
  const double R = _sphere._radius;
  
  // spheric distance from P to Q
  // this is the right form, but it's the most complex
  // theres is a minimum error considering SphericalPlanet instead of ellipsoid
  const double latP = g2._latitude._radians;
  const double lonP = g2._longitude._radians;
  const double latQ = g1._latitude._radians;
  const double lonQ = g1._longitude._radians;
  const double coslatP = mu->cos(latP);
  const double sinlatP = mu->sin(latP);
  const double coslonP = mu->cos(lonP);
  const double sinlonP = mu->sin(lonP);
  const double coslatQ = mu->cos(latQ);
  const double sinlatQ = mu->sin(latQ);
  const double coslonQ = mu->cos(lonQ);
  const double sinlonQ = mu->sin(lonQ);
  const double pq = (coslatP * sinlonP * coslatQ * sinlonQ +
                     sinlatP * sinlatQ +
                     coslatP * coslonP * coslatQ * coslonQ);
  return mu->acos(pq) * R;
}


// compute distance from two points
double SphericalPlanet::computeFastLatLonDistance(const Geodetic2D& g1,
                                                  const Geodetic2D& g2) const {
  const IMathUtils* mu = IMathUtils::instance();
  
  const double R = _sphere._radius;
  
  const double medLat = g1._latitude._degrees;
  const double medLon = g1._longitude._degrees;
  
  // this way is faster, and works properly further away from the poles
  //double diflat = fabs(g._latitude-medLat);
  double diflat = mu->abs(g2._latitude._degrees - medLat);
  if (diflat > 180) {
    diflat = 360 - diflat;
  }
  
  double diflon = mu->abs(g2._longitude._degrees - medLon);
  if (diflon > 180) {
    diflon = 360 - diflon;
  }
  
  double dist = mu->sqrt(diflat * diflat + diflon * diflon);
  return dist * PI / 180 * R;
}


Vector3D SphericalPlanet::closestPointToSphere(const Vector3D& pos, const Vector3D& ray) const {
  const IMathUtils* mu = IMathUtils::instance();
  
  double t = 0;
  
  // compute radius for the rotation
  const double R0 = _sphere._radius;
  
  // compute the point in this ray that are to a distance R from the origin.
  const double U2 = ray.squaredLength();
  const double O2 = pos.squaredLength();
  const double OU = pos.dot(ray);
  const double a = U2;
  const double b = 2 * OU;
  const double c = O2 - R0 * R0;
  double rad = b * b - 4 * a * c;
  
  // if there is solution, the ray intersects the SphericalPlanet
  if (rad > 0) {
    // compute the final point (the smaller positive t value)
    t = (-b - mu->sqrt(rad)) / (2 * a);
    if (t < 1) t = (-b + mu->sqrt(rad)) / (2 * a);
    // if the ideal ray intersects, but not the mesh --> case 2
    if (t < 1) rad = -12345;
  }
  
  // if no solution found, find a point in the contour line
  if (rad < 0) {
    const double D = mu->sqrt(O2);
    const double co2 = R0 * R0 / (D * D);
    const double a_ = OU * OU - co2 * O2 * U2;
    const double b_ = 2 * OU * O2 - co2 * 2 * OU * O2;
    const double c_ = O2 * O2 - co2 * O2 * O2;
    const double rad_ = b_ * b_ - 4 * a_ * c_;
    t = (-b_ - mu->sqrt(rad_)) / (2 * a_);
  }
  
  // compute the final point
  Vector3D result = pos.add(ray.times(t));
  return result;
}

MutableMatrix44D SphericalPlanet::createGeodeticTransformMatrix(const Geodetic3D& position) const {
  const MutableMatrix44D translation = MutableMatrix44D::createTranslationMatrix( toCartesian(position) );
  const MutableMatrix44D rotation    = MutableMatrix44D::createGeodeticRotationMatrix( position );
  
  return translation.multiply(rotation);
}


void SphericalPlanet::beginSingleDrag(const Vector3D& origin, const Vector3D& touchedPosition) const
{
  _origin = origin.asMutableVector3D();
  //_initialPoint = closestIntersection(origin, initialRay).asMutableVector3D();
  _initialPoint = touchedPosition.asMutableVector3D();
  _dragRadius = _sphere._radius + toGeodetic3D(touchedPosition)._height;
  
  _validSingleDrag = false;
}

MutableMatrix44D SphericalPlanet::createDragMatrix(const Vector3D initialPoint,
                                                   const Vector3D finalPoint) const
{
  // compute the rotation axis
  const Vector3D rotationAxis = initialPoint.cross(finalPoint);
  
  // compute the angle
  double sinus = rotationAxis.length()/initialPoint.length()/finalPoint.length();
  const Angle rotationDelta = Angle::fromRadians(-IMathUtils::instance()->asin(sinus));
  if (rotationDelta.isNan()) return MutableMatrix44D::invalid();
  
  // save params for possible inertial animations
  //  _lastDragAxis = rotationAxis.asMutableVector3D();
  _lastDragAxis.copyFrom(rotationAxis);
  double radians = rotationDelta._radians;
  _lastDragRadiansStep = radians - _lastDragRadians;
  _lastDragRadians = radians;
  _validSingleDrag = true;
  
  // return rotation matrix
  return MutableMatrix44D::createRotationMatrix(rotationDelta, rotationAxis);
}

void SphericalPlanet::multByDragMatrix(const MutableVector3D& initialPoint,
                                       const MutableVector3D& finalPoint,
                                       MutableMatrix44D& matrix) {
  // compute the rotation axis
  const MutableVector3D rotationAxis = initialPoint.cross(finalPoint);
  
  // compute the angle
  double sinus = rotationAxis.length()/initialPoint.length()/finalPoint.length();
  double rotationDeltaInRadians = -IMathUtils::instance()->asin(sinus);
  if (rotationDeltaInRadians != rotationDeltaInRadians) {
    matrix.setInvalid();
    return;
  }
  
  // return rotation matrix
  matrix.createRotationMatrix(rotationDeltaInRadians, rotationAxis);
}


MutableMatrix44D SphericalPlanet::singleDrag(const Vector3D& finalRay) const
{
  // check if initialPoint is valid
  if (_initialPoint.isNan()) return MutableMatrix44D::invalid();
  
  // compute final point
  const Vector3D origin = _origin.asVector3D();
  MutableVector3D finalPoint = Sphere::closestIntersectionCenteredSphereWithRay(origin,
                                                                                finalRay,
                                                                                _dragRadius).asMutableVector3D();
  if (finalPoint.isNan()) {
    //printf ("--invalid final point in drag!!\n");
    //    finalPoint = closestPointToSphere(origin, finalRay).asMutableVector3D();
    finalPoint.copyFrom(closestPointToSphere(origin, finalRay));
    if (finalPoint.isNan()) {
      ILogger::instance()->logWarning("SphericalPlanet::singleDrag-> finalPoint is NaN");
      return MutableMatrix44D::invalid();
    }
  }
  
  return createDragMatrix(_initialPoint.asVector3D(), finalPoint.asVector3D());
}


Effect* SphericalPlanet::createEffectFromLastSingleDrag() const
{
  if (!_validSingleDrag || _lastDragAxis.isNan()) return NULL;
  return new RotateWithAxisEffect(_lastDragAxis.asVector3D(), Angle::fromRadians(_lastDragRadiansStep));
}


void SphericalPlanet::beginDoubleDrag(const Vector3D& origin,
                                      const Vector3D& centerRay,
                                      const Vector3D& centerPosition,
                                      const Vector3D& touchedPosition0,
                                      const Vector3D& touchedPosition1) const
{
  _origin = origin.asMutableVector3D();
  _centerRay = centerRay.normalized().asMutableVector3D();
  _initialPoint0 = touchedPosition0.asMutableVector3D();
  
  _dragRadius0 = _sphere._radius + toGeodetic3D(touchedPosition0)._height;
  _initialPoint1 = touchedPosition1.asMutableVector3D();
  _dragRadius1 = _sphere._radius + toGeodetic3D(touchedPosition1)._height;
  _centerPoint = centerPosition.asMutableVector3D();
  _lastDoubleDragAngle = 0;
  _lastCorrectingRollAngle = NAND;
  _lastCorrectingRollRotationAxis.set(NAND, NAND, NAND);
  
  _prevFactor = 0;
}

/*
double SphericalPlanet::testDoubleDragIteration(double factor,
                                                const Vector3D& finalRay0,
                                                const Vector3D& finalRay1,
                                                MutableMatrix44D& matrix) const
{
  MutableVector3D positionCamera = _origin;
  
  // move forward
  double distance = _centerPoint.sub(_origin).length();
  Vector3D traslation = _centerRay.asVector3D().times(distance*factor);
  matrix.copyValue(MutableMatrix44D::createTranslationMatrix(traslation));
  
  // compute new final points after moving forward
  const Vector3D traslatedOrigin = _origin.asVector3D().add(traslation);
  Vector3D finalPoint0 = Sphere::closestIntersectionCenteredSphereWithRay(traslatedOrigin,
                                                                          finalRay0, _dragRadius0); // A1
  if (finalPoint0.isNan()) {
    matrix.copyValue(MutableMatrix44D::invalid());
    return 0;
  }
  
  // drag initial point 0 to final point 0
  MutableMatrix44D dragMatrix = createDragMatrix(_initialPoint0.asVector3D(), finalPoint0);
  matrix.copyValueOfMultiplication(dragMatrix, matrix);
  
  // transform points to set axis origin in initialPoint0
  {
    Vector3D draggedCameraPos = positionCamera.transformedBy(matrix, 1.0).asVector3D();
    Vector3D finalPoint1 = Sphere::closestIntersectionCenteredSphereWithRay(draggedCameraPos,
                                                                            finalRay1.transformedBy(matrix, 0), _dragRadius1); // B1
    
    //Taking whole system to origin
    MutableMatrix44D M = createGeodeticTransformMatrix(toGeodetic3D(_initialPoint0.asVector3D()));
    MutableMatrix44D transform = M.inversed();
    
    Vector3D transformedInitialPoint1 = _initialPoint1.transformedBy(transform, 1.0).asVector3D();
    Vector3D transformedFinalPoint1 = finalPoint1.transformedBy(transform, 1.0);
    Vector3D transformedCameraPos = draggedCameraPos.transformedBy(transform, 1.0);
    Vector3D v0 = transformedFinalPoint1.sub(transformedCameraPos);
    
    //Angles to rotate transformedInitialPoint1 to adjust the plane that contains origin, TFP1 and TCP
    Vector3D planeNormal = transformedCameraPos.cross(v0).normalized();
    Plane plane(planeNormal, v0);
    Vector2D angles = plane.rotationAngleAroundZAxisToFixPointInRadians(transformedInitialPoint1);
    
    //Selecting best angle to rotate (smallest)
    double angulo1 = angles._x;
    double angulo2 = angles._y;
    double dif1 = Angle::distanceBetweenAnglesInRadians(angulo1, _lastDoubleDragAngle);
    double dif2 = Angle::distanceBetweenAnglesInRadians(angulo2, _lastDoubleDragAngle);
    double finalAngle = (dif1 < dif2)? angulo1 : angulo2;
    if (finalAngle != finalAngle) {
      matrix.copyValue(MutableMatrix44D::invalid());
      return 0;
    }
    
    //Creating rotation matrix
    _lastDoubleDragAngle = finalAngle;
    Vector3D normal0 = geodeticSurfaceNormal(_initialPoint0);
    MutableMatrix44D rotation = MutableMatrix44D::createGeneralRotationMatrix(Angle::fromRadians(-_lastDoubleDragAngle),normal0, _initialPoint0.asVector3D());
    matrix.copyValueOfMultiplication(rotation, matrix);// = rotation.multiply(matrix);
  }
  
  {
    Vector3D P0   = positionCamera.transformedBy(matrix, 1.0).asVector3D();
    Vector3D B    = _initialPoint1.asVector3D();
    Vector3D B0   = B.sub(P0);
    Vector3D Ra   = finalRay0.transformedBy(matrix, 0.0).normalized();
    Vector3D Rb   = finalRay1.transformedBy(matrix, 0.0).normalized();
    return Ra.angleBetween(B0)._degrees - Ra.angleBetween(Rb)._degrees;
  }
}
*/


double SphericalPlanet::testDoubleDragIteration(double factor,
                                                const Vector3D& finalRay0,
                                                const Vector3D& finalRay1,
                                                MutableMatrix44D& matrix) const
{
  MutableVector3D positionCamera = _origin;

  // move forward
  factor *= MutableVector3D::distanceBetween(_centerPoint, _origin);
  double traslationX = _centerRay.x() * factor;
  double traslationY = _centerRay.y() * factor;
  double traslationZ = _centerRay.z() * factor;
  matrix.setTranslationMatrix(traslationX, traslationY, traslationZ);
  
  // compute new final points after moving forward
  MutableVector3D finalPoint0; //A1
  Sphere::setClosestIntersectionCenteredSphereWithRay(_origin.x() + traslationX,
                                                      _origin.y() + traslationY,
                                                      _origin.z() + traslationZ,
                                                      finalRay0._x,
                                                      finalRay0._y,
                                                      finalRay0._z,
                                                      _dragRadius0,
                                                      finalPoint0);
  if (finalPoint0.isNan()) {
    matrix.setInvalid();
    return 0;
  }
  
  
  
  // AGUSTIN AT WORK REMOVING GARBAGE..............
  
  
  
  // drag initial point 0 to final point 0
  //MutableMatrix44D dragMatrix = createDragMatrix(_initialPoint0.asVector3D(), finalPoint0.asVector3D());
  MutableMatrix44D dragMatrix;
  multByDragMatrix(_initialPoint0, finalPoint0, dragMatrix);
  matrix.copyValueOfMultiplication(dragMatrix, matrix);
  
  // transform points to set axis origin in initialPoint0
  // (en el mundo plano es solo una traslacion)
  // (en el esférico será un cambio de sistema de referencia: traslacion + rotacion, usando el sistema local normal en ese punto)
  {
    Vector3D draggedCameraPos = positionCamera.transformedBy(matrix, 1.0).asVector3D();
    Vector3D finalPoint1 = Sphere::closestIntersectionCenteredSphereWithRay(draggedCameraPos,
                                                                            finalRay1.transformedBy(matrix, 0), _dragRadius1); // B1
    
    //Taking whole system to origin
    MutableMatrix44D M = createGeodeticTransformMatrix(toGeodetic3D(_initialPoint0.asVector3D()));
    MutableMatrix44D transform = M.inversed();
    
    Vector3D transformedInitialPoint1 = _initialPoint1.transformedBy(transform, 1.0).asVector3D();
    Vector3D transformedFinalPoint1 = finalPoint1.transformedBy(transform, 1.0);
    Vector3D transformedCameraPos = draggedCameraPos.transformedBy(transform, 1.0);
    Vector3D v0 = transformedFinalPoint1.sub(transformedCameraPos);
    
    //Angles to rotate transformedInitialPoint1 to adjust the plane that contains origin, TFP1 and TCP
    Vector3D planeNormal = transformedCameraPos.cross(v0).normalized();
    Plane plane(planeNormal, v0);
    Vector2D angles = plane.rotationAngleAroundZAxisToFixPointInRadians(transformedInitialPoint1);
    
    //Selecting best angle to rotate (smallest)
    double angulo1 = angles._x;
    double angulo2 = angles._y;
    double dif1 = Angle::distanceBetweenAnglesInRadians(angulo1, _lastDoubleDragAngle);
    double dif2 = Angle::distanceBetweenAnglesInRadians(angulo2, _lastDoubleDragAngle);
    double finalAngle = (dif1 < dif2)? angulo1 : angulo2;
    if (finalAngle != finalAngle) {
        matrix.setInvalid();
        return 0;
    }
    
    //Creating rotation matrix
    _lastDoubleDragAngle = finalAngle;
    Vector3D normal0 = geodeticSurfaceNormal(_initialPoint0);
    MutableMatrix44D rotation = MutableMatrix44D::createGeneralRotationMatrix(Angle::fromRadians(-_lastDoubleDragAngle),normal0, _initialPoint0.asVector3D());
    matrix.copyValueOfMultiplication(rotation, matrix);// = rotation.multiply(matrix);
  }
  
  {
    Vector3D P0   = positionCamera.transformedBy(matrix, 1.0).asVector3D();
    Vector3D B    = _initialPoint1.asVector3D();
    Vector3D B0   = B.sub(P0);
    Vector3D Ra   = finalRay0.transformedBy(matrix, 0.0).normalized();
    Vector3D Rb   = finalRay1.transformedBy(matrix, 0.0).normalized();
    return Ra.angleBetween(B0)._degrees - Ra.angleBetween(Rb)._degrees;
  }
}


MutableMatrix44D SphericalPlanet::doubleDrag(const Vector3D& finalRay0,
                                             const Vector3D& finalRay1) const
{
  // test if initialPoints are valid
  if (_initialPoint0.isNan() || _initialPoint1.isNan())
    return MutableMatrix44D::invalid();
  
  // init params
//  const IMathUtils* mu = IMathUtils::instance();
  MutableMatrix44D matrix;
  
  // search min & max factor for translation
  int iter = 0;
  double factor=0, minFactor, maxFactor;
  double deltaFactor = 0.04;
  double difAngleInDegrees = testDoubleDragIteration(_prevFactor, finalRay0, finalRay1, matrix);
  iter++;
  if (difAngleInDegrees != difAngleInDegrees)
    return MutableMatrix44D::invalid();
  if (difAngleInDegrees < 0) {
    minFactor = _prevFactor;
    maxFactor = _prevFactor + deltaFactor;
    difAngleInDegrees = testDoubleDragIteration(maxFactor, finalRay0, finalRay1, matrix);
    iter++;
    if (difAngleInDegrees != difAngleInDegrees)
      return MutableMatrix44D::invalid();
    while (difAngleInDegrees < 0) {
      maxFactor += deltaFactor;
      difAngleInDegrees = testDoubleDragIteration(maxFactor, finalRay0, finalRay1, matrix);
      iter++;
      if (difAngleInDegrees != difAngleInDegrees)
        return MutableMatrix44D::invalid();
    }
  } else {
    minFactor = _prevFactor - deltaFactor;
    maxFactor = _prevFactor;
    difAngleInDegrees = testDoubleDragIteration(minFactor, finalRay0, finalRay1, matrix);
    iter++;
    if (difAngleInDegrees != difAngleInDegrees)
      return MutableMatrix44D::invalid();
    while (difAngleInDegrees > 0) {
      minFactor -= deltaFactor;
      difAngleInDegrees = testDoubleDragIteration(minFactor, finalRay0, finalRay1, matrix);
      iter++;
      if (difAngleInDegrees != difAngleInDegrees)
        return MutableMatrix44D::invalid();
    }
  }
  
  // iterations until find right translation distance
  while (maxFactor-minFactor > 1e-3) {
    factor = (minFactor + maxFactor) / 2;
    difAngleInDegrees = testDoubleDragIteration(factor, finalRay0, finalRay1, matrix);
    iter++;
    if (difAngleInDegrees != difAngleInDegrees) {
      return MutableMatrix44D::invalid();
    }
    //printf("difAngleInDegrees=%.4f.  Min=%.4f  Factor=%.4f  Max=%.4f)\n", difAngleInDegrees, minFactor, factor, maxFactor);
    if (difAngleInDegrees>0)
      maxFactor = factor;
    else
      minFactor = factor;
  }
  
  printf("Converge doble drag %d iteraciones con factor=%.3f\n", iter, factor);
  
  _prevFactor = factor;
  return matrix;
}


/*
 MutableMatrix44D SphericalPlanet::doubleDrag(const Vector3D& finalRay0,
 const Vector3D& finalRay1,
 bool allowRotation) const
 {
 // test if initialPoints are valid
 if (_initialPoint0.isNan() || _initialPoint1.isNan())
 return MutableMatrix44D::invalid();
 
 // init params
 const IMathUtils* mu = IMathUtils::instance();
 const Vector3D origin = _origin.asVector3D();
 MutableVector3D positionCamera = _origin;
 
 // compute final points
 Vector3D finalPoint0 = Sphere::closestIntersectionCenteredSphereWithRay(origin,
 finalRay0, _dragRadius0); // A1
 if (finalPoint0.isNan()) return MutableMatrix44D::invalid();
 
 // drag initial point 0 to final point 0
 MutableMatrix44D matrix = createDragMatrix(_initialPoint0.asVector3D(), finalPoint0);
 
 // transform points to set axis origin in initialPoint0
 // (en el mundo plano es solo una traslacion)
 // (en el esférico será un cambio de sistema de referencia: traslacion + rotacion, usando el sistema local normal en ese punto)
 {
 Vector3D draggedCameraPos = positionCamera.transformedBy(matrix, 1.0).asVector3D();
 Vector3D finalPoint1 = Sphere::closestIntersectionCenteredSphereWithRay(draggedCameraPos,
 finalRay1.transformedBy(matrix, 0), _dragRadius1); // B1
 
 //Taking whole system to origin
 MutableMatrix44D M = createGeodeticTransformMatrix(toGeodetic3D(_initialPoint0.asVector3D()));
 MutableMatrix44D transform = M.inversed();
 
 Vector3D transformedInitialPoint1 = _initialPoint1.transformedBy(transform, 1.0).asVector3D();
 Vector3D transformedFinalPoint1 = finalPoint1.transformedBy(transform, 1.0);
 Vector3D transformedCameraPos = draggedCameraPos.transformedBy(transform, 1.0);
 Vector3D v0 = transformedFinalPoint1.sub(transformedCameraPos);
 
 //Angles to rotate transformedInitialPoint1 to adjust the plane that contains origin, TFP1 and TCP
 Vector3D planeNormal = transformedCameraPos.cross(v0).normalized();
 Plane plane(planeNormal, v0);
 Vector2D angles = plane.rotationAngleAroundZAxisToFixPointInRadians(transformedInitialPoint1);
 
 //Selecting best angle to rotate (smallest)
 double angulo1 = angles._x;
 double angulo2 = angles._y;
 double dif1 = Angle::distanceBetweenAnglesInRadians(angulo1, _lastDoubleDragAngle);
 double dif2 = Angle::distanceBetweenAnglesInRadians(angulo2, _lastDoubleDragAngle);
 _lastDoubleDragAngle = (dif1 < dif2)? angulo1 : angulo2;
 
 //Creating rotating matrix
 Vector3D normal0 = geodeticSurfaceNormal(_initialPoint0);
 MutableMatrix44D rotation = MutableMatrix44D::createGeneralRotationMatrix(Angle::fromRadians(-_lastDoubleDragAngle),normal0, _initialPoint0.asVector3D());
 matrix.copyValueOfMultiplication(rotation, matrix);// = rotation.multiply(matrix);
 }
   
   
   {
     Vector3D P0   = positionCamera.transformedBy(matrix, 1.0).asVector3D();
     Vector3D B    = _initialPoint1.asVector3D();
     Vector3D B0   = B.sub(P0);
     Vector3D Ra   = finalRay0.transformedBy(matrix, 0.0).normalized();
     Vector3D Rb   = finalRay1.transformedBy(matrix, 0.0).normalized();
     printf("angulo buscado:%.4f. angulo final:%.4f,    ", Ra.angleBetween(Rb)._degrees, Ra.angleBetween(B0)._degrees);
   }

 // zoom camera (see chuleta en pdf)
 // ahora mismo lo que se hace es buscar cuánto acercar para que el angulo de las dos parejas de vectores
 // sea el mismo
 {
 Vector3D P0   = positionCamera.transformedBy(matrix, 1.0).asVector3D();
 Vector3D B    = _initialPoint1.asVector3D();
 Vector3D B0   = B.sub(P0);
 Vector3D Ra   = finalRay0.transformedBy(matrix, 0.0).normalized();
 Vector3D Rb   = finalRay1.transformedBy(matrix, 0.0).normalized();
 double b      = -2 * (B0.dot(Ra));
 double c      = B0.squaredLength();
 double k      = Ra.dot(B0);
 double RaRb2  = Ra.dot(Rb) * Ra.dot(Rb);
 double at     = RaRb2 - 1;
 double bt     = b*RaRb2 + 2*k;
 double ct     = c*RaRb2 - k*k;
 
 Vector2D sol = mu->solveSecondDegreeEquation(at, bt, ct);
 if (sol.isNan()){
 return MutableMatrix44D::invalid();
 }
 double t = sol._x;
 
 MutableMatrix44D zoom = MutableMatrix44D::createTranslationMatrix(Ra.times(t));
 matrix.copyValueOfMultiplication(zoom, matrix);// = zoom.multiply(matrix);
 }
   
   
   {
     Vector3D P0   = positionCamera.transformedBy(matrix, 1.0).asVector3D();
     Vector3D B    = _initialPoint1.asVector3D();
     Vector3D B0   = B.sub(P0);
     Vector3D Ra   = finalRay0.transformedBy(matrix, 0.0).normalized();
     Vector3D Rb   = finalRay1.transformedBy(matrix, 0.0).normalized();
     printf("angulo final:%.4f\n", Ra.angleBetween(B0)._degrees);
   }

 
 return matrix;
 }
*/



Effect* SphericalPlanet::createDoubleTapEffect(const Vector3D& origin,
                                               const Vector3D& centerRay,
                                               const Vector3D& touchedPosition) const
{
  //const Vector3D initialPoint = closestIntersection(origin, tapRay);
  if (touchedPosition.isNan()) return NULL;
  
  // compute central point of view
  //const Vector3D centerPoint = closestIntersection(origin, centerRay);
  double touchedHeight = toGeodetic3D(touchedPosition)._height;
  double dragRadius = _sphere._radius + touchedHeight;
  const Vector3D centerPoint = Sphere::closestIntersectionCenteredSphereWithRay(origin,
                                                                                centerRay,
                                                                                dragRadius);
  
  // compute drag parameters
  const IMathUtils* mu = IMathUtils::instance();
  const Vector3D axis = touchedPosition.cross(centerPoint);
  const Angle angle   = Angle::fromRadians(- mu->asin(axis.length()/touchedPosition.length()/centerPoint.length()));
  
  // compute zoom factor
  const double distanceToGround = toGeodetic3D(origin)._height - touchedHeight;
  const double distance = distanceToGround * 0.6;
  
  // create effect
  return new DoubleTapRotationEffect(TimeInterval::fromSeconds(0.75), axis, angle, distance);
}


double SphericalPlanet::distanceToHorizon(const Vector3D& position) const
{
  double R = _sphere._radius;
  double D = position.length();
  return sqrt(D*D - R*R);
}


MutableMatrix44D SphericalPlanet::drag(const Geodetic3D& origin, const Geodetic3D& destination) const
{
  const Vector3D P0 = toCartesian(origin);
  const Vector3D P1 = toCartesian(destination);
  const Vector3D axis = P0.cross(P1);
  if (axis.length()<1e-3) return MutableMatrix44D::invalid();
  
  const Angle angle = P0.angleBetween(P1);
  const MutableMatrix44D rotation = MutableMatrix44D::createRotationMatrix(angle, axis);
  const Vector3D rotatedP0 = P0.transformedBy(rotation, 1);
  const MutableMatrix44D traslation = MutableMatrix44D::createTranslationMatrix(P1.sub(rotatedP0));
  return traslation.multiply(rotation);
}

void SphericalPlanet::applyCameraConstrainers(const Camera* previousCamera,
                                              Camera* nextCamera) const {
  
  //  Vector3D pos = nextCamera->getCartesianPosition();
  //  Vector3D origin = _origin.asVector3D();
  //  double maxDist = _sphere.getRadius() * 5;
  //
  //  if (pos.distanceTo(origin) > maxDist) {
  //    nextCamera->copyFromForcingMatrixCreation(*previousCamera);
  ////    Vector3D pos2 = nextCamera->getCartesianPosition();
  ////    printf("TOO FAR %f -> pos2: %f\n", pos.distanceTo(origin) / maxDist, pos2.distanceTo(origin) / maxDist);
  //  }
  
}

void SphericalPlanet::correctPitchAfterDoubleDrag(Camera* camera, const Vector2F& finalPixel0, const Vector2F& finalPixel1) const{
  
  Vector3D finalPoint0 = camera->pixel2PlanetPoint(finalPixel0);
  Vector3D finalPoint1 = camera->pixel2PlanetPoint(finalPixel1);
  if (finalPoint0.isNan() || finalPoint1.isNan()){
    return;
  }
  
  //  printf("dist = %f\n", finalPoint0.distanceTo(finalPoint1));
  
  Vector3D axis = finalPoint0.sub(finalPoint1);
  
  
  //Avoiding big jumps
  bool axisCorrect = true;
  bool angleCorrect = true;
  if (_lastCorrectingRollRotationAxis.isNan()){
    _lastCorrectingRollRotationAxis.copyFrom(axis);
  } else{
    double axisDirectionJump = axis.angleBetween(_lastCorrectingRollRotationAxis.asVector3D())._degrees;
    axisCorrect = (axisDirectionJump < 5.0);
  }
  
  if (axisCorrect){
    
    //Taking axis to camera coordinate system
    MutableMatrix44D csm( *camera->getModelMatrix44D() );
    Vector3D axisCS = axis.transformedBy(csm, 0.0).normalized(); //ROTATION AXIS
    Vector3D rotationPointCS = finalPoint0.transformedBy(csm, 1.0); //ROTATION POINT
    Vector3D planetCenterCS = Vector3D::zero.transformedBy(csm, 1.0); //Point to be dragged
    
    //The angle should take the planet center to the center of the view (Plane ZY) -> X = 0
    
    double angleInRadians = _lastCorrectingRollAngle;
    
    std::vector<double> angs = planetCenterCS.rotationAngleInRadiansToYZPlane(axisCS, rotationPointCS);
    if (angs.size() > 0){
      
      Angle a0 = Angle::fromRadians(angs[0]);
      Angle a1 = Angle::fromRadians(angs[1]);
      Angle last = Angle::fromRadians(_lastCorrectingRollAngle);
      
      //angleInRadians = a0.distanceTo(last)._radians < a1.distanceTo(last)._radians? a0._radians : a1._radians;
      
      if (a0.distanceTo(last)._radians < a1.distanceTo(last)._radians)
        angleInRadians = a0._radians;
      else
        angleInRadians = a1._radians;
      
      angleInRadians *= -1; //Inverting for camera
      
      if (ISNAN(_lastCorrectingRollAngle)){
        _lastCorrectingRollAngle = angleInRadians;
      } else{
        double jump = Angle::fromRadians(angleInRadians).distanceTo(Angle::fromRadians(_lastCorrectingRollAngle))._degrees;
        angleCorrect = (jump < 20);
        //      if (jump > 20){
        //        printf("CORRECTED ROLL JUMPED %f DEGREES\n", jump);
        //        angleInRadians = _lastCorrectingRollAngle;
        //      }
      }
      
      
      
    } else{
      printf("NONE CORRECT ROLL ANGLE FOR THIS FRAME\n");
    }
    
    if (angleCorrect){ //In angle and axis haven't change much
      Angle angle = Angle::fromRadians(angleInRadians);
      
      //printf("CORRECTING ROLL %f GRAD\n", angle->_degrees);
      MutableMatrix44D m = MutableMatrix44D::createGeneralRotationMatrix(angle, axis, finalPoint0);
      camera->applyTransform(m);
      
      //Storing for next frame
      _lastCorrectingRollAngle = angle._radians;
      _lastCorrectingRollRotationAxis.copyFrom(axis);
    } else{
      printf("ROLL ANGLE JUMPED\n");
    }
    
  } else{
    printf("ROLL AXIS JUMPED\n");
  }
}


