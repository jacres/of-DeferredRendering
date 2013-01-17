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
  m_fbo_h = h;
  
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
  // create all gbuffer textures
  glGenTextures(1, &m_ssaoTex);
  
  // 8-bit RGB texture for SSAO
  glBindTexture(GL_TEXTURE_2D, m_ssaoTex);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_fbo_w, m_fbo_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  
  // create an fbo
  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoTex, 0);

  // check status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    cout << "ssaoPass::setup() - error could not create framebuffer" << endl;
    return false;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint SSAOPass::getTextureReference() {
  return m_ssaoTex;
}

void SSAOPass::applySSAO(GLuint positionTexUnit, GLuint normalTexUnit, GLuint depthTexUnit) {
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  
  glEnable(GL_BLEND);
  
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
  GLuint drawBuffers[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, drawBuffers);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glDisable(GL_BLEND);
  
  glActiveTexture(GL_TEXTURE10);
  glBindTexture(GL_TEXTURE_2D, m_randomTexture.getTextureReference().getTextureData().textureID);
  
  m_ssaoShader.begin();
  
  m_ssaoShader.setUniform1i("u_randomJitterTex", 10);
  m_ssaoShader.setUniform1i("u_viewSpacePositionTex", positionTexUnit);
  m_ssaoShader.setUniform1i("u_normalTex", normalTexUnit);
  m_ssaoShader.setUniform1i("u_linearDepthTex", depthTexUnit);
  
  m_ssaoShader.setUniform1f("u_texelWidth", m_texel_w);
  m_ssaoShader.setUniform1f("u_texelHeight", m_texel_h);
  
  m_ssaoShader.setUniform1f("u_occluderBias", 0.05f);
  m_ssaoShader.setUniform1f("u_samplingRadius", 20.0f);
  m_ssaoShader.setUniform2f("u_attenuation", 1.0f, 0.005f);
  
  drawScreenQuad();

  m_ssaoShader.end();
  
  glActiveTexture(GL_TEXTURE10); glBindTexture(GL_TEXTURE_2D, 0); // unbind jitter texture
  glActiveTexture(GL_TEXTURE0); // restore active texture to texture0 (OF expects this)
 
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
  
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
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

void SSAOPass::drawDebug(int x, int y) {
  // draw buffers at 1/4 size
  int quarterW = m_fbo_w/4;
  int quarterH = m_fbo_h/4;
  
  int wy = ofGetWindowHeight() - y - quarterH;
  
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
  
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, m_fbo_w, m_fbo_h, 0+x, 0+wy, quarterW+x, quarterH+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
