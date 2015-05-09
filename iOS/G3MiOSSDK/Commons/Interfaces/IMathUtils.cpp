//
//  IMathUtils.cpp
//  G3MiOSSDK
//
//  Created by José Miguel S N on 29/08/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "IMathUtils.hpp"
#include "Vector2D.hpp"

IMathUtils* IMathUtils::_instance = NULL;

Vector2D IMathUtils::solveSecondDegreeEquation(double A, double B, double C) const {
  
  double x = B*B - 4*A*C;
  if (x < 0){
    return Vector2D::nan();
  }
  
  double squareRoot = this->sqrt(x);
  double A2 = 2*A;
  
  return Vector2D((-B + squareRoot) / A2,
                  (-B - squareRoot) / A2);
}

void IMathUtils::solveSecondDegreeEquation(double A, double B, double C,
                                           double& result_x, double& result_y) const {
  
  double x = B*B - 4*A*C;
  if (x < 0){
    result_x = NAND;
    result_y = NAND;
    return;
  }
  
  double squareRoot = this->sqrt(x);
  double A2 = 2*A;
  result_x = (-B + squareRoot) / A2;
  result_y = (-B - squareRoot) / A2;
  
  /*return Vector2D((-B + squareRoot) / A2,
                  (-B - squareRoot) / A2);*/
}

