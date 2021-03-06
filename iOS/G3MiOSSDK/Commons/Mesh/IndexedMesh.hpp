//
//  IndexedMesh.hpp
//  G3MiOSSDK
//
//  Created by José Miguel S N on 22/06/12.
//

#ifndef G3MiOSSDK_IndexedMesh
#define G3MiOSSDK_IndexedMesh

#include "AbstractMesh.hpp"

class IShortBuffer;

class IndexedMesh : public AbstractMesh {
private:
  IShortBuffer* _indices;
  const bool    _ownsIndices;

protected:
  void renderMesh(const G3MRenderContext* rc,
                  GLState* glState) const;

public:
  IndexedMesh(const int primitive,
              const Vector3D& center,
              IFloatBuffer* vertices,
              bool ownsVertices,
              IShortBuffer* indices,
              bool ownsIndices,
              float lineWidth,
              float pointSize = 1,
              const Color* flatColor = NULL,
              IFloatBuffer* colors = NULL,
              bool depthTest = true,
              IFloatBuffer* normals = NULL,
              bool polygonOffsetFill = false,
              float polygonOffsetFactor = 0,
              float polygonOffsetUnits = 0);

  ~IndexedMesh();

  const IShortBuffer* getIndices() const {
    return _indices;
  }

};

#endif
