//
//  gbuffer.h
//  app
//
//  Created by james on 12-12-07.
//  Copyright (c) 2012 james. All rights reserved.
//

#ifndef __app__gbuffer__
#define __app__gbuffer__

#include "ofMain.h"
#include <iostream>

using namespace std;

class GBuffer {
  
public:
  enum GBUFFER_TEXTURE_TYPE {
    GBUFFER_TEXTURE_TYPE_ALBEDO,
    GBUFFER_TEXTURE_TYPE_NORMALS_DEPTH,
    GBUFFER_TEXTURE_TYPE_LIGHT_PASS,
    GBUFFER_NUM_TEXTURES
  };
  
  GBuffer();
  ~GBuffer();
  
  bool setup(unsigned int windowWidth, unsigned windowHeight);
  bool setupFbo(unsigned int windowWidth, unsigned int windowHeight);
  
  void bindForGeomPass(float near, float far);
  void unbindForGeomPass();
  
  void bindForReading();
  void unbindForReading();
  
  void bindForLightPass();
  void bindForStencilPass();
  
  void resetLightPass();
  
  void unbind();
  
  void drawDebug(int x, int y);
  
  GLuint getTexture(GBUFFER_TEXTURE_TYPE textureType);
  
private:
  bool  m_bIsSetup;
  ofShader m_gBufferShader;
  
  int    m_buffer_w;
  int    m_buffer_h;

  GLuint m_fbo;
  GLuint m_textures[GBUFFER_NUM_TEXTURES];
  GLuint m_renderBuffer;

  void loadShaders();
};

#endif /* defined(__app__gbuffer__) */
