//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#ifndef __deferred__light__
#define __deferred__light__

#include <iostream>
#include "ofMain.h"

class Light : public ofNode {
public:
  Light() {
    memset(ambient, 0, sizeof(ambient));
    memset(diffuse, 0, sizeof(diffuse));
    memset(specular, 0, sizeof(specular));
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

  float ambient[4];
  float diffuse[4];
  float specular[4];
  
  ofVec3f orbitAxis;
};

#endif /* defined(__deferred__light__) */
