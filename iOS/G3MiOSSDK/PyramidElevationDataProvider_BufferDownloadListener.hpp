//
//  PyramidElevationDataProvider_BufferDownloadListener.hpp
//  G3MiOSSDK
//
//  Created by Sebastian Ortega Trujillo on 18/2/16.
//
//

#ifndef G3MiOSSDK_PyramidElevationDataProvider_BufferDownloadListener_hpp
#define G3MiOSSDK_PyramidElevationDataProvider_BufferDownloadListener_hpp

#include <stdio.h>
#include "IBufferDownloadListener.hpp"
#include "ElevationDataProvider.hpp"
#include "ShortBufferElevationData.hpp"
#include "Sector.hpp"
#include "BilParser.hpp"
#include "JSONObject.hpp"

class PyramidElevationDataProvider_BufferDownloadListener : public IBufferDownloadListener {
private:
    
    const Sector &_sector;
    int _width, _height;
    IElevationDataListener *_listener;
    bool _autodeleteListener;
    double _deltaHeight;
    bool _variableSized;
    
    const Vector2I* getResolution(const JSONObject *data);
    ShortBufferElevationData* getElevationData(Sector sector,
                                               Vector2I extent,
                                               const JSONObject *data,
                                               double deltaHeight);
    
public:
    PyramidElevationDataProvider_BufferDownloadListener(const Sector& sector,
                                           const Vector2I& extent,
                                           bool variableSized,
                                           IElevationDataListener *listener,
                                           bool autodeleteListener,
                                           double deltaHeight);
    
    void onDownload(const URL& url,
                    IByteBuffer* buffer,
                    bool expired);
    void onError(const URL& url);
    void onCancel(const URL& url);
    
    void onCanceledDownload(const URL& url,
                            IByteBuffer* data,
                            bool expired);
    
};

#endif