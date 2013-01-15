//  ssaoPass.cpp
//
//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#include "ssaoPass.h"

ofVbo SSAOPass::s_quadVbo;

const ofVec2f SSAOPass::s_quadVerts[] = {
  ofVec2f(-1.0f, -1.0f),
  ofVec2f(1.0f, -1.0f),
  ofVec2f(1.0f, 1.0f),
  ofVec2f(-1.0f, 1.0f)
};

const ofVec2f SSAOPass::s_quadTexCoords[] = {
  ofVec2f(0.0f, 0.0f),
  ofVec2f(1.0f, 0.0f),
  ofVec2f(1.0f, 1.0f),
  ofVec2f(0.0f, 1.0f)
};


SSAOPass::SSAOPass() :
  m_fbo_w(1024),
  m_fbo_h(1024),
  m_numSamples(12)
{
}

SSAOPass::~SSAOPass() {
}

void SSAOPass::setup(int w, int h, int numSamples) {
  m_fbo_w = w;
  m_fbo_w = h;
  
  m_texel_w = 1.0f/(float)m_fbo_w;
  m_texel_h = 1.0f/(float)m_fbo_h;
  
  m_numSamples = numSamples;
  
  // full viewport quad vbo
  s_quadVbo.setVertexData(&s_quadVerts[0], 4, GL_STATIC_DRAW);
  s_quadVbo.setTexCoordData(&s_quadTexCoords[0], 4, GL_STATIC_DRAW);
  
  m_ssaoShader.load("shaders/ssao.vert", "shaders/ssao.frag");

  // load random normals texture for SSAO
  m_randomTexture.loadImage("textures/random.png");

  // setup fbo
  ofFbo::Settings s;
  s.width = w;
  s.height = h;
  s.internalformat = GL_RGBA;
  s.numColorbuffers = 1;
  s.textureTarget = GL_TEXTURE_2D;
  s.useDepth = false;
  s.useStencil = false;
  s.depthStencilAsTexture = false;
  
  m_fbo.allocate(s);
}

ofTexture& SSAOPass::getTextureReference() {
  return m_fbo.getTextureReference();
}

void SSAOPass::applySSAO(ofTexture& positionTex, ofTexture& normalTex, ofTexture& depthTex) {
  applySSAO(positionTex.texData.textureID, normalTex.texData.textureID, depthTex.texData.textureID);
}

void SSAOPass::applySSAO(GLuint positionTex, GLuint normalTex, GLuint depthTex) {
  m_fbo.begin();
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glDisable(GL_BLEND);

  glActiveTexture(GL_TEXTURE0); m_randomTexture.getTextureReference().bind();
  glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, positionTex);
  glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, normalTex);
  glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, depthTex);
  
  m_ssaoShader.begin();
  
  m_ssaoShader.setUniform1i("u_randomJitterTex", 0);
  m_ssaoShader.setUniform1i("u_viewSpacePositionTex", 1);
  m_ssaoShader.setUniform1i("u_normalTex", 2);
  m_ssaoShader.setUniform1i("u_linearDepthTex", 3);
  
  m_ssaoShader.setUniform1f("u_texelWidth", m_texel_w);
  m_ssaoShader.setUniform1f("u_texelHeight", m_texel_h);
  
  m_ssaoShader.setUniform1f("u_occluderBias", 0.05f);
  m_ssaoShader.setUniform1f("u_samplingRadius", 20.0f);
  m_ssaoShader.setUniform2f("u_attenuation", 1.0f, 0.005f);
  
  drawScreenQuad();

  m_ssaoShader.end();
 
  glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
  
  m_fbo.end();
}

void SSAOPass::drawScreenQuad() {
  // set identity matrices and save current matrix state
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  // draw the full viewport quad
  s_quadVbo.draw(GL_QUADS, 0, 4);
  
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

void SSAOPass::drawDebug(int x, int y, int w, int h) {
  ofPushMatrix();
  ofScale(1.0, -1.0, 1.0);
  m_fbo.draw(x, -y-h, w, h);
  ofPopMatrix();
}
