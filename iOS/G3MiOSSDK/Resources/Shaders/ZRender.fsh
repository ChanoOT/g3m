//
//  Default.vsh
//
//  Created by José Miguel Santana Núñez
//

void main() {

  highp float winZ =  gl_FragCoord.z; //FragCoord are defined in Window Space
  
  // convert float z value to 24bits integer (32bits causes precision error)
  highp float zFar = 16777215.0; // 2^24-1
  highp float z = winZ * zFar;
  highp float Z = floor(z+0.5);
  highp float R = floor(Z/65536.0);
  Z -= R * 65536.0;
  highp float G = floor(Z/256.0);
  highp float B = Z - G * 256.0;
  
  R /= 255.0;
  G /= 255.0;
  B /= 255.0;
  
  // writes zvalue
  gl_FragColor = vec4(R, G, B, 0.0);
}