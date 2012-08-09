package org.glob3.mobile.generated; 
//
//  CameraEventHandler.hpp
//  G3MiOSSDK
//
//  Created by Agustín Trujillo Pino on 07/08/12.
//  Copyright (c) 2012 Universidad de Las Palmas. All rights reserved.
//



//C++ TO JAVA CONVERTER NOTE: Java has no need of forward class declarations:
//class TouchEvent;
//C++ TO JAVA CONVERTER NOTE: Java has no need of forward class declarations:
//class RenderContext;


public abstract class CameraEventHandler
{

  public abstract boolean onTouchEvent(EventContext eventContext, TouchEvent touchEvent, CameraContext cameraContext);
  public abstract int render(RenderContext rc, CameraContext cameraContext);
  public void dispose()
  {
  }

  private abstract void onDown(EventContext eventContext, TouchEvent touchEvent, CameraContext cameraContext);
  private abstract void onMove(EventContext eventContext, TouchEvent touchEvent, CameraContext cameraContext);
  private abstract void onUp(EventContext eventContext, TouchEvent touchEvent, CameraContext cameraContext);

}