package org.glob3.mobile.generated; 
//
//  Petition.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 20/08/12.
//
//

//
//  Petition.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 20/08/12.
//
//


//class IImage;
//class TimeInterval;

public class Petition
{
  private final Sector _sector ;
  private IImage _image;
  private final float _layerTransparency;

  final private URL _url;

  private final long _timeToCacheInMS;
  private final boolean _readExpired;

  private final boolean _isTransparent;

//C++ TO JAVA CONVERTER TODO TASK: The implementation of the following method could not be found:
//  Petition(Petition that);

//C++ TO JAVA CONVERTER TODO TASK: The implementation of the following method could not be found:
//  void operator =(Petition that);


  public Petition(Sector sector, URL url, TimeInterval timeToCache, boolean readExpired, boolean isTransparent, float layerTransparency)
  {
     _sector = new Sector(sector);
     _url = url;
     _timeToCacheInMS = timeToCache._milliseconds;
     _readExpired = readExpired;
     _isTransparent = isTransparent;
     _image = null;
     _layerTransparency = layerTransparency;
  }

  public void dispose()
  {
    releaseImage();
  }

  public final void releaseImage()
  {
    if (_image != null)
       _image.dispose();
    _image = null;
  }

  public final URL getURL()
  {
    return _url;
  }

  public final Sector getSector()
  {
    return _sector;
  }

  public final void setImage(IImage image)
  {
    releaseImage();
    _image = image;
  }

  public final IImage getImage()
  {
    return _image;
  }

  public final TimeInterval getTimeToCache()
  {
    return TimeInterval.fromMilliseconds(_timeToCacheInMS);
  }

  public final boolean getReadExpired()
  {
    return _readExpired;
  }

  public final boolean isTransparent()
  {
    return _isTransparent;
  }

  public final String description()
  {
    IStringBuilder isb = IStringBuilder.newStringBuilder();
    isb.addString("Petition(url=");
    isb.addString(_url.description());
    isb.addString(", sector=");
    isb.addString(_sector.description());
    isb.addString(", image=");
    if (_image == null)
    {
      isb.addString("NULL");
    }
    else
    {
      isb.addString(_image.description());
    }
  
    final String s = isb.getString();
    if (isb != null)
       isb.dispose();
    return s;
  }
  @Override
  public String toString() {
    return description();
  }

  public final float getLayerTransparency()
  {
    return _layerTransparency;
  }

}