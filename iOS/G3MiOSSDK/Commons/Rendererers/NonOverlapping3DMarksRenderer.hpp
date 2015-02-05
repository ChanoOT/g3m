//
//  NonOverlappingMarkers3DRenderer.cpp
//  G3MiOSSDK
//
//  Created by Stefanie Alfonso on 2/1/15.
//
//

#ifndef __G3MiOSSDK__NonOverlapping3DMarksRenderer__
#define __G3MiOSSDK__NonOverlapping3DMarksRenderer__

#include <stdio.h>
#include "DefaultRenderer.hpp"
#include "Geodetic3D.hpp"
#include "Vector2F.hpp"
#include "Vector3D.hpp"
#include "IImageBuilderListener.hpp"

class IImageBuilder;
class Geodetic3D;
class Vector2D;
class Camera;
class Planet;
class GLState;
class IImage;
class TextureIDReference;
class Geometry2DGLFeature;
class ViewportExtentGLFeature;
class TexturesHandler;

class MarkWidget;

class MarkWidgetTouchListener {
public:
    virtual ~MarkWidgetTouchListener() {
    }
    
    virtual bool touchedMark(MarkWidget* mark, float x, float y) = 0;
};


class MarkWidget{
    GLState* _glState;
    Geometry2DGLFeature* _geo2Dfeature;
    ViewportExtentGLFeature* _viewportExtent;
#ifdef C_CODE
    const IImage* _image;
#endif
#ifdef JAVA_CODE
    private IImage _image;
#endif
    std::string _imageName;
    IImageBuilder* _imageBuilder;
    TexturesHandler* _texHandler;
    
    float _halfWidth;
    float _halfHeight;
    
    float _x, _y; //Screen position
    
    MarkWidgetTouchListener* _touchListener;
    
    class WidgetImageListener: public IImageBuilderListener{
        MarkWidget* _widget;
    public:
        WidgetImageListener(MarkWidget* widget, MarkWidgetTouchListener *touchListener = NULL):_widget(widget){}
        
        ~WidgetImageListener() {}
        
        virtual void imageCreated(const IImage*      image,
                                  const std::string& imageName){
            _widget->prepareWidget(image, imageName);
        }
        
        virtual void onError(const std::string& error){
            ILogger::instance()->logError(error);
        }
        
    };
    
    void prepareWidget(const IImage*      image,
                       const std::string& imageName);
    
public:
    MarkWidget(IImageBuilder* imageBuilder);
    
    ~MarkWidget();
    
    void init(const G3MRenderContext *rc,
              int viewportWidth, int viewportHeight);
    
    void render(const G3MRenderContext* rc, GLState* glState);
    
    void setScreenPos(float x, float y);
    Vector2F getScreenPos() const{ return Vector2F(_x, _y);}
    void resetPosition();
    
    float getHalfWidth() const{ return _halfWidth;}
    float getHalfHeight() const{ return _halfHeight;}
    
    void onResizeViewportEvent(int width, int height);
    
    bool isReady() const{
        return _image != NULL;
    }
    
    void clampPositionInsideScreen(int viewportWidth, int viewportHeight, int margin);
    
    bool onTouchEvent(float x, float y);
    
    void setTouchListener(MarkWidgetTouchListener* tl){
        if (_touchListener != NULL && _touchListener != tl){
            delete _touchListener;
        }
        _touchListener = tl;
    }
    
};


class NonOverlapping3DMark{
    float _springLengthInPixels;
    
    mutable Vector3D* _cartesianPos;
    Geodetic3D _geoPosition;
    
    float _dX, _dY; //Velocity vector (pixels per second)
    float _fX, _fY; //Applied Force
    
#ifdef C_CODE
    MarkWidget _widget;
    MarkWidget _anchorWidget;
#endif
#ifdef JAVA_CODE
    private MarkWidget _widget;
    private MarkWidget _anchorWidget;
#endif
    
    const float _springK;
    const float _maxSpringLength;
    const float _minSpringLength;
    const float _electricCharge;
    const float _anchorElectricCharge;
    const float _maxWidgetSpeedInPixelsPerSecond;
    const float _resistanceFactor;
    const float _minWidgetSpeedInPixelsPerSecond;
    
public:
    
    NonOverlapping3DMark(IImageBuilder* imageBuilderWidget,
                       IImageBuilder* imageBuilderAnchor,
                       const Geodetic3D& position,
                       MarkWidgetTouchListener* touchListener = NULL,
                       float springLengthInPixels = 10.0f,
                       float springK = 1.0f,
                       float maxSpringLength = 100.0f,
                       float minSpringLength = 5.0f,
                       float electricCharge = 3000.0f,
                       float anchorElectricCharge = 1500.0f,
                       float maxWidgetSpeedInPixelsPerSecond = 1000.0f,
                       float minWidgetSpeedInPixelsPerSecond = 35.0f,
                       float resistanceFactor = 0.95f);
    
    ~NonOverlapping3DMark();
    
    Vector3D getCartesianPosition(const Planet* planet) const;
    
    void computeAnchorScreenPos(const Camera* cam, const Planet* planet);
    
    Vector2F getScreenPos() const{ return _widget.getScreenPos();}
    Vector2F getAnchorScreenPos() const{ return _anchorWidget.getScreenPos();}
    
    void render(const G3MRenderContext* rc, GLState* glState);
    
    void applyCoulombsLaw(NonOverlapping3DMark* that); //EM
    void applyCoulombsLawFromAnchor(NonOverlapping3DMark* that);
    
    void applyHookesLaw();   //Spring
    
    void applyForce(float x, float y){
        _fX += x;
        _fY += y;
    }
    
    void updatePositionWithCurrentForce(double elapsedMS, float viewportWidth, float viewportHeight);
    
    void onResizeViewportEvent(int width, int height);
    
    void resetWidgetPositionVelocityAndForce(){ _widget.resetPosition(); _dX = 0; _dY = 0; _fX = 0; _fY = 0;}
    
    bool onTouchEvent(float x, float y);
    
};

class NonOverlapping3DMarksRenderer: public DefaultRenderer{
    
    int _maxVisibleMarks;
    
    std::vector<NonOverlapping3DMark*> _visibleMarks;
    std::vector<NonOverlapping3DMark*> _marks;
    
    void computeMarksToBeRendered(const Camera* cam, const Planet* planet);
    
    long long _lastPositionsUpdatedTime;
    
    GLState * _connectorsGLState;
    void renderConnectorLines(const G3MRenderContext* rc);
    
    void computeForces(const Camera* cam, const Planet* planet);
    void renderMarks(const G3MRenderContext* rc, GLState* glState);
    void applyForces(long long now, const Camera* cam);
    
    
public:
    NonOverlapping3DMarksRenderer(int maxVisibleMarks = 30);
    
    ~NonOverlapping3DMarksRenderer();
    
    void addMark(NonOverlapping3DMark* mark);
    
    virtual RenderState getRenderState(const G3MRenderContext* rc) {
        return RenderState::ready();
    }
    
    virtual void render(const G3MRenderContext* rc,
                        GLState* glState);
    
    virtual bool onTouchEvent(const G3MEventContext* ec,
                              const TouchEvent* touchEvent);
    
    virtual void onResizeViewportEvent(const G3MEventContext* ec, int width, int height);
    
    virtual void start(const G3MRenderContext* rc) {
        
    }
    
    virtual void stop(const G3MRenderContext* rc) {
        
    }
    
    virtual SurfaceElevationProvider* getSurfaceElevationProvider() {
        return NULL;
    }
    
    virtual PlanetRenderer* getPlanetRenderer() {
        return NULL;
    }
    
    virtual bool isPlanetRenderer() {
        return false;
    }
    
};

#endif /* defined(__G3MiOSSDK__NonOverlapping3DMarksRenderer__) */
