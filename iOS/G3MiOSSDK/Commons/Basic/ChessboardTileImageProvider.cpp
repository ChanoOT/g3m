//
//  ChessboardTileImageProvider.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 4/23/14.
//
//

#include "ChessboardTileImageProvider.hpp"

#include "IImage.hpp"
#include "TileImageListener.hpp"
#include "Tile.hpp"
#include "RectangleF.hpp"
#include "IFactory.hpp"
#include "ICanvas.hpp"
#include "Color.hpp"
#include "IImageListener.hpp"

class ChessboardTileImageProvider_IImageListener : public IImageListener {
private:
  ChessboardTileImageProvider* _parent;
  const Tile*                  _tile;
  TileImageListener*           _listener;
  const bool                   _deleteListener;

public:

  ChessboardTileImageProvider_IImageListener(ChessboardTileImageProvider* parent,
                                             const Tile* tile,
                                             TileImageListener* listener,
                                             bool deleteListener) :
  _parent(parent),
  _tile(tile),
  _listener(listener),
  _deleteListener(deleteListener)
  {
  }

  void imageCreated(const IImage* image) {
    _parent->imageCreated(image,
                          _tile,
                          _listener,
                          _deleteListener);
  }
};

ChessboardTileImageProvider::~ChessboardTileImageProvider() {
  delete _image;
}

TileImageContribution ChessboardTileImageProvider::contribution(const Tile* tile) {
  //return FULL_COVERAGE_TRANSPARENT;
  return TileImageContribution::fullCoverageTransparent(1);
}

void ChessboardTileImageProvider::imageCreated(const IImage* image,
                                               const Tile* tile,
                                               TileImageListener* listener,
                                               bool deleteListener) {
  _image = image->shallowCopy();

  listener->imageCreated(tile,
                         image,
                         "ChessboardTileImageProvider_image",
                         tile->_sector,
                         RectangleF(0, 0, image->getWidth(), image->getHeight()),
                         1);

  if (deleteListener) {
    delete listener;
  }
}

void ChessboardTileImageProvider::create(const Tile* tile,
                                         const Vector2I& resolution,
                                         long long tileDownloadPriority,
                                         TileImageListener* listener,
                                         bool deleteListener) {
  if (_image == NULL) {
    const int width = resolution._x;
    const int height = resolution._y;

    ICanvas* canvas = IFactory::instance()->createCanvas();
    canvas->initialize(width, height);

    canvas->setFillColor(Color::white());
    canvas->fillRectangle(0, 0, width, height);


//    canvas->setFillColor(Color::gray());

    canvas->setFillColor(Color::fromRGBA(0.9f, 0.9f, 0.35f, 1));

    const float xInterval = (float) width  / _splits;
    const float yInterval = (float) height / _splits;

//    for (int col = 0; col <= _splits; col += 2) {
//      const float x  = col * xInterval;
//      const float x2 = (col + 1) * xInterval;
//      for (int row = 0; row <= _splits; row += 2) {
//        const float y  = row * yInterval;
//        const float y2 = (row + 1) * yInterval;
//
//        canvas->fillRectangle(x - (xInterval / 2),
//                              y - (yInterval / 2),
//                              xInterval,
//                              yInterval);
//        canvas->fillRectangle(x2 - (xInterval / 2),
//                              y2 - (yInterval / 2),
//                              xInterval,
//                              yInterval);
//      }
//    }

    for (int col = 0; col < _splits; col += 2) {
      const float x  = col * xInterval;
      const float x2 = (col + 1) * xInterval;
      for (int row = 0; row < _splits; row += 2) {
        const float y  = row * yInterval;
        const float y2 = (row + 1) * yInterval;

        canvas->fillRoundedRectangle(x + 1,
                                     y + 1,
                                     xInterval - 2,
                                     yInterval - 2,
                                     4);
        canvas->fillRoundedRectangle(x2 + 1,
                                     y2 + 1,
                                     xInterval - 2,
                                     yInterval - 2,
                                     4);
      }
    }

//    canvas->setLineColor(Color::magenta());
//    canvas->strokeRectangle(0, 0, width, height);

    canvas->createImage(new ChessboardTileImageProvider_IImageListener(this,
                                                                       tile,
                                                                       listener,
                                                                       deleteListener),
                        true);

    delete canvas;
  }
  else {
    IImage* image = _image->shallowCopy();
    listener->imageCreated(tile,
                           image,
                           "ChessboardTileImageProvider_image",
                           tile->_sector,
                           RectangleF(0, 0, image->getWidth(), image->getHeight()),
                           1);
    if (deleteListener) {
      delete listener;
    }
  }
}

void ChessboardTileImageProvider::cancel(const Tile* tile) {
  // do nothing, can't cancel
}