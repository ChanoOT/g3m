//
//  TextureMapping.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 12/07/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//

#include "TextureMapping.hpp"

#include "Context.hpp"
#include "GL.hpp"
#include "GPUProgramManager.hpp"
#include "GPUProgram.hpp"
#include "TexturesHandler.hpp"
#include "GLState.hpp"


void SimpleTextureMapping::setTranslation(float translationU,
                                          float translationV) {
  _translationU = translationU;
  _translationV = translationV;
#warning    updateState();
}

void SimpleTextureMapping::setScale(float scaleU,
                                    float scaleV) {
  _scaleU = scaleU;
  _scaleV = scaleV;
#warning    updateState();
}

void SimpleTextureMapping::setRotation(float rotationAngleInRadians,
                                       float rotationCenterU,
                                       float rotationCenterV) {
  _rotationInRadians = rotationAngleInRadians;
  _rotationCenterU = rotationCenterU;
  _rotationCenterV = rotationCenterV;
#warning    updateState();
}

void SimpleTextureMapping::releaseGLTextureId() {

  if (_glTextureId != NULL) {
#ifdef C_CODE
    delete _glTextureId;
#endif
#ifdef JAVA_CODE
    _glTextureId.dispose();
#endif
    _glTextureId = NULL;
  } else{
    ILogger::instance()->logError("Releasing invalid simple texture mapping");
  }
}

SimpleTextureMapping::~SimpleTextureMapping() {
  if (_ownedTexCoords) {
    delete _texCoords;
  }

  releaseGLTextureId();

#ifdef JAVA_CODE
  super.dispose();
#endif
}

void SimpleTextureMapping::modifyGLState(GLState& state) const{
  if (_texCoords == NULL) {
    ILogger::instance()->logError("SimpleTextureMapping::bind() with _texCoords == NULL");
  }
  else {
    state.clearGLFeatureGroup(COLOR_GROUP);

    if ((_scaleU != 1) ||
        (_scaleV != 1) ||
        (_translationU != 0) ||
        (_translationV != 0) ||
        (_rotationInRadians != 0)) {
      state.addGLFeature(new TextureGLFeature(_glTextureId->getID(),
                                              _texCoords,
                                              2,
                                              0,
                                              false,
                                              0,
                                              _transparent,
                                              GLBlendFactor::srcAlpha(),
                                              GLBlendFactor::oneMinusSrcAlpha(),
                                              _translationU,
                                              _translationV,
                                              _scaleU,
                                              _scaleV,
                                              _rotationInRadians,
                                              _rotationCenterU,
                                              _rotationCenterV),
                         false);
    }
    else {
      state.addGLFeature(new TextureGLFeature(_glTextureId->getID(),
                                              _texCoords,
                                              2,
                                              0,
                                              false,
                                              0,
                                              _transparent,
                                              GLBlendFactor::srcAlpha(),
                                              GLBlendFactor::oneMinusSrcAlpha()),
                         false);
    }
  }
}
