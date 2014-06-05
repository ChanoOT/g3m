package org.glob3.mobile.generated; 
public class Camera
{
  public Camera(Camera that)
  {
     _viewPortWidth = that._viewPortWidth;
     _viewPortHeight = that._viewPortHeight;
     _planet = that._planet;
     _position = new MutableVector3D(that._position);
     _groundHeight = that._groundHeight;
     _center = new MutableVector3D(that._center);
     _up = new MutableVector3D(that._up);
     _dirtyFlags = new CameraDirtyFlags(that._dirtyFlags);
     _frustumData = new FrustumData(that._frustumData);
     _projectionMatrix = new MutableMatrix44D(that._projectionMatrix);
     _modelMatrix = new MutableMatrix44D(that._modelMatrix);
     _modelViewMatrix = new MutableMatrix44D(that._modelViewMatrix);
     _cartesianCenterOfView = new MutableVector3D(that._cartesianCenterOfView);
     _geodeticCenterOfView = (that._geodeticCenterOfView == null) ? null : new Geodetic3D(that._geodeticCenterOfView);
     _frustum = (that._frustum == null) ? null : new Frustum(that._frustum);
     _frustumInModelCoordinates = (that._frustumInModelCoordinates == null) ? null : new Frustum(that._frustumInModelCoordinates);
     _widerFrustumInModelCoordinates = null;
     _camEffectTarget = new CameraEffectTarget();
     _geodeticPosition = (that._geodeticPosition == null) ? null: new Geodetic3D(that._geodeticPosition);
     _angle2Horizon = that._angle2Horizon;
     _normalizedPosition = new MutableVector3D(that._normalizedPosition);
     _tanHalfVerticalFieldOfView = java.lang.Double.NaN;
     _tanHalfHorizontalFieldOfView = java.lang.Double.NaN;
     _rollInRadians = that._rollInRadians;
  }

  public Camera()
  {
     _planet = null;
     _position = new MutableVector3D(0, 0, 0);
     _groundHeight = 0;
     _center = new MutableVector3D(0, 0, 0);
     _up = new MutableVector3D(0, 0, 1);
     _dirtyFlags = new CameraDirtyFlags();
     _frustumData = new FrustumData();
     _projectionMatrix = new MutableMatrix44D();
     _modelMatrix = new MutableMatrix44D();
     _modelViewMatrix = new MutableMatrix44D();
     _cartesianCenterOfView = new MutableVector3D(0,0,0);
     _geodeticCenterOfView = null;
     _frustum = null;
     _frustumInModelCoordinates = null;
     _camEffectTarget = new CameraEffectTarget();
     _geodeticPosition = null;
     _angle2Horizon = -99;
     _normalizedPosition = new MutableVector3D(0, 0, 0);
     _tanHalfVerticalFieldOfView = java.lang.Double.NaN;
     _tanHalfHorizontalFieldOfView = java.lang.Double.NaN;
     _rollInRadians = 0;
    resizeViewport(0, 0);
    _dirtyFlags.setAll(true);
  }

  public void dispose()
  {
    if (_camEffectTarget != null)
       _camEffectTarget.dispose();
    if (_frustum != null)
       _frustum.dispose();
    if (_frustumInModelCoordinates != null)
       _frustumInModelCoordinates.dispose();
    if (_widerFrustumInModelCoordinates != null)
       _widerFrustumInModelCoordinates.dispose();
    if (_geodeticCenterOfView != null)
       _geodeticCenterOfView.dispose();
    if (_geodeticPosition != null)
       _geodeticPosition.dispose();
  }

  public final void copyFrom(Camera that)
  {
    //TODO: IMPROVE PERFORMANCE
    _viewPortWidth = that._viewPortWidth;
    _viewPortHeight = that._viewPortHeight;
  
    _planet = that._planet;
  
    _position = new MutableVector3D(that._position);
    _center = new MutableVector3D(that._center);
    _up = new MutableVector3D(that._up);
    _normalizedPosition = new MutableVector3D(that._normalizedPosition);
  
    _dirtyFlags.copyFrom(that._dirtyFlags);
  
    _frustumData = new FrustumData(that._frustumData);
  
    _projectionMatrix.copyValue(that._projectionMatrix);
    _modelMatrix.copyValue(that._modelMatrix);
    _modelViewMatrix.copyValue(that._modelViewMatrix);
  
    _cartesianCenterOfView = new MutableVector3D(that._cartesianCenterOfView);
  
    if (_geodeticCenterOfView != null)
       _geodeticCenterOfView.dispose();
    _geodeticCenterOfView = (that._geodeticCenterOfView == null) ? null : new Geodetic3D(that._geodeticCenterOfView);
  
    if (_frustum != null)
       _frustum.dispose();
    _frustum = (that._frustum == null) ? null : new Frustum(that._frustum);
  
    if (_frustumInModelCoordinates != null)
       _frustumInModelCoordinates.dispose();
    _frustumInModelCoordinates = (that._frustumInModelCoordinates == null) ? null : new Frustum(that._frustumInModelCoordinates);
  
    if (_geodeticPosition != null)
       _geodeticPosition.dispose();
    _geodeticPosition = ((that._geodeticPosition == null) ? null : new Geodetic3D(that._geodeticPosition));
    _angle2Horizon = that._angle2Horizon;
  
    _tanHalfVerticalFieldOfView = that._tanHalfVerticalFieldOfView;
    _tanHalfHorizontalFieldOfView = that._tanHalfHorizontalFieldOfView;
  }

  public final void copyFromForcingMatrixCreation(Camera c)
  {
    c.forceMatrixCreation();
    copyFrom(c);
  }

  public final void resizeViewport(int width, int height)
  {
    _viewPortWidth = width;
    _viewPortHeight = height;
  
    _dirtyFlags.setAll(true);
  }

  public final Vector3D pixel2Ray(Vector2I pixel)
  {
    final double px = pixel._x + 0.5;
    final double py = _viewPortHeight - pixel._y - 0.5;
    final Vector3D pixel3D = new Vector3D(px, py, 0);
  
    final Vector3D obj = getModelViewMatrix().unproject(pixel3D, 0, 0, _viewPortWidth, _viewPortHeight);
    if (obj.isNan())
    {
      return obj;
    }
  
    return obj.sub(_position.asVector3D());
  }

  //void Camera::render(const G3MRenderContext* rc,
  //                    const GLGlobalState& parentState) const {
  //  //TODO: NO LONGER NEEDED!!!
  //}
  
  public final Vector3D pixel2Ray(Vector3D pixel3D)
  {
  
    final Vector3D obj = getModelViewMatrix().unproject(pixel3D, 0, 0, _viewPortWidth, _viewPortHeight);
    if (obj.isNan())
    {
      return obj;
    }
  
    return obj.sub(_position.asVector3D());
  }

  public final Vector3D pixel2PlanetPoint(Vector2I pixel)
  {
    return _planet.closestIntersection(_position.asVector3D(), pixel2Ray(pixel));
  }

  public final Vector2F point2Pixel(Vector3D point)
  {
    final Vector2D p = getModelViewMatrix().project(point, 0, 0, _viewPortWidth, _viewPortHeight);
  
    Vector3D direction = point.sub(getCartesianPosition());
    double angle = direction.angleBetween(getViewDirection())._degrees;
    if (angle > 90) //Projecting point behind the camera
    {
      return new Vector2F((float)-p._x, (float)-(_viewPortHeight - p._y));
    }
  
    return new Vector2F((float) p._x, (float)(_viewPortHeight - p._y));
  }
  public final Vector2F point2Pixel(Vector3F point)
  {
    final Vector2F p = getModelViewMatrix().project(point, 0, 0, _viewPortWidth, _viewPortHeight);
  
    Vector3D direction = point.asVector3D().sub(getCartesianPosition());
    double angle = direction.angleBetween(getViewDirection())._degrees;
    if (angle > 90) //Projecting point behind the camera
    {
      return new Vector2F((float)-p._x, (float)-(_viewPortHeight - p._y));
    }
  
    return new Vector2F(p._x, (_viewPortHeight - p._y));
  }

  public final int getViewPortWidth()
  {
     return _viewPortWidth;
  }
  public final int getViewPortHeight()
  {
     return _viewPortHeight;
  }

  public final float getViewPortRatio()
  {
    return (float) _viewPortWidth / _viewPortHeight;
  }

  public final EffectTarget getEffectTarget()
  {
    return _camEffectTarget;
  }

  public final Vector3D getCartesianPosition()
  {
     return _position.asVector3D();
  }
  public final Vector3D getNormalizedPosition()
  {
     return _normalizedPosition.asVector3D();
  }
  public final Vector3D getCenter()
  {
     return _center.asVector3D();
  }
  public final Vector3D getUp()
  {
     return _up.asVector3D();
  }
  public final Geodetic3D getGeodeticCenterOfView()
  {
     return _getGeodeticCenterOfView();
  }
  public final Vector3D getXYZCenterOfView()
  {
     return _getCartesianCenterOfView().asVector3D();
  }
  public final Vector3D getViewDirection()
  {
     return _center.sub(_position).asVector3D();
  }

  public final void dragCamera(Vector3D p0, Vector3D p1)
  {
    // compute the rotation axe
    final Vector3D rotationAxis = p0.cross(p1);
  
    // compute the angle
    //const Angle rotationDelta = Angle::fromRadians( - acos(p0.normalized().dot(p1.normalized())) );
    final Angle rotationDelta = Angle.fromRadians(-IMathUtils.instance().asin(rotationAxis.length()/p0.length()/p1.length()));
  
    if (rotationDelta.isNan())
    {
      return;
    }
  
    rotateWithAxis(rotationAxis, rotationDelta);
  }
  public final void rotateWithAxis(Vector3D axis, Angle delta)
  {
    applyTransform(MutableMatrix44D.createRotationMatrix(delta, axis));
  }
  public final void moveForward(double d)
  {
    final Vector3D view = getViewDirection().normalized();
    applyTransform(MutableMatrix44D.createTranslationMatrix(view.times(d)));
  }
  public final void translateCamera(Vector3D desp)
  {
    applyTransform(MutableMatrix44D.createTranslationMatrix(desp));
  }

  public final void pivotOnCenter(Angle a)
  {
    final Vector3D rotationAxis = _position.sub(_center).asVector3D();
    rotateWithAxis(rotationAxis, a);
  }

  public final void rotateWithAxisAndPoint(Vector3D axis, Vector3D point, Angle delta)
  {
    final MutableMatrix44D m = MutableMatrix44D.createGeneralRotationMatrix(delta, axis, point);
    applyTransform(m);
  }

  public final void print()
  {
    getModelMatrix().print("Model Matrix", ILogger.instance());
    getProjectionMatrix().print("Projection Matrix", ILogger.instance());
    getModelViewMatrix().print("ModelView Matrix", ILogger.instance());
    ILogger.instance().logInfo("Viewport width: %d, height %d\n", _viewPortWidth, _viewPortHeight);
  }

  public final Frustum getFrustumInModelCoordinates()
  {
    if (_dirtyFlags._frustumMCDirty)
    {
      _dirtyFlags._frustumMCDirty = false;
      if (_frustumInModelCoordinates != null)
         _frustumInModelCoordinates.dispose();
      _frustumInModelCoordinates = getFrustum().transformedBy_P(getModelMatrix());
    }
    return _frustumInModelCoordinates;
  }

  public final Frustum getWiderFrustumInModelCoordinates(float factor)
  {
    if (_widerFrustumInModelCoordinates != null)
      if (_widerFrustumInModelCoordinates != null)
         _widerFrustumInModelCoordinates.dispose();
    Frustum widerFrustum = new Frustum(calculateWiderFrustumData(factor));
    _widerFrustumInModelCoordinates = widerFrustum.transformedBy_P(getModelMatrix());
    return _widerFrustumInModelCoordinates;
  }

  public final Vector3D getHorizontalVector()
  {
    //int todo_remove_get_in_matrix;
    final MutableMatrix44D M = getModelMatrix();
    return new Vector3D(M.get0(), M.get4(), M.get8());
  }

  public final Angle compute3DAngularDistance(Vector2I pixel0, Vector2I pixel1)
  {
    final Vector3D point0 = pixel2PlanetPoint(pixel0);
    if (point0.isNan())
    {
      return Angle.nan();
    }
  
    final Vector3D point1 = pixel2PlanetPoint(pixel1);
    if (point1.isNan())
    {
      return Angle.nan();
    }
  
    return point0.angleBetween(point1);
  }

  public final void initialize(G3MContext context)
  {
    _planet = context.getPlanet();
    if (_planet.isFlat())
    {
      setCartesianPosition(new MutableVector3D(0, 0, _planet.getRadii()._y * 5));
      setUp(new MutableVector3D(0, 1, 0));
    }
    else
    {
      setCartesianPosition(new MutableVector3D(_planet.getRadii().maxAxis() * 5, 0, 0));
      setUp(new MutableVector3D(0, 0, 1));
    }
    _dirtyFlags.setAll(true);
  }

  public final void setCartesianPosition(MutableVector3D v)
  {
    if (!v.equalTo(_position))
    {
      _position = new MutableVector3D(v);
      if (_geodeticPosition != null)
         _geodeticPosition.dispose();
      _geodeticPosition = null;
      _dirtyFlags.setAll(true);
      final double distanceToPlanetCenter = _position.length();
      final double planetRadius = distanceToPlanetCenter - getGeodeticPosition()._height;
      _angle2Horizon = Math.acos(planetRadius/distanceToPlanetCenter);
      _normalizedPosition = _position.normalized();
    }
  }

  public final void setCartesianPosition(Vector3D v)
  {
    setCartesianPosition(v.asMutableVector3D());
  }

  public final Angle getHeading()
  {
    return getHeadingPitchRoll()._heading;
  }
  public final void setHeading(Angle angle)
  {
    //ILogger::instance()->logInfo("SET CAMERA HEADING: %f", angle._degrees);
    final TaitBryanAngles angles = getHeadingPitchRoll();
    final CoordinateSystem localRS = getLocalCoordinateSystem();
    final CoordinateSystem cameraRS = localRS.applyTaitBryanAngles(angle, angles._pitch, angles._roll);
    setCameraCoordinateSystem(cameraRS);
  }
  public final Angle getPitch()
  {
    return getHeadingPitchRoll()._pitch;
  }
  public final void setPitch(Angle angle)
  {
    //ILogger::instance()->logInfo("SET CAMERA PITCH: %f", angle._degrees);
    final TaitBryanAngles angles = getHeadingPitchRoll();
    final CoordinateSystem localRS = getLocalCoordinateSystem();
    final CoordinateSystem cameraRS = localRS.applyTaitBryanAngles(angles._heading, angle, angles._roll);
    setCameraCoordinateSystem(cameraRS);
  }

  public final Geodetic3D getGeodeticPosition()
  {
    if (_geodeticPosition == null)
    {
      _geodeticPosition = new Geodetic3D(_planet.toGeodetic3D(getCartesianPosition()));
    }
    return _geodeticPosition;
  }

  public final void setGeodeticPosition(Geodetic3D g3d)
  {
    final Angle heading = getHeading();
    final Angle pitch = getPitch();
    setPitch(Angle.fromDegrees(-90));
    MutableMatrix44D dragMatrix = _planet.drag(getGeodeticPosition(), g3d);
    if (dragMatrix.isValid())
       applyTransform(dragMatrix);
    setHeading(heading);
    setPitch(pitch);
  }

  public final void setGeodeticPositionStablePitch(Geodetic3D g3d)
  {
    MutableMatrix44D dragMatrix = _planet.drag(getGeodeticPosition(), g3d);
    if (dragMatrix.isValid())
       applyTransform(dragMatrix);
  }

  public final void setGeodeticPosition(Angle latitude, Angle longitude, double height)
  {
    setGeodeticPosition(new Geodetic3D(latitude, longitude, height));
  }

  public final void setGeodeticPosition(Geodetic2D g2d, double height)
  {
    setGeodeticPosition(new Geodetic3D(g2d, height));
  }

  /**
   This method put the camera pointing to given center, at the given distance, using the given angles.

   The situation is like in the figure of this url:
   http: //en.wikipedia.org/wiki/Azimuth

   At the end, camera will be in the 'Star' point, looking at the 'Observer' point.
   */
  public final void setPointOfView(Geodetic3D center, double distance, Angle azimuth, Angle altitude)
  {
    // TODO_deal_with_cases_when_center_in_poles
    final Vector3D cartesianCenter = _planet.toCartesian(center);
    final Vector3D normal = _planet.geodeticSurfaceNormal(center);
    final Vector3D north2D = _planet.getNorth().projectionInPlane(normal);
    final Vector3D orientedVector = north2D.rotateAroundAxis(normal, azimuth.times(-1));
    final Vector3D axis = orientedVector.cross(normal);
    final Vector3D finalVector = orientedVector.rotateAroundAxis(axis, altitude);
    final Vector3D position = cartesianCenter.add(finalVector.normalized().times(distance));
    final Vector3D finalUp = finalVector.rotateAroundAxis(axis, Angle.fromDegrees(90.0f));
    setCartesianPosition(position.asMutableVector3D());
    setCenter(cartesianCenter.asMutableVector3D());
    setUp(finalUp.asMutableVector3D());
    //  _dirtyFlags.setAll(true);
  }

  public final void forceMatrixCreation()
  {
    //MutableMatrix44D projectionMatrix = MutableMatrix44D::createProjectionMatrix(_frustumData);
    //getFrustumData();
    getProjectionMatrix44D();
    getModelMatrix44D();
    getModelViewMatrix().asMatrix44D();
  }

  public final Matrix44D getModelMatrix44D()
  {
    return getModelMatrix().asMatrix44D();
  }

  public final Matrix44D getProjectionMatrix44D()
  {
    return getProjectionMatrix().asMatrix44D();
  }

  public final Matrix44D getModelViewMatrix44D()
  {
    return getModelViewMatrix().asMatrix44D();
  }

  public final double getAngle2HorizonInRadians()
  {
     return _angle2Horizon;
  }

  public final double getProjectedSphereArea(Sphere sphere)
  {
    // this implementation is not right exact, but it's faster.
    final double z = sphere._center.distanceTo(getCartesianPosition());
    final double rWorld = sphere._radius * _frustumData._znear / z;
    final double rScreen = rWorld * _viewPortHeight / (_frustumData._top - _frustumData._bottom);
    return DefineConstants.PI * rScreen * rScreen;
  }

  public final void applyTransform(MutableMatrix44D M)
  {
    setCartesianPosition(_position.transformedBy(M, 1.0));
    setCenter(_center.transformedBy(M, 1.0));
  
    setUp(_up.transformedBy(M, 0.0));
  
    //_dirtyFlags.setAll(true);
  }

  public final boolean isPositionWithin(Sector sector, double height)
  {
    final Geodetic3D position = getGeodeticPosition();
    return sector.contains(position._latitude, position._longitude) && height >= position._height;
  }
  public final boolean isCenterOfViewWithin(Sector sector, double height)
  {
    final Geodetic3D position = getGeodeticCenterOfView();
    return sector.contains(position._latitude, position._longitude) && height >= position._height;
  }

  public final void setGroundHeightFromCartesianPoint(Vector3D point)
  {
    _groundHeight = _planet.toGeodetic3D(point)._height;
  }

  public final double getHeightFromGround()
  {
    return getGeodeticPosition()._height - _groundHeight;
  }

  //In case any of the angles is NAN it would be inferred considering the vieport ratio
  public final void setFOV(Angle vertical, Angle horizontal)
  {
    final Angle halfHFOV = horizontal.div(2.0);
    final Angle halfVFOV = vertical.div(2.0);
    final double newH = halfHFOV.tangent();
    final double newV = halfVFOV.tangent();
    if ((newH != _tanHalfHorizontalFieldOfView) || (newV != _tanHalfVerticalFieldOfView))
    {
      _tanHalfHorizontalFieldOfView = newH;
      _tanHalfVerticalFieldOfView = newV;
  
      _dirtyFlags._frustumDataDirty = true;
      _dirtyFlags._projectionMatrixDirty = true;
      _dirtyFlags._modelViewMatrixDirty = true;
      _dirtyFlags._frustumDirty = true;
      _dirtyFlags._frustumMCDirty = true;
    }
  }

  public final Angle getRoll()
  {
    return getHeadingPitchRoll()._roll;
  }
  public final void setRoll(Angle angle)
  {
    //ILogger::instance()->logInfo("SET CAMERA ROLL: %f", angle._degrees);
    final TaitBryanAngles angles = getHeadingPitchRoll();
  
    final CoordinateSystem localRS = getLocalCoordinateSystem();
    final CoordinateSystem cameraRS = localRS.applyTaitBryanAngles(angles._heading, angles._pitch, angle);
    setCameraCoordinateSystem(cameraRS);
  }

  public final CoordinateSystem getLocalCoordinateSystem()
  {
    return _planet.getCoordinateSystemAt(getGeodeticPosition());
  }
  public final CoordinateSystem getCameraCoordinateSystem()
  {
    return new CoordinateSystem(getViewDirection(), getUp(), getCartesianPosition());
  }
  public final TaitBryanAngles getHeadingPitchRoll()
  {
    final CoordinateSystem localRS = getLocalCoordinateSystem();
    final CoordinateSystem cameraRS = getCameraCoordinateSystem();
    return cameraRS.getTaitBryanAngles(localRS);
  }
  public final void setHeadingPitchRoll(Angle heading, Angle pitch, Angle roll)
  {
    final CoordinateSystem localRS = getLocalCoordinateSystem();
    final CoordinateSystem newCameraRS = localRS.applyTaitBryanAngles(heading, pitch, roll);
    setCameraCoordinateSystem(newCameraRS);
  }

  public final double getEstimatedPixelDistance(Vector3D point0, Vector3D point1)
  {
    final Vector3D ray0 = _position.sub(point0);
    final Vector3D ray1 = _position.sub(point1);
    final double angleInRadians = ray1.angleInRadiansBetween(ray0);
    final FrustumData frustumData = getFrustumData();
    final double distanceInMeters = frustumData._znear * IMathUtils.instance().tan(angleInRadians/2);
    return distanceInMeters * _viewPortHeight / frustumData._top;
  }

  //  const Angle getHeading(const Vector3D& normal) const;

  //IF A NEW ATTRIBUTE IS ADDED CHECK CONSTRUCTORS AND RESET() !!!!
  private int _viewPortWidth;
  private int _viewPortHeight;
  private Planet _planet;
  private MutableVector3D _position = new MutableVector3D(); // position
  private MutableVector3D _center = new MutableVector3D(); // point where camera is looking at
  private MutableVector3D _up = new MutableVector3D(); // vertical vector

  private Geodetic3D _geodeticPosition; //Must be updated when changing position

  private double _groundHeight;

  // this value is only used in the method Sector::isBackOriented
  // it's stored in double instead of Angle class to optimize performance in android
  // Must be updated when changing position
  private double _angle2Horizon;
  private MutableVector3D _normalizedPosition = new MutableVector3D();

  private CameraDirtyFlags _dirtyFlags = new CameraDirtyFlags();
  private FrustumData _frustumData = new FrustumData();
  private MutableMatrix44D _projectionMatrix = new MutableMatrix44D();
  private MutableMatrix44D _modelMatrix = new MutableMatrix44D();
  private MutableMatrix44D _modelViewMatrix = new MutableMatrix44D();
  private MutableVector3D _cartesianCenterOfView = new MutableVector3D();
  private Geodetic3D _geodeticCenterOfView;
  private Frustum _frustum;
  private Frustum _frustumInModelCoordinates;
  private Frustum _widerFrustumInModelCoordinates;
  private double _tanHalfVerticalFieldOfView;
  private double _tanHalfHorizontalFieldOfView;
  private double _rollInRadians;

  //The Camera Effect Target
  private static class CameraEffectTarget implements EffectTarget
  {
    public void dispose()
    {
    }
  }

  private CameraEffectTarget _camEffectTarget;

  private Vector3D centerOfViewOnPlanet()
  {
    return _planet.closestIntersection(_position.asVector3D(), getViewDirection());
  }

  private void setCenter(MutableVector3D v)
  {
    if (!v.equalTo(_center))
    {
      _center = new MutableVector3D(v);
      _dirtyFlags.setAll(true);
    }
  }

  private void setUp(MutableVector3D v)
  {
    if (!v.equalTo(_up))
    {
      _up = new MutableVector3D(v);
      _dirtyFlags.setAll(true);
    }
  }

  // data to compute frustum
  private FrustumData getFrustumData()
  {
    if (_dirtyFlags._frustumDataDirty)
    {
      _dirtyFlags._frustumDataDirty = false;
      _frustumData = calculateFrustumData();
    }
    return _frustumData;
  }

  // intersection of view direction with globe in(x,y,z)
  private MutableVector3D _getCartesianCenterOfView()
  {
    if (_dirtyFlags._cartesianCenterOfViewDirty)
    {
      _dirtyFlags._cartesianCenterOfViewDirty = false;
      _cartesianCenterOfView = centerOfViewOnPlanet().asMutableVector3D();
    }
    return _cartesianCenterOfView;
  }

  // intersection of view direction with globe in geodetic
  private Geodetic3D _getGeodeticCenterOfView()
  {
    if (_dirtyFlags._geodeticCenterOfViewDirty)
    {
      _dirtyFlags._geodeticCenterOfViewDirty = false;
      if (_geodeticCenterOfView != null)
         _geodeticCenterOfView.dispose();
      _geodeticCenterOfView = new Geodetic3D(_planet.toGeodetic3D(getXYZCenterOfView()));
    }
    return _geodeticCenterOfView;
  }

  // camera frustum
  private Frustum getFrustum()
  {
    if (_dirtyFlags._frustumDirty)
    {
      _dirtyFlags._frustumDirty = false;
      if (_frustum != null)
         _frustum.dispose();
      _frustum = new Frustum(getFrustumData());
    }
    return _frustum;
  }

  private FrustumData calculateFrustumData()
  {
    final double heightFromGround = getHeightFromGround();
  
    double zNear = heightFromGround * 0.1;
  
    //printf ("computing new znear=%.3f.  Height from ground =%.2f\n", zNear, heightFromGround);
  
    double zFar = _planet.distanceToHorizon(_position.asVector3D());
  
    final double goalRatio = 1000;
    final double ratio = zFar / zNear;
    if (ratio < goalRatio)
    {
      zNear = zFar / goalRatio;
    }
  
    //  int __TODO_remove_debug_code;
    //  printf(">>> height=%f zNear=%f zFar=%f ratio=%f\n",
    //         height,
    //         zNear,
    //         zFar,
    //         ratio);
  
    // compute rest of frustum numbers
  
    double tanHalfHFOV = _tanHalfHorizontalFieldOfView;
    double tanHalfVFOV = _tanHalfVerticalFieldOfView;
  
    if ((tanHalfHFOV != tanHalfHFOV) || (tanHalfVFOV != tanHalfVFOV))
    {
      final double ratioScreen = (double) _viewPortHeight / _viewPortWidth;
  
      if ((tanHalfHFOV != tanHalfHFOV) && (tanHalfVFOV != tanHalfVFOV))
      {
        tanHalfVFOV = 0.3; //Default behaviour _tanHalfFieldOfView = 0.3 => aprox tan(34 degrees / 2)
        tanHalfHFOV = tanHalfVFOV / ratioScreen;
      }
      else
      {
        if ((tanHalfHFOV != tanHalfHFOV))
        {
          tanHalfHFOV = tanHalfVFOV / ratioScreen;
        }
        else
        {
          if (tanHalfVFOV != tanHalfVFOV)
          {
            tanHalfVFOV = tanHalfHFOV * ratioScreen;
          }
        }
      }
    }
  
    final double right = tanHalfHFOV * zNear;
    final double left = -right;
    final double top = tanHalfVFOV * zNear;
    final double bottom = -top;
  
    return new FrustumData(left, right, bottom, top, zNear, zFar);
  
  }
  private FrustumData calculateWiderFrustumData(float factor)
  {
    final double heightFromGround = getHeightFromGround();
  
    double zNear = heightFromGround * 0.1;
  
    //printf ("computing new znear=%.3f.  Height from ground =%.2f\n", zNear, heightFromGround);
  
    double zFar = _planet.distanceToHorizon(_position.asVector3D());
  
    final double goalRatio = 1000;
    final double ratio = zFar / zNear;
    if (ratio < goalRatio)
    {
      zNear = zFar / goalRatio;
    }
  
    //  int __TODO_remove_debug_code;
    //  printf(">>> height=%f zNear=%f zFar=%f ratio=%f\n",
    //         height,
    //         zNear,
    //         zFar,
    //         ratio);
  
    // compute rest of frustum numbers
  
    double tanHalfHFOV = _tanHalfHorizontalFieldOfView;
    double tanHalfVFOV = _tanHalfVerticalFieldOfView;
  
    if ((tanHalfHFOV != tanHalfHFOV) || (tanHalfVFOV != tanHalfVFOV))
    {
      final double ratioScreen = (double) _viewPortHeight / _viewPortWidth;
  
      if ((tanHalfHFOV != tanHalfHFOV) && (tanHalfVFOV != tanHalfVFOV))
      {
        tanHalfVFOV = 0.3; //Default behaviour _tanHalfFieldOfView = 0.3 => aprox tan(34 degrees / 2)
        tanHalfHFOV = tanHalfVFOV / ratioScreen;
      }
      else
      {
        if ((tanHalfHFOV != tanHalfHFOV))
        {
          tanHalfHFOV = tanHalfVFOV / ratioScreen;
        }
        else
        {
          if (tanHalfVFOV != tanHalfVFOV)
          {
            tanHalfVFOV = tanHalfHFOV * ratioScreen;
          }
        }
      }
    }
  
    final double right = tanHalfHFOV * zNear;
    final double left = -right;
    final double top = tanHalfVFOV * zNear;
    final double bottom = -top;
  
    return new FrustumData(factor *left, factor *right, factor *bottom, factor *top, zNear, zFar);
  
  }

  // opengl projection matrix
  private MutableMatrix44D getProjectionMatrix()
  {
    if (_dirtyFlags._projectionMatrixDirty)
    {
      _dirtyFlags._projectionMatrixDirty = false;
      _projectionMatrix = MutableMatrix44D.createProjectionMatrix(getFrustumData());
    }
    return _projectionMatrix;
  }

  // Model matrix, computed in CPU in double precision
  private MutableMatrix44D getModelMatrix()
  {
    if (_dirtyFlags._modelMatrixDirty)
    {
      _dirtyFlags._modelMatrixDirty = false;
      _modelMatrix = MutableMatrix44D.createModelMatrix(_position, _center, _up);
    }
    return _modelMatrix;
  }

  // multiplication of model * projection
  private MutableMatrix44D getModelViewMatrix()
  {
    if (_dirtyFlags._modelViewMatrixDirty)
    {
      _dirtyFlags._modelViewMatrixDirty = false;
      _modelViewMatrix = getProjectionMatrix().multiply(getModelMatrix());
    }
    return _modelViewMatrix;
  }



  private void setCameraCoordinateSystem(CoordinateSystem rs)
  {
    _center = _position.add(rs._y.asMutableVector3D());
    _up = rs._z.asMutableVector3D();
    _dirtyFlags.setAll(true); //Recalculate Everything
  }

}