package org.glob3.mobile.generated; 
//
//  Layer.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 23/08/12.
//
//

//
//  Layer.hpp
//  G3MiOSSDK
//
//  Created by José Miguel S N on 23/07/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//



//class LayerCondition;
//class LayerTouchEventListener;
//class LayerSet;
//class LayerTilesRenderParameters;
//class G3MRenderContext;
//class G3MEventContext;
//class Tile;
//class URL;
//class RenderState;
//class Geodetic2D;
//class G3MContext;
//class Sector;
//class LayerTouchEvent;
//class Petition;
//class TileImageProvider;


public abstract class Layer
{
  protected java.util.ArrayList<LayerTouchEventListener> _listeners = new java.util.ArrayList<LayerTouchEventListener>();
  protected java.util.ArrayList<String> _errors = new java.util.ArrayList<String>();

  protected LayerSet _layerSet;

  protected boolean _enable;

  protected String _disclaimerInfo;
  protected java.util.ArrayList<String> _infos = new java.util.ArrayList<String>();

  protected LayerTilesRenderParameters _parameters;

  protected final float _transparency;
  protected final LayerCondition _condition;

  protected final void notifyChanges()
  {
    if (_layerSet != null)
    {
      _layerSet.layerChanged(this);
      _layerSet.changedInfo(_infos);
    }
  }

  protected String _title;

  protected Layer(LayerTilesRenderParameters parameters, float transparency, LayerCondition condition, String disclaimerInfo)
  {
     _parameters = parameters;
     _transparency = transparency;
     _condition = condition;
     _disclaimerInfo = disclaimerInfo;
     _layerSet = null;
     _enable = true;
     _title = "";
  }

  protected final void setParameters(LayerTilesRenderParameters parameters)
  {
    if (parameters != _parameters)
    {
      _parameters = null;
      _parameters = parameters;
      notifyChanges();
    }
  }

  protected abstract String getLayerType();

  protected abstract boolean rawIsEquals(Layer that);

  protected final Tile getParentTileOfSuitableLevel(Tile tile)
  {
      final int maxLevel = _parameters._maxLevel;
      Tile result = tile;
      while ((result != null) && (result._level > maxLevel))
      {
          result = result.getParent();
      }
      return result;
  }


  public void setEnable(boolean enable)
  {
    if (enable != _enable)
    {
      _enable = enable;
      notifyChanges();
    }
  }

  public boolean isEnable()
  {
    return _enable;
  }

  public void dispose()
  {
    if (_condition != null)
       _condition.dispose();
    _parameters = null;
  }

  public boolean isAvailable(Tile tile)
  {
    if (!isEnable())
    {
      return false;
    }
    if (_condition == null)
    {
      return true;
    }
    return _condition.isAvailable(tile);
  }

  public abstract URL getFeatureInfoURL(Geodetic2D position, Sector sector);

  public abstract RenderState getRenderState();

  public void initialize(G3MContext context)
  {
  }

  public final void addLayerTouchEventListener(LayerTouchEventListener listener)
  {
    _listeners.add(listener);
  }

  public final boolean onLayerTouchEventListener(G3MEventContext ec, LayerTouchEvent tte)
  {
    final int listenersSize = _listeners.size();
    for (int i = 0; i < listenersSize; i++)
    {
      LayerTouchEventListener listener = _listeners.get(i);
      if (listener != null)
      {
        if (listener.onTerrainTouch(ec, tte))
        {
          return true;
        }
      }
    }
    return false;
  }

  public final void setLayerSet(LayerSet layerSet)
  {
    if (_layerSet != null)
    {
      ILogger.instance().logError("LayerSet already set.");
    }
    _layerSet = layerSet;
  }

  public final void removeLayerSet(LayerSet layerSet)
  {
    if (_layerSet != layerSet)
    {
      ILogger.instance().logError("_layerSet doesn't match.");
    }
    _layerSet = null;
  }

  public final LayerTilesRenderParameters getLayerTilesRenderParameters()
  {
    return _parameters;
  }

  public abstract String description();
  @Override
  public String toString() {
    return description();
  }

  public boolean isEquals(Layer that)
  {
    if (this == that)
    {
      return true;
    }
  
    if (that == null)
    {
      return false;
    }
  
    if (!getLayerType().equals(that.getLayerType()))
    {
      return false;
    }
  
    if (_condition != that._condition)
    {
      return false;
    }
  
    final int thisListenersSize = _listeners.size();
    final int thatListenersSize = that._listeners.size();
    if (thisListenersSize != thatListenersSize)
    {
      return false;
    }
  
    for (int i = 0; i < thisListenersSize; i++)
    {
      if (_listeners.get(i) != that._listeners.get(i))
      {
        return false;
      }
    }
  
    if (_enable != that._enable)
    {
      return false;
    }
  
    if (!_parameters.isEquals(that._parameters))
    {
      return false;
    }
  
  
    if (!(_infos == that._infos))
    {
      return false;
    }
  
    if (!(_disclaimerInfo.equals(that._disclaimerInfo)))
    {
      return false;
    }
  
    return rawIsEquals(that);
  }

  public abstract Layer copy();

  public abstract Sector getDataSector();

  public final String getTitle()
  {
    return _title;
  }

  public final void setTitle(String title)
  {
    _title = title;
  }

  public abstract java.util.ArrayList<Petition> createTileMapPetitions(G3MRenderContext rc, LayerTilesRenderParameters layerTilesRenderParameters, Tile tile);

  public abstract TileImageProvider createTileImageProvider(G3MRenderContext rc, LayerTilesRenderParameters layerTilesRenderParameters);

  public final String getInfo()
  {
    return _disclaimerInfo;
  }

  public final void setInfo(String disclaimerInfo)
  {
    if (!_disclaimerInfo.equals(disclaimerInfo))
    {
      _disclaimerInfo = disclaimerInfo;
      if (_layerSet != null)
      {
        _layerSet.changedInfo(getInfos());
      }
    }
  }

  public final java.util.ArrayList<String> getInfos()
  {
//C++ TO JAVA CONVERTER TODO TASK: There is no preprocessor in Java:
//#warning TODO BETTER
    _infos.clear();
    final String layerInfo = getInfo();
    _infos.add(layerInfo);
    return _infos;
  }

}