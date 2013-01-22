//  gBuffer.cpp
//
//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#include "gBuffer.h"

GBuffer::GBuffer() :
m_fbo(0),
m_bIsSetup(false)
{
  // set all textures to zero
  memset(m_textures, 0, sizeof(m_textures));
}

GBuffer::~GBuffer() {

}

bool GBuffer::setup(unsigned int windowWidth, unsigned int windowHeight) {

  if (!m_bIsSetup) {
    loadShaders();
    m_bIsSetup = true;
  }

  bool success = setupFbo(windowWidth, windowHeight);

  return success;
}

void GBuffer::loadShaders() {
  m_gBufferShader.load("shaders/gbuffer.vert", "shaders/gbuffer.frag");
}

bool GBuffer::setupFbo(unsigned int windowWidth, unsigned int windowHeight) {

  m_buffer_w = windowWidth;
  m_buffer_h = windowHeight;

  // delete existing fbo, textures, and render buffer in case we are regenerating at new size
  glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
  glDeleteRenderbuffers(1, &m_renderBuffer);
  glDeleteFramebuffers(1, &m_fbo);

  // create an fbo
  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

  // create all gbuffer textures
  glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);

  // albedo/diffuse (16-bit channel rgba)
  glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_ALBEDO]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_buffer_w, m_buffer_h, 0, GL_RGBA, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_ALBEDO, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_ALBEDO], 0);

  // normals + depth (32-bit RGBA float for accuracy)
  glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_NORMALS_DEPTH]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_buffer_w, m_buffer_h, 0, GL_RGBA, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_NORMALS_DEPTH, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_NORMALS_DEPTH], 0);

  // lighting pass (16-bit RGBA)
  glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_LIGHT_PASS]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_buffer_w, m_buffer_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_LIGHT_PASS, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_LIGHT_PASS], 0);

  // create depth texture (we don't use this explicitly, but since we use depth testing when rendering + for our stencil pass, our FBO needs a depth buffer)
  // we make it a renderbuffer and not a texture as we'll never access it directly in a shader
  glGenRenderbuffers(1, &m_renderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, m_buffer_w, m_buffer_h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);

  // check status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    ofLogError("GBuffer::setupFbo()", "Could not create framebuffer");
    return false;
  }

  return true;
}

GLuint GBuffer::getTexture(GBUFFER_TEXTURE_TYPE textureType) {
  return m_textures[textureType];
}

void GBuffer::bindForGeomPass(float near, float far) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_buffer_w, m_buffer_h);

  GLuint drawBuffers[] = {GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_ALBEDO, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_NORMALS_DEPTH };
  glDrawBuffers(2, drawBuffers);

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glDisable(GL_BLEND);

  m_gBufferShader.begin();
  m_gBufferShader.setUniform1i("u_texture", 0);
  m_gBufferShader.setUniform1f("u_farDistance", far);
}

void GBuffer::unbindForGeomPass() {

  m_gBufferShader.end();

  glDepthMask(GL_FALSE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
}


void GBuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
}

void GBuffer::bindForReading() {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
}

void GBuffer::unbindForReading() {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GBuffer::bindForStencilPass() {
  glDrawBuffer(GL_NONE); // disable draw buffer as we don't want to be drawing to the depth buffer or anything else
  // we only want to affect the stencil buffer

  glEnable(GL_STENCIL_TEST);
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_CULL_FACE);

  glStencilFunc(GL_ALWAYS, 0, 0); // always pass stencil

  // first param (GL_KEEP) is when stencil test fails - it's set to always pass, so is irrelevant
  // second param is when stencil passes, but depth test fails - we want to increment/decrement
  // third param is when stencil passes and depth test passes (means vertex is completely outside of volume (behind)
  glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
  glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
}

void GBuffer::bindForLightPass() {
  glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_LIGHT_PASS);

  glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
  glDisable(GL_DEPTH_TEST);

  // enable additive blending since we're going to add all the light colours together as they are individually drawn
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
}

void GBuffer::resetLightPass() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
  glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_LIGHT_PASS);

  glViewport(0, 0, m_buffer_w, m_buffer_h);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GBuffer::drawDebug(int x, int y) {
  // draw buffers at 1/4 size
  int quarterW = m_buffer_w/4;
  int quarterH = m_buffer_h/4;

  int wy = ofGetHeight() - y - quarterH;

  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, m_buffer_w, m_buffer_h, 0+x, 0+wy, quarterW+x, quarterH+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0, 0, m_buffer_w, m_buffer_h, quarterW+x, 0+wy, 2*quarterW+x, quarterH+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT2);
  glBlitFramebuffer(0, 0, m_buffer_w, m_buffer_h, 2*quarterW+x, 0+wy, 3*quarterW+x, quarterH+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
