package org.glob3.mobile.generated; 
//
//  IRenderer.h
//  G3MiOSSDK
//
//  Created by José Miguel S N on 31/05/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


//C++ TO JAVA CONVERTER NOTE: Java has no need of forward class declarations:
//class TouchEvent;
//C++ TO JAVA CONVERTER NOTE: Java has no need of forward class declarations:
//class InitializationContext;
//C++ TO JAVA CONVERTER NOTE: Java has no need of forward class declarations:
//class RenderContext;

public abstract class Renderer
{

  public static final int maxTimeToRender = 1000;

  public abstract void initialize(InitializationContext ic);

  public abstract boolean isReadyToRender(RenderContext rc);

  public abstract int render(RenderContext rc);

  public abstract boolean onTouchEvent(TouchEvent touchEvent);

  public abstract void onResizeViewportEvent(int width, int height);

  public void dispose()
  {
  }
}