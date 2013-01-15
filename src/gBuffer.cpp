//  gBuffer.cpp
//
//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#include "gbuffer.h"

ofVbo GBuffer::s_quadVbo;

const ofVec2f GBuffer::s_quadVerts[] = {
  ofVec2f(-1.0f, -1.0f),
  ofVec2f(1.0f, -1.0f),
  ofVec2f(1.0f, 1.0f),
  ofVec2f(-1.0f, 1.0f)
};

const ofVec2f GBuffer::s_quadTexCoords[] = {
  ofVec2f(0.0f, 0.0f),
  ofVec2f(1.0f, 0.0f),
  ofVec2f(1.0f, 1.0f),
  ofVec2f(0.0f, 1.0f)
};


GBuffer::GBuffer() :
m_fbo(0)
{
  // set all textures and draw buffers to zero
  memset(m_textures, 0, sizeof(m_textures));
  memset(m_drawBuffers, 0, sizeof(m_drawBuffers));
}

GBuffer::~GBuffer() {
  
}

bool GBuffer::setup(unsigned int windowWidth, unsigned windowHeight) {
  m_fbo_w = windowWidth;
  m_fbo_h = windowHeight;
  
  // full viewport quad vbo
  s_quadVbo.setVertexData(&s_quadVerts[0], 4, GL_STATIC_DRAW);
  s_quadVbo.setTexCoordData(&s_quadTexCoords[0], 4, GL_STATIC_DRAW);

  loadShaders();
  setupFbo();
  
  // return success
  return true;
}

void GBuffer::loadShaders() {
  m_gBufferShader.load("shaders/gbuffer.vert", "shaders/gbuffer.frag");
}

void GBuffer::setupFbo() {
  // create an fbo
  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  
  // create all gbuffer textures
  glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);
  
  // albedo/diffuse (8-bit channel rgb)
  glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_DIFFUSE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_fbo_w, m_fbo_h, 0, GL_RGB, GL_UNSIGNED_INT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_DIFFUSE], 0);
  
  // position (32-bit RGB float for accuracy)
  glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_fbo_w, m_fbo_h, 0, GL_RGB, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION], 0);
  
  // normal (16-bit RGB float for accuracy)
  glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_NORMAL]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_fbo_w, m_fbo_h, 0, GL_RGB, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_NORMAL], 0);
  
  // linear depth (16-bit RGB float for accuracy)
  glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_LINEAR_DEPTH]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_fbo_w, m_fbo_h, 0, GL_RGB, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_LINEAR_DEPTH], 0);

  // create depth texture (we don't use this explicitly, but since we use depth testing when rendering, our FBO needs a depth buffer)
  // we make it a renderbuffer and not a texture as we'll never access it
  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, m_fbo_w, m_fbo_h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
  
  // specify draw buffers for render targets
  for (unsigned int i=0; i<GBUFFER_NUM_TEXTURES; i++) {
    m_drawBuffers[i] = GL_COLOR_ATTACHMENT0+i;
  };
  
  glDrawBuffers(GBUFFER_NUM_TEXTURES, m_drawBuffers);

  // check status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    cout << "GBuffer::init() - error could not create framebuffer" << endl;
    return false;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  
}

GLuint GBuffer::getTexture(GBUFFER_TEXTURE_TYPE textureType) {
  return m_textures[textureType];
}

void GBuffer::bindForWriting(float near, float far) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glDrawBuffers(GBUFFER_NUM_TEXTURES, m_drawBuffers);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  m_gBufferShader.begin();
  m_gBufferShader.setUniform1i("u_texture", 0);
  m_gBufferShader.setUniform1f("u_linearDepthConstant", 1.0f/(far-near));
}

void GBuffer::unbindForWriting() {
  m_gBufferShader.end();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK_LEFT);
}

void GBuffer::bindForReading() {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
}

void GBuffer::unbindForReading() {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GBuffer::drawDebug(int x, int y) {
  
  int wy = ofGetWindowHeight() - y - 256;
  
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, m_fbo_w, m_fbo_h, 0+x, 0+wy, 256+x, 256+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0, 0, m_fbo_w, m_fbo_h, 256+x, 0+wy, 512+x, 256+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT2);
  glBlitFramebuffer(0, 0, m_fbo_w, m_fbo_h, 512+x, 0+wy, 768+x, 256+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glReadBuffer(GL_COLOR_ATTACHMENT3);
  glBlitFramebuffer(0, 0, m_fbo_w, m_fbo_h, 768+x, 0+wy, 1024+x, 256+wy, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}