package org.glob3.mobile.generated; 
//
//  DebugTileImageProvider.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 4/18/14.
//
//

//
//  DebugTileImageProvider.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 4/18/14.
//
//




public class DebugTileImageProvider extends CanvasTileImageProvider
{
  private static class ImageListener extends IImageListener
  {
    private final String _tileId;
    private final TileImageContribution _contribution;

    private TileImageListener _listener;
    private boolean _deleteListener;

    private static String getImageId(String tileId)
    {
      IStringBuilder isb = IStringBuilder.newStringBuilder();
      isb.addString("DebugTileImageProvider/");
      isb.addString(tileId);
      final String s = isb.getString();
      if (isb != null)
         isb.dispose();
      return s;
    }

    public ImageListener(String tileId, TileImageContribution contribution, TileImageListener listener, boolean deleteListener)
    {
       _tileId = tileId;
       _contribution = contribution;
       _listener = listener;
       _deleteListener = deleteListener;
      TileImageContribution.retainContribution(_contribution);
    }

    public void dispose()
    {
      TileImageContribution.releaseContribution(_contribution);
      super.dispose();
    }

    public final void imageCreated(IImage image)
    {
      final String imageId = getImageId(_tileId);
      _listener.imageCreated(_tileId, image, imageId, _contribution);
      if (_deleteListener)
      {
        if (_listener != null)
           _listener.dispose();
      }
    }

  }

  public void dispose()
  {
    super.dispose();
  }


  public final TileImageContribution contribution(Tile tile)
  {
    return TileImageContribution.fullCoverageTransparent(1);
  }

  public final void create(Tile tile, TileImageContribution contribution, Vector2I resolution, long tileDownloadPriority, boolean logDownloadActivity, TileImageListener listener, boolean deleteListener, FrameTasksExecutor frameTasksExecutor)
  {
    final int width = resolution._x;
    final int height = resolution._y;
  
    ICanvas canvas = getCanvas(width, height);
  
    canvas.setLineColor(Color.green());
    canvas.setLineWidth(1);
    canvas.strokeRectangle(0, 0, width, height);
  
    canvas.createImage(new DebugTileImageProvider.ImageListener(tile._id, contribution, listener, deleteListener), true);
  }

  public final void cancel(String tileId)
  {
    // do nothing, can't cancel
  }

}