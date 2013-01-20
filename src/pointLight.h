//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#ifndef __deferred__pointlight__
#define __deferred__pointlight__

#include <iostream>
#include "ofMain.h"

class PointLight : public ofNode {
public:
  PointLight() :
  intensity(1.0f)
  {
    memset(ambient, 0, sizeof(ambient));
    memset(diffuse, 0, sizeof(diffuse));
    memset(specular, 0, sizeof(specular));
    memset(attenuation, 1.0f, sizeof(attenuation));
  }
  
  void setAmbient(float r, float g, float b, float a=1.0f) {
    ambient[0] = r;
    ambient[1] = g;
    ambient[2] = b;
    ambient[3] = a;
  }

  void setDiffuse(float r, float g, float b, float a=1.0f) {
    diffuse[0] = r;
    diffuse[1] = g;
    diffuse[2] = b;
    diffuse[3] = a;
  }

  void setSpecular(float r, float g, float b, float a=1.0f) {
    specular[0] = r;
    specular[1] = g;
    specular[2] = b;
    specular[3] = a;
  }
  
  void setAttenuation(float constant, float linear, float exponential) {
    attenuation[0] = constant;
    attenuation[1] = linear;
    attenuation[2] = exponential;
  }
  
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float attenuation[3];

  float intensity;
  
  ofVec3f orbitAxis;
};

#endif /* defined(__deferred__pointlight__) */
