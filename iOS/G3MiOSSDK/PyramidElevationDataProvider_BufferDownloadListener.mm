//
//  PyramidElevationDataProvider_BufferDownloadListener.m
//  G3MiOSSDK
//
//  Created by Sebastian Ortega Trujillo on 18/2/16.
//
//

#import <Foundation/Foundation.h>
#import "PyramidElevationDataProvider_BufferDownloadListener.hpp"
#include "JSONParser_iOS.hpp"
#include "JSONArray.hpp"

PyramidElevationDataProvider_BufferDownloadListener::PyramidElevationDataProvider_BufferDownloadListener(const Sector& sector,
                                                                               const Vector2I& extent,
                                                                               bool variableSized,
                                                                               IElevationDataListener *listener,
                                                                               bool autodeleteListener,
                                                                               double deltaHeight) : _sector(sector){
    //_sector = Sector(sector);
    _width = extent._x;
    _height = extent._y;
    _listener = listener;
    _autodeleteListener = autodeleteListener;
    _deltaHeight = deltaHeight;
    _variableSized = variableSized;
}

void PyramidElevationDataProvider_BufferDownloadListener::onDownload(const URL& url,IByteBuffer* buffer,bool expired){
    
    //Vector2I *resolution = NULL;
    ShortBufferElevationData *elevationData;
    /*if (!_variableSized) {
        resolution = new Vector2I(8, 8);
        elevationData = BilParser::parseBil16MaxMin(_sector, *resolution, buffer, _deltaHeight);
    }
    else {
        elevationData = BilParser::parseBil16Redim(_sector, buffer, _deltaHeight);
        resolution = new Vector2I(elevationData->getExtent());
    }*/
    std::string contents = buffer->getAsString();
    const JSONObject *jsonContent = JSONParser_iOS::instance()->parse(contents)->asObject();
    const Vector2I *resolution = getResolution(jsonContent);
    elevationData = getElevationData(_sector, *resolution, jsonContent, _deltaHeight);
    
    if (buffer != NULL) delete buffer;
    
    if (elevationData == NULL)
    {
        _listener->onError(_sector, *resolution);
    }
    else
    {
        _listener->onData(_sector, *resolution, elevationData);
        //elevationData->_release();
    }
    
    
    if (_autodeleteListener)
    {
        if (_listener != NULL) delete _listener;
        _listener = NULL;
    }
}
void PyramidElevationDataProvider_BufferDownloadListener::onError(const URL& url){
    const Vector2I resolution = Vector2I(_width, _height);
    
    _listener->onError(_sector, resolution);
    if (_autodeleteListener)
    {
        if (_listener != NULL) delete _listener;
        _listener = NULL;
    }
}
void PyramidElevationDataProvider_BufferDownloadListener::onCancel(const URL& url){
    if (_listener != NULL)
    {
        const Vector2I resolution = Vector2I(_width, _height);
        _listener->onCancel(_sector, resolution);
        if (_autodeleteListener)
        {
            if (_listener != NULL) delete _listener;
            _listener = NULL;
        }
    }
}

void PyramidElevationDataProvider_BufferDownloadListener::onCanceledDownload(const URL& url,
                                                                IByteBuffer* data,
                                                                bool expired){
    if (_autodeleteListener)
    {
        if (_listener != NULL) delete _listener;
        _listener = NULL;
    }
}

const Vector2I* PyramidElevationDataProvider_BufferDownloadListener::getResolution(const JSONObject *data){
    return new Vector2I((int) data->getAsNumber("width",0),(int) data->getAsNumber("height",0));
}

ShortBufferElevationData* PyramidElevationDataProvider_BufferDownloadListener::getElevationData(Sector sector,
                                                                                                Vector2I extent,
                                                                                                const JSONObject *data,
                                                                                                double deltaHeight){
    const short minValue = IMathUtils::instance()->minInt16();
    const int size = extent._x * extent._y;
    const JSONArray *dataArray = data->getAsArray("data");
    short *shortBuffer = new short[size];
    for (int i = 0; i < size; i++)
    {
        short height = (short) dataArray->getAsNumber(i, minValue);
        
        if (height == 15000) //Our own NODATA, since -9999 is a valid height.
        {
            height = ShortBufferElevationData::NO_DATA_VALUE;
        }
        else if (height == minValue)
        {
            height = ShortBufferElevationData::NO_DATA_VALUE;
        }
        
        shortBuffer[i] = height;
    }
    
    short max = (short) data->getAsNumber("max",IMathUtils::instance()->minInt16());
    short min = (short) data->getAsNumber("min",IMathUtils::instance()->maxInt16());
    short children = (short) data->getAsNumber("withChildren",0);
    short similarity = (short) data->getAsNumber("similarity",0);
    
    return new ShortBufferElevationData(sector, extent, sector, extent, shortBuffer,
                                        size, deltaHeight,max,min,children,similarity);
}