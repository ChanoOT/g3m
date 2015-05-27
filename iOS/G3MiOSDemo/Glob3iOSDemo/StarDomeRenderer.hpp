//
//  StarDomeRenderer.hpp
//  G3MiOSDemo
//
//  Created by Jose Miguel SN on 19/3/15.
//
//

#ifndef __G3MiOSDemo__StarDomeRenderer__
#define __G3MiOSDemo__StarDomeRenderer__

#include "DefaultRenderer.hpp"
#include "GLState.hpp"

#include "MeshShape.hpp"
#include "Color.hpp"
#include <vector>
#include <string>

#include "IFactory.hpp"
#include "ITimer.hpp"
#include "Geodetic3D.hpp"
#include "Color.hpp"
#include "MarksRenderer.hpp"

#include "LabelImageBuilder.hpp"
#include "IImageBuilderListener.hpp"
#include "MutableVector3D.hpp"


class StarsMeshShape: public MeshShape{
public:
  StarsMeshShape(Geodetic3D* position,
                 AltitudeMode altitudeMode,
                 Mesh* mesh):
  MeshShape(position,altitudeMode, mesh){
    
  }
  
  std::vector<double> intersectionsDistances(const Planet* planet,
                                             const Vector3D& origin,
                                             const Vector3D& direction) const{
    return std::vector<double>();
  }
};


class Star{
  
  const double _ascencion;
  const double _declination;
  const std::string _name;
  
public:
  Color* _color;
  
  Star(const std::string& name,
       double ascencion,
       double declination,
       const Color& color):
  _name(name),
  _ascencion(ascencion),
  _declination(declination),
  _color(new Color(color)){
    
  }
  
  std::string getName() const{
    return _name;
  }
  
  double getTrueNorthAzimuthInDegrees(double siderealTime, const Geodetic2D viewerPosition) const;
  
  double getAltitude(double siderealTime, const Geodetic2D viewerPosition) const;
  
  Vector3D getStarDisplacementInDome(double domeHeight, double siderealTime, const Geodetic2D& domePos) const;
  
  ~Star(){
    delete _color;
  }
  
  Star& operator=(const Star& s){
    return *(new Star(s._name, s._ascencion, s._declination, *s._color));
  }
  
  Star(const Star& s):
  _name(s._name),
  _ascencion(s._ascencion),
  _declination(s._declination),
  _color(new Color(*s._color)){
    
  }
  
  void setColor(const Color& color){
    _color = new Color(color);
  }
  
  double distanceInDegrees(const Angle& trueNorthAzimuthInDegrees,
                           const Angle& altitudeInDegrees,
                           double siderealTime,
                           const Geodetic2D& viewerPosition){
    return Angle::fromDegrees(getAltitude(siderealTime, viewerPosition)).distanceTo(altitudeInDegrees)._degrees +
    Angle::fromDegrees(getTrueNorthAzimuthInDegrees(siderealTime, viewerPosition)).distanceTo(trueNorthAzimuthInDegrees)._degrees;
  }
  
};

class Constellation{
public:
  std::string _name;
  std::vector<Star> _stars;
  std::vector<int> _lines;
  Color* _color;
  
  Constellation(const std::string& name,
                const Color& color,
                const std::vector<Star> stars,
                const std::vector<int>& lines):
  _name(name), _color(new Color(color)), _stars(stars), _lines(lines){}
  
  Constellation(const Constellation& c){
    _name = c._name;
    _color = new Color(*c._color);
    _lines = c._lines;
    _stars = c._stars;
  }
  
  ~Constellation(){
    delete _color;
  }
};


class StarDomeRenderer : public DefaultRenderer {
  
  StarsMeshShape* _starsShape;
  
  GLState* _glState;
  
  std::vector<Star> _stars;
  
  std::vector<int> _lines;
  
  const Camera* _currentCamera;
  
  const std::string _name;
  
  Geodetic3D* _position;
  double _clockTimeInDegrees;
  
  int _dayOfTheYear;
  
  //Theta0 from first to last day of the year 2015
  static double theta0[];
  
  Color _color;
  
  MarksRenderer* _mr;
  
public:
  
  StarDomeRenderer(std::string& name, std::vector<Star> stars, std::vector<int> lines,
                   const Geodetic3D& position,
                   double clockTimeInDegrees,
                   int dayOfTheYear,
                   Color color,
                   MarksRenderer* mr = NULL):
  _starsShape(NULL),
  _glState(new GLState()),
  _stars(stars),
  _lines(lines),
  _currentCamera(NULL),
  _name(name),
  _position(new Geodetic3D(position)),
  _clockTimeInDegrees(clockTimeInDegrees),
  _dayOfTheYear(dayOfTheYear),
  _color(color),
  _mr(mr)
  {
  }
  
  ~StarDomeRenderer();
  
  double getCurrentClockTimeInDegrees() const{
    return _clockTimeInDegrees;
  }
  
  void setClockTimeInDegrees(double v){
    _clockTimeInDegrees = v;
  }
  
  void clear();
  
  static double getSiderealTimeInDegrees(double placeLongitudeInDegrees, double clockTimeInDegrees, int dayOfTheYear){
    
    double thetaZero = theta0[dayOfTheYear - 1] * 15;
    
    double I = 366.2422 / 365.2422;
    double TU = clockTimeInDegrees - IMathUtils::instance()->round(placeLongitudeInDegrees);
#warning CHECK WITH PILAR - 12 hours
    Angle a = Angle::fromDegrees(thetaZero + TU * I + placeLongitudeInDegrees);
    return a.normalized()._degrees;
  }
  
  void initialize(const G3MContext* context);
  
  void render(const G3MRenderContext* rc,
              GLState* glState);
  
  void onResizeViewportEvent(const G3MEventContext* ec,
                             int width, int height) {
  }
  
  void start(const G3MRenderContext* rc){}
  
  void stop(const G3MRenderContext* rc){}
  
  void onResume(const G3MContext* context) {
    
  }
  
  void onPause(const G3MContext* context) {
    
  }
  
  void onDestroy(const G3MContext* context) {
    
  }
  
  bool onTouchEvent(const G3MEventContext* ec,
                    const TouchEvent* touchEvent);
  
  void selectStar(const Angle& trueNorthAzimuthInDegrees,
                  const Angle& altitudeInDegrees);
  
  
  
};

#endif /* defined(__G3MiOSDemo__StarDomeRenderer__) */
