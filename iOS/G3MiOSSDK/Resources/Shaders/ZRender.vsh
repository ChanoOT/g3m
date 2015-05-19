//
//  Default.vsh
//
//  Created by José Miguel Santana Núñez
//

attribute vec4 aPosition; 

uniform mat4 uModelview;
uniform float uPointSize;

varying highp float R;
varying highp float G;
varying highp float B;

void main() {
  gl_Position = uModelview * aPosition;
  gl_PointSize = uPointSize;

  highp float NDCz = (gl_Position.z / gl_Position.w); //GL_POSITION IS CLIP COORDINATES (AFTER PROJECTION) [-1, 1]

  highp float winZ = 0.5 + (NDCz/2.0); //[0, 1]

  // convert float z value to 24bits integer (32bits causes precision error)
  highp float zFar = 16777215.0; // 2^24-1
  highp float z = winZ * zFar;
  highp float Z = floor(z+0.5);
  R = floor(Z/65536.0);
  Z -= R * 65536.0;
  G = floor(Z/256.0);
  B = Z - G * 256.0;

  R /= 255.0;
  G /= 255.0;
  B /= 255.0;
  
}
 

