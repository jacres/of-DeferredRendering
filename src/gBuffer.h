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
    GBUFFER_TEXTURE_TYPE_DIFFUSE,
    GBUFFER_TEXTURE_TYPE_POSITION,
    GBUFFER_TEXTURE_TYPE_NORMAL,
    GBUFFER_TEXTURE_TYPE_LINEAR_DEPTH,
    GBUFFER_NUM_TEXTURES
  };
  
  GBuffer();
  ~GBuffer();
  
  bool setup(unsigned int windowWidth, unsigned windowHeight);
  
  void bindForWriting(float near, float far);
  void unbindForWriting();
  void bindForReading();
  void unbindForReading();
  
  void drawDebug(int x, int y);
  
  GLuint getTexture(GBUFFER_TEXTURE_TYPE textureType);
  
private:
  static const ofVec2f  s_quadVerts[];
  static const ofVec2f  s_quadTexCoords[];
  static ofVbo          s_quadVbo;

  ofShader m_gBufferShader;
  
  int    m_fbo_w;
  int    m_fbo_h;
  
  GLuint m_fbo;
  GLuint m_textures[GBUFFER_NUM_TEXTURES];
  GLenum m_drawBuffers[GBUFFER_NUM_TEXTURES];

  void loadShaders();
  void setupFbo();
};

#endif /* defined(__app__gbuffer__) */
