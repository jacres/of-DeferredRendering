#ifndef __deferred__ssaopass__
#define __deferred__ssaopass__

#include "ofMain.h"
#include <vector>

class SSAOPass {

public:
  SSAOPass();
  ~SSAOPass();

  bool setup(int w, int h, int numSamples=12);
  bool setupFbo(int w, int h);
  void setParameters(float occluderBias, float samplingRadius, float constantAttenuation, float linearAttenuation);
  void setCameraProperties(const ofMatrix4x4& invProjMatrix, float farDistance);
  void applySSAO(GLuint normalsAndDepthTexUnit);
  void drawDebug(int x, int y);
  
  GLuint getTextureReference();

private:
  static const ofVec2f  s_quadVerts[];
  static const ofVec2f  s_quadTexCoords[];
  static ofVbo          s_quadVbo;

  bool  m_bIsSetup;
  
  int   m_fbo_w;
  int   m_fbo_h;
  float m_texel_w;
  float m_texel_h;
  int   m_numSamples;
  
  ofShader  m_ssaoShader;
  ofImage   m_randomTexture;
  GLuint    m_fbo;
  GLuint    m_ssaoTex;
  
  void drawScreenQuad();
};

#endif
