

package com.glob3mobile.pointcloud.kdtree;

import java.util.LinkedList;


public class KDMultiLeafNode
   extends
      KDLeafNode {

   private final int[] _vertexIndexes;


   KDMultiLeafNode(final KDNode parent,
                   final int[] vertexIndexes) {
      super(parent);
      _vertexIndexes = vertexIndexes;
   }


   @Override
   void breadthFirstAcceptVisitor(final KDTreeVisitor visitor,
                                  final LinkedList<KDNode> queue) throws KDTreeVisitor.AbortVisiting {
      visitor.visitLeafNode(this);
   }


   @Override
   public int[] getVertexIndexes() {
      return _vertexIndexes;
   }

}
