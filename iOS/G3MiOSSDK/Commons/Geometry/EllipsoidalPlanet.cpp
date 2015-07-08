//
//  EllipsoidalPlanet.cpp
//  G3MiOSSDK
//
//  Created by Jose Miguel SN on 17/05/13.
//
//

#include <stdio.h>

#include "EllipsoidalPlanet.hpp"
#include "CameraEffects.hpp"
#include "Camera.hpp"
#include "Sphere.hpp"


EllipsoidalPlanet::EllipsoidalPlanet(const Ellipsoid& ellipsoid):
_ellipsoid(ellipsoid),
_sphericalPlanetDoubleDragDelegate(NULL)
{
}


Vector3D EllipsoidalPlanet::geodeticSurfaceNormal(const Angle& latitude,
                                                  const Angle& longitude) const {
  const double cosLatitude = COS(latitude._radians);

  return Vector3D(cosLatitude * COS(longitude._radians),
                  cosLatitude * SIN(longitude._radians),
                  SIN(latitude._radians));
}

void EllipsoidalPlanet::geodeticSurfaceNormal(const Angle& latitude,
                                              const Angle& longitude,
                                              MutableVector3D& result) const {
  const double cosLatitude = COS(latitude._radians);

  result.set(cosLatitude * COS(longitude._radians),
             cosLatitude * SIN(longitude._radians),
             SIN(latitude._radians));
}

Vector3D EllipsoidalPlanet::toCartesian(const Angle& latitude,
                                        const Angle& longitude,
                                        const double height) const {
  const Vector3D n = geodeticSurfaceNormal(latitude, longitude);

  const Vector3D k = _ellipsoid._radiiSquared.times(n);
  const double gamma = IMathUtils::instance()->sqrt((k._x * n._x) +
                                                    (k._y * n._y) +
                                                    (k._z * n._z));

  const Vector3D rSurface = k.div(gamma);
  return rSurface.add(n.times(height));
}

void EllipsoidalPlanet::toCartesian(const Angle& latitude,
                                    const Angle& longitude,
                                    const double height,
                                    MutableVector3D& result) const {
  geodeticSurfaceNormal(latitude, longitude, result);
  const double nX = result.x();
  const double nY = result.y();
  const double nZ = result.z();

  const double kX = nX * _ellipsoid._radiiSquared._x;
  const double kY = nY * _ellipsoid._radiiSquared._y;
  const double kZ = nZ * _ellipsoid._radiiSquared._z;

  const double gamma = IMathUtils::instance()->sqrt((kX * nX) +
                                                    (kY * nY) +
                                                    (kZ * nZ));

  const double rSurfaceX = kX / gamma;
  const double rSurfaceY = kY / gamma;
  const double rSurfaceZ = kZ / gamma;

  result.set(rSurfaceX + (nX * height),
             rSurfaceY + (nY * height),
             rSurfaceZ + (nZ * height));
}

Geodetic2D EllipsoidalPlanet::toGeodetic2D(const Vector3D& positionOnEllipsoidalPlanet) const {
  const Vector3D n = geodeticSurfaceNormal(positionOnEllipsoidalPlanet);

  const IMathUtils* mu = IMathUtils::instance();
  return Geodetic2D(Angle::fromRadians(mu->asin(n._z)),
                    Angle::fromRadians(mu->atan2(n._y, n._x)));
}


Geodetic3D EllipsoidalPlanet::toGeodetic3D(const Vector3D& position) const {
  const Vector3D p = scaleToGeodeticSurface(position);
  const Vector3D h = position.sub(p);

  const double height = (h.dot(position) < 0) ? -1 * h.length() : h.length();

  return Geodetic3D(toGeodetic2D(p), height);
}


Vector3D EllipsoidalPlanet::scaleToGeodeticSurface(const Vector3D& position) const {
  const IMathUtils* mu = IMathUtils::instance();

  const Vector3D oneOverRadiiSquared = _ellipsoid._oneOverRadiiSquared;
  const Vector3D radiiSquared        = _ellipsoid._radiiSquared;
  const Vector3D radiiToTheFourth    = _ellipsoid._radiiToTheFourth;

  const double beta = 1.0 / mu->sqrt((position._x * position._x) * oneOverRadiiSquared._x +
                                     (position._y * position._y) * oneOverRadiiSquared._y +
                                     (position._z * position._z) * oneOverRadiiSquared._z);

  const double n = Vector3D(beta * position._x * oneOverRadiiSquared._x,
                            beta * position._y * oneOverRadiiSquared._y,
                            beta * position._z * oneOverRadiiSquared._z).length();

  double alpha = (1.0 - beta) * (position.length() / n);

  const double x2 = position._x * position._x;
  const double y2 = position._y * position._y;
  const double z2 = position._z * position._z;

  double da = 0.0;
  double db = 0.0;
  double dc = 0.0;

  double s = 0.0;
  double dSdA = 1.0;

  do {
    alpha -= (s / dSdA);

    da = 1.0 + (alpha * oneOverRadiiSquared._x);
    db = 1.0 + (alpha * oneOverRadiiSquared._y);
    dc = 1.0 + (alpha * oneOverRadiiSquared._z);

    const double da2 = da * da;
    const double db2 = db * db;
    const double dc2 = dc * dc;

    const double da3 = da * da2;
    const double db3 = db * db2;
    const double dc3 = dc * dc2;

    s = (x2 / (radiiSquared._x * da2) +
         y2 / (radiiSquared._y * db2) +
         z2 / (radiiSquared._z * dc2) - 1.0);

    dSdA = (-2.0 *
            (x2 / (radiiToTheFourth._x * da3) +
             y2 / (radiiToTheFourth._y * db3) +
             z2 / (radiiToTheFourth._z * dc3)));
  }
  while (mu->abs(s) > 1e-10);

  return Vector3D(position._x / da,
                  position._y / db,
                  position._z / dc);
}


Vector3D EllipsoidalPlanet::scaleToGeocentricSurface(const Vector3D& position) const {
  Vector3D oneOverRadiiSquared = _ellipsoid._oneOverRadiiSquared;

  const double beta = 1.0 / IMathUtils::instance()->sqrt((position._x * position._x) * oneOverRadiiSquared._x +
                                                         (position._y * position._y) * oneOverRadiiSquared._y +
                                                         (position._z * position._z) * oneOverRadiiSquared._z);

  return position.times(beta);
}


Geodetic2D EllipsoidalPlanet::getMidPoint (const Geodetic2D& P0,
                                           const Geodetic2D& P1) const {
  const Vector3D v0 = toCartesian(P0);
  const Vector3D v1 = toCartesian(P1);
  const Vector3D normal = v0.cross(v1).normalized();
  const Angle theta = v0.angleBetween(v1);
  const Vector3D midPoint = scaleToGeocentricSurface(v0.rotateAroundAxis(normal, theta.times(0.5)));
  return toGeodetic2D(midPoint);
}


std::list<Vector3D> EllipsoidalPlanet::computeCurve(const Vector3D& start,
                                                    const Vector3D& stop,
                                                    double granularity) const {
  if (granularity <= 0.0) {
    //throw new ArgumentOutOfRangeException("granularity", "Granularity must be greater than zero.");
    return std::list<Vector3D>();
  }

  const Vector3D normal = start.cross(stop).normalized();
  const double theta = start.angleInRadiansBetween(stop);

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
double EllipsoidalPlanet::computePreciseLatLonDistance(const Geodetic2D& g1,
                                                       const Geodetic2D& g2) const {
  const IMathUtils* mu = IMathUtils::instance();

  const Vector3D radius = _ellipsoid._radii;
  const double R = (radius._x + radius._y + radius._z) / 3;

  // spheric distance from P to Q
  // this is the right form, but it's the most complex
  // theres is a minimum error considering sphere instead of EllipsoidalPlanet
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
double EllipsoidalPlanet::computeFastLatLonDistance(const Geodetic2D& g1,
                                                    const Geodetic2D& g2) const {
  const IMathUtils* mu = IMathUtils::instance();

  const Vector3D radius = _ellipsoid._radii;
  const double R = (radius._x + radius._y + radius._z) / 3;

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


Vector3D EllipsoidalPlanet::closestPointToSphere(const Vector3D& pos, const Vector3D& ray) const {
  const IMathUtils* mu = IMathUtils::instance();

  double t = 0;

  // compute radius for the rotation
  const double R0 = _ellipsoid.getMeanRadius();

  // compute the point in this ray that are to a distance R from the origin.
  const double U2 = ray.squaredLength();
  const double O2 = pos.squaredLength();
  const double OU = pos.dot(ray);
  const double a = U2;
  const double b = 2 * OU;
  const double c = O2 - R0 * R0;
  double rad = b * b - 4 * a * c;

  // if there is solution, the ray intersects the sphere
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

MutableMatrix44D EllipsoidalPlanet::createGeodeticTransformMatrix(const Geodetic3D& position) const {
  const MutableMatrix44D translation = MutableMatrix44D::createTranslationMatrix( toCartesian(position) );
  const MutableMatrix44D rotation    = MutableMatrix44D::createGeodeticRotationMatrix( position );

  return translation.multiply(rotation);
}


void EllipsoidalPlanet::beginSingleDrag(const Vector3D& origin, const Vector3D& touchedPosition) const
{
  _origin = origin.asMutableVector3D();
  //_initialPoint = closestIntersection(origin, initialRay).asMutableVector3D();
  _initialPoint = touchedPosition.asMutableVector3D();
  Vector3D _originalRadiusSquared = _ellipsoid.getRadiiSquared();
  double _dragRadiusFactorSquared = touchedPosition._x * touchedPosition._x / _originalRadiusSquared._x +
                                    touchedPosition._y * touchedPosition._y / _originalRadiusSquared._y +
                                    touchedPosition._z * touchedPosition._z / _originalRadiusSquared._z;
  _oneOverDragRadiiSquared = MutableVector3D(1.0 / _dragRadiusFactorSquared / _originalRadiusSquared._x,
                                             1.0 / _dragRadiusFactorSquared / _originalRadiusSquared._y,
                                             1.0 / _dragRadiusFactorSquared / _originalRadiusSquared._z);

/*
  =======
//  _origin = origin.asMutableVector3D();
  _origin.copyFrom(origin);
//  _initialPoint = closestIntersection(origin, initialRay).asMutableVector3D();
  _initialPoint.copyFrom(closestIntersection(origin, initialRay));
>>>>>>> origin/purgatory
 */
  
  _validSingleDrag = false;
}


MutableMatrix44D EllipsoidalPlanet::singleDrag(const Vector3D& finalRay) const
{
  // test if initialPoint is valid
  if (_initialPoint.isNan()) return MutableMatrix44D::invalid();

  // compute final point
  const Vector3D origin = _origin.asVector3D();
  MutableVector3D finalPoint = Ellipsoid::closestIntersectionCenteredEllipsoidWithRay(origin,
                                                                                      finalRay,
                                                                                      _oneOverDragRadiiSquared.asVector3D()).asMutableVector3D();
  if (finalPoint.isNan()) {
    //printf ("--invalid final point in drag!!\n");
//    finalPoint = closestPointToSphere(origin, finalRay).asMutableVector3D();
    finalPoint.copyFrom(closestPointToSphere(origin, finalRay));
    if (finalPoint.isNan()) {
      ILogger::instance()->logWarning("EllipsoidalPlanet::singleDrag-> finalPoint is NaN");
      return MutableMatrix44D::invalid();
    }
  }

  // compute the rotation axis
  const Vector3D rotationAxis = _initialPoint.cross(finalPoint).asVector3D();

  // compute the angle
  double sinus = rotationAxis.length()/_initialPoint.length()/finalPoint.length();
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


Effect* EllipsoidalPlanet::createEffectFromLastSingleDrag() const
{
  if (!_validSingleDrag || _lastDragAxis.isNan()) return NULL;
  return new RotateWithAxisEffect(_lastDragAxis.asVector3D(), Angle::fromRadians(_lastDragRadiansStep));
}

//This version of double drag gesture assumes that ellipsoidal is almost spherical
//Subsequently many calculus have been simplified
void EllipsoidalPlanet::beginDoubleDrag(const Vector3D& origin,
                     const Vector3D& centerRay,
                     const Vector3D& centerPosition,
                     const Vector3D& touchedPosition0,
                     const Vector3D& touchedPosition1) const{

  if (_sphericalPlanetDoubleDragDelegate == NULL){
    _sphericalPlanetDoubleDragDelegate = new SphericalPlanet(Sphere(Vector3D::zero, _ellipsoid._radii.maxAxis()));
  }
  _sphericalPlanetDoubleDragDelegate->beginDoubleDrag(origin, centerRay, centerPosition, touchedPosition0, touchedPosition1);
}


MutableMatrix44D EllipsoidalPlanet::doubleDrag(const Vector3D& finalRay0,
                                               const Vector3D& finalRay1) const
{
  return _sphericalPlanetDoubleDragDelegate->doubleDrag(finalRay0, finalRay1);
}


Effect* EllipsoidalPlanet::createDoubleTapEffect(const Vector3D& origin,
                                                 const Vector3D& centerRay,
                                                 const Vector3D& touchedPosition) const
{
  //const Vector3D initialPoint = closestIntersection(origin, tapRay);
  if (touchedPosition.isNan()) return NULL;

  // compute central point of view
  //const Vector3D centerPoint = closestIntersection(origin, centerRay);
  Vector3D originalRadiusSquared = _ellipsoid.getRadiiSquared();
  double dragRadiusFactorSquared = touchedPosition._x * touchedPosition._x / originalRadiusSquared._x +
                                    touchedPosition._y * touchedPosition._y / originalRadiusSquared._y +
                                    touchedPosition._z * touchedPosition._z / originalRadiusSquared._z;
  Vector3D oneOverDragRadiiSquared = Vector3D(1.0 / dragRadiusFactorSquared / originalRadiusSquared._x,
                                              1.0 / dragRadiusFactorSquared / originalRadiusSquared._y,
                                              1.0 / dragRadiusFactorSquared / originalRadiusSquared._z);
  Vector3D centerPoint = Ellipsoid::closestIntersectionCenteredEllipsoidWithRay(origin,
                                                                                centerRay,
                                                                                oneOverDragRadiiSquared);

  // compute drag parameters
  const IMathUtils* mu = IMathUtils::instance();
  const Vector3D axis = touchedPosition.cross(centerPoint);
  const Angle angle   = Angle::fromRadians(- mu->asin(axis.length()/touchedPosition.length()/centerPoint.length()));

  // compute zoom factor
  const double distanceToGround   = toGeodetic3D(origin)._height - toGeodetic3D(touchedPosition)._height;
  const double distance = distanceToGround * 0.6;

  // create effect
  return new DoubleTapRotationEffect(TimeInterval::fromSeconds(0.75), axis, angle, distance);
}


double EllipsoidalPlanet::distanceToHorizon(const Vector3D& position) const
{
  double R = getRadii().minAxis();
  double D = position.length();
  return sqrt(D*D - R*R);
}


MutableMatrix44D EllipsoidalPlanet::drag(const Geodetic3D& origin, const Geodetic3D& destination) const
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

void EllipsoidalPlanet::applyCameraConstrainers(const Camera* previousCamera,
                                                Camera* nextCamera) const {

}


void EllipsoidalPlanet::correctPitchAfterDoubleDrag(Camera* camera, const Vector2F& finalPixel0, const Vector2F& finalPixel1) const{
  _sphericalPlanetDoubleDragDelegate->correctPitchAfterDoubleDrag(camera, finalPixel0, finalPixel1);
}


MutableMatrix44D EllipsoidalPlanet::zoomUsingMouseWheel(double factor,
                                                        const Vector3D& origin,
                                                        const Vector3D& centerRay,
                                                        const Vector3D& centerPosition,
                                                        const Vector3D& touchedPosition,
                                                        const Vector3D& finalRay) const {
  return _sphericalPlanetDoubleDragDelegate->zoomUsingMouseWheel(factor,
                                                                 origin,
                                                                 centerRay,
                                                                 centerPosition,
                                                                 touchedPosition,
                                                                 finalRay);
}


