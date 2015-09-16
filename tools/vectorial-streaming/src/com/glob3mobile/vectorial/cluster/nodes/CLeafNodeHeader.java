

package com.glob3mobile.vectorial.cluster.nodes;

import com.glob3mobile.geo.Geodetic2D;
import com.glob3mobile.geo.Sector;


public class CLeafNodeHeader
   extends
      CNodeHeader {


   private final int _featuresCount;


   public CLeafNodeHeader(final Sector nodeSector,
                          final Sector minimumSector,
                          final Geodetic2D averagePosition,
                          final int featuresCount) {
      super(nodeSector, minimumSector, averagePosition);
      _featuresCount = featuresCount;
   }


   public int getFeaturesCount() {
      return _featuresCount;
   }


}
