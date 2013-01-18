//  testApp.cpp
//
//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#include "testApp.h"

testApp::testApp() :
m_angle(0),
m_bDrawDebug(true),
m_bPulseLights(false)
{};

//--------------------------------------------------------------
void testApp::setup() {
  ofSetVerticalSync(false); // can cause problems on some Linux implementations
  ofDisableArbTex();
  
  // set up our gbuffer and ssao pass
  m_gBuffer.setup(ofGetWindowWidth(), ofGetWindowHeight());
  m_ssaoPass.setup(ofGetWindowWidth(), ofGetWindowHeight(), 12);

  setupScreenQuad();

  m_cam.setupPerspective( false, 45.0f, 0.1f, 100.0f );
  m_cam.setDistance(40.0f);
  m_cam.setGlobalPosition( 0.0f, 0.0f, 35.0f );
  m_cam.lookAt( ofVec3f( 0.0f, 0.0f, 0.0f ) );
  
  m_shader.load("shaders/mainScene.vert", "shaders/mainScene.frag");
  m_pointLightPassShader.load("shaders/pointLightPass.vert", "shaders/pointLightPass.frag");
  
  m_texture.loadImage("textures/concrete.jpg");
  
  setupLights();
  createRandomBoxes();
  
  // setup fbo
  glGenTextures(1, &m_renderTex);
  
  glBindTexture(GL_TEXTURE_2D, m_renderTex);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ofGetWindowWidth(), ofGetWindowHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  
  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, ofGetWindowWidth(), ofGetWindowWidth());
  
  // create an fbo
  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTex, 0);  
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

  // check status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    cout << "ssaoPass::setup() - error could not create framebuffer" << endl;
    return false;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  bindGBufferTextures(); // bind them once to upper texture units - faster than binding/unbinding every frame
}

void testApp::setupScreenQuad() {
  ofVec2f quadVerts[] = {
    ofVec2f(-1.0f, -1.0f),
    ofVec2f(1.0f, -1.0f),
    ofVec2f(1.0f, 1.0f),
    ofVec2f(-1.0f, 1.0f)
  };
  
  ofVec2f quadTexCoords[] = {
    ofVec2f(0.0f, 0.0f),
    ofVec2f(1.0f, 0.0f),
    ofVec2f(1.0f, 1.0f),
    ofVec2f(0.0f, 1.0f)
  };
  
  // full viewport quad vbo
  m_quadVbo.setVertexData(&quadVerts[0], 4, GL_STATIC_DRAW);
  m_quadVbo.setTexCoordData(&quadTexCoords[0], 4, GL_STATIC_DRAW);
}

void testApp::createRandomBoxes() {
  // create 75 randomly rotated
  for (unsigned int i=0; i<75; i++) {
    float x = 0;
    float y = 0;
    float z = 0;
    float size = skRadius;
    float angle = ofRandom(0.0f, 90.0f);
    ofVec3f axis = ofVec3f(ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f));
    axis.normalize();

    m_boxes.push_back(Box(ofVec3f(x, y, z), angle, axis.x, axis.y, axis.z, size));
  }
}

void testApp::setupLights() {
  for (unsigned int i=0; i<skNumLights; i++) {
    addRandomLight();
  }
}

void testApp::addRandomLight() {
  // create a random light that is positioned on bounding sphere of scene (skRadius)
  PointLight l;
  ofVec3f posOnSphere = ofVec3f(ofRandom(-1.0f, 1.0f), ofRandom(-1.0f, 1.0f), ofRandom(-1.0f, 1.0f));
  posOnSphere.normalize();
  
  ofVec3f orbitAxis = ofVec3f(ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f));
  orbitAxis.normalize();
  l.orbitAxis = orbitAxis;
  
  posOnSphere.scale(skRadius-1);
  
  l.setPosition(posOnSphere);
  l.setAmbient(0.0f, 0.0f, 0.0f);
  
  ofVec3f col = ofVec3f(ofRandom(0.1f, 0.5f), ofRandom(0.2f, 0.4f), ofRandom(0.4f, 1.0f));
  l.setDiffuse(col.x, col.y, col.z);
  l.setSpecular(col.x, col.y, col.z);
  l.setAttenuation(0.0f, 0.0f, 0.08f); // set constant, linear, and exponential attenuation
  l.intensity = 0.7f;
  
  m_lights.push_back(l);
}

void testApp::randomizeLightColors() {
  for (vector<PointLight>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
    ofVec3f col = ofVec3f(ofRandom(0.4f, 1.0f), ofRandom(0.1f, 1.0f), ofRandom(0.3f, 1.0f));
    it->setDiffuse(col.x, col.y, col.z);
  }
}

void testApp::bindGBufferTextures() {
  // set up the texture units we want to use - we're using them every frame, so we'll leave them bound to these units to save speed vs. binding/unbinding
  m_textureUnits[TEX_UNIT_ALBEDO] = 11;
  m_textureUnits[TEX_UNIT_POSITION] = 12;
  m_textureUnits[TEX_UNIT_NORMAL] = 13;
  m_textureUnits[TEX_UNIT_DEPTH] = 14;
  m_textureUnits[TEX_UNIT_SSAO] = 15;
  
  // bind all GBuffer textures
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_ALBEDO]);
  glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE));
  
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_POSITION]);
  glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION));
  
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_NORMAL]);
  glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL));
  
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_DEPTH]);
  glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(GBuffer::GBUFFER_TEXTURE_TYPE_LINEAR_DEPTH));

  // bind SSAO texture
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_SSAO]);
  glBindTexture(GL_TEXTURE_2D, m_ssaoPass.getTextureReference());
  
  m_shader.begin();  // our final deferred scene shader
  m_shader.setUniform1i("u_albedoTex", m_textureUnits[TEX_UNIT_ALBEDO]);
  m_shader.setUniform1i("u_positionTex", m_textureUnits[TEX_UNIT_POSITION]);
  m_shader.setUniform1i("u_normalTex", m_textureUnits[TEX_UNIT_NORMAL]);
  m_shader.setUniform1i("u_linearDepthTex", m_textureUnits[TEX_UNIT_DEPTH]);
  m_shader.setUniform1i("u_ssaoTex", m_textureUnits[TEX_UNIT_SSAO]);
  m_shader.end();
  
  m_pointLightPassShader.begin();  // our point light pass shader
  m_pointLightPassShader.setUniform1i("u_albedoTex", m_textureUnits[TEX_UNIT_ALBEDO]);
  m_pointLightPassShader.setUniform1i("u_positionTex", m_textureUnits[TEX_UNIT_POSITION]);
  m_pointLightPassShader.setUniform1i("u_normalTex", m_textureUnits[TEX_UNIT_NORMAL]);
  m_pointLightPassShader.setUniform1i("u_linearDepthTex", m_textureUnits[TEX_UNIT_DEPTH]);
  m_pointLightPassShader.setUniform1i("u_ssaoTex", m_textureUnits[TEX_UNIT_SSAO]);
  m_pointLightPassShader.setUniform2f("u_inverseScreenSize", 1.0f/ofGetWindowWidth(), 1.0f/ofGetWindowHeight());
  m_pointLightPassShader.end();

  glActiveTexture(GL_TEXTURE0);
}

void testApp::unbindGBufferTextures() {
  // unbind textures and reset active texture back to zero (OF expects it at 0 - things like ofDrawBitmapString() will break otherwise)
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_ALBEDO]); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_POSITION]); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_NORMAL]); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_DEPTH]); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_SSAO]); glBindTexture(GL_TEXTURE_2D, 0);
  
  glActiveTexture(GL_TEXTURE0);
}

void testApp::geometryPass() {
  // CREATE GBUFFER
  // --------------
  // start our GBuffer for writing (pass in near and far so that we can create linear depth values in that range)
  m_gBuffer.bindForWriting(m_cam.getNearClip(), m_cam.getFarClip());
  
  m_cam.begin();
  
  glActiveTexture(GL_TEXTURE0); // bind concrete texture
  m_texture.getTextureReference().bind();
  
  // draw our randomly rotate boxes
  for (vector<Box>::iterator it=m_boxes.begin() ; it < m_boxes.end(); it++) {
    ofPushMatrix();
    ofRotate(it->angle, it->axis_x, it->axis_y, it->axis_z);
    ofBox( it->pos, it->size );
    ofPopMatrix();
  }
  
  // draw all of our light spheres so we can see where our lights are at
  for (vector<PointLight>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
    ofSphere(it->getGlobalPosition(), 0.25f);
  }
  
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture
  
  m_cam.end();
  
  m_gBuffer.unbindForWriting(); // done rendering out to our GBuffer  
}

void testApp::pointLightPass() {
  
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
  
  // enable additive blending since we're going to add all the light colours together as they are individually drawn
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);
  
  ofSetSphereResolution(8.0f);
  
  glClear(GL_COLOR_BUFFER_BIT);

  m_cam.begin();
  
  m_pointLightPassShader.begin();

    // pass in lighting info
    int numLights = m_lights.size();
    m_pointLightPassShader.setUniform1i("u_numLights", numLights);

    ofMatrix4x4 camModelViewMatrix = m_cam.getModelViewMatrix(); // need to multiply light positions by camera's modelview matrix to transform them from world space to view space (reason for this is our normals and positions in the GBuffer are in view space so we must do our lighting calculations in the same space). It's faster to do it here on CPU vs. in shader

    for (vector<PointLight>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
      ofVec3f lightPos = it->getPosition();
      ofVec3f lightPosInViewSpace = it->getPosition() * camModelViewMatrix;

      m_pointLightPassShader.setUniform3fv("u_lightPosition", &lightPosInViewSpace.getPtr()[0]);
      m_pointLightPassShader.setUniform4fv("u_lightAmbient", it->ambient);
      m_pointLightPassShader.setUniform4fv("u_lightDiffuse", it->diffuse);
      m_pointLightPassShader.setUniform4fv("u_lightSpecular", it->specular);
      m_pointLightPassShader.setUniform1f("u_lightIntensity", it->intensity);
      m_pointLightPassShader.setUniform3fv("u_lightAttenuation", it->attenuation);
      
      float radius = it->intensity * skMaxPointLightRadius;
      
      m_pointLightPassShader.setUniform1f("u_lightRadius", radius);
      
      ofPushMatrix();
      ofTranslate(lightPos);
      ofScale(radius, radius, radius);
      ofSphere(0, 0, 0, 1.0f);
      ofPopMatrix();
    }

  m_pointLightPassShader.end();

  m_cam.end();

  glDisable(GL_BLEND);
  
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void testApp::deferredRender() {
  // final deferred shading pass
  glDisable(GL_DEPTH_TEST);
  ofDisableLighting();
  
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, m_renderTex);
  
  m_shader.begin();
    m_shader.setUniform1i("u_pointLightPassTex", 7);
    drawScreenQuad();
  m_shader.end();

  glActiveTexture(GL_TEXTURE0);
}

//--------------------------------------------------------------
void testApp::update() {
  m_angle += 1.0f;
  
  int count = 0;
  float time = ofGetElapsedTimeMillis()/1000.0f;
  
  // orbit our lights around (0, 0, 0) in a random orbit direction that was assigned when
  // we created them
  for (vector<PointLight>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
    float percent = count/(float)m_lights.size();
    it->rotateAround(percent * 0.25f + 0.1f, it->orbitAxis, ofVec3f(0.0f, 0.0f, 0.0f));
    
    if (m_bPulseLights) {
      it->intensity = 0.5f + 0.25f * (1.0f + cosf(time + percent*PI)); // pulse between 0.5 and 1
    }
    
    ++count;
  }
}

//--------------------------------------------------------------
void testApp::draw() {
  
  glEnable(GL_CULL_FACE); // cull backfaces
  glCullFace(GL_BACK);
  
  glColor4f(1.0, 1.0, 1.0, 1.0);

  
  geometryPass();
  pointLightPass();
  
  
  // GENERATE SSAO TEXTURE
  // ---------------------
  // pass in texture units. ssaoPass.applySSAO() expects the required textures to already be bound at these units
  m_ssaoPass.applySSAO(m_textureUnits[TEX_UNIT_POSITION],
                       m_textureUnits[TEX_UNIT_NORMAL],
                       m_textureUnits[TEX_UNIT_DEPTH]);
  
  

  deferredRender();

  
  if (m_bDrawDebug) {
    m_gBuffer.drawDebug(0, 0);
    m_ssaoPass.drawDebug(0, ofGetWindowHeight()/4);

    // draw our debug/message string
    ofEnableAlphaBlending();
    glDisable(GL_CULL_FACE); // need to do this to draw planes in OF because of vertex ordering

    ofSetColor(255, 255, 255, 255);
    char debug_str[255];
    sprintf(debug_str, "Frame rate: %f\nNumber of lights: %li\nPress SPACE to toggle drawing of debug buffers\nPress +/- to add and remove lights\n'p' to toggle pulsing of light intensity\n'r' to randomize light colours", ofGetFrameRate(), m_lights.size());
    ofDrawBitmapString(debug_str, ofPoint(15, 20));
  }
}

void testApp::drawScreenQuad() {
  // set identity matrices and save current matrix state
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  // draw the full viewport quad
  m_quadVbo.draw(GL_QUADS, 0, 4);
  
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
  if (key == ' ') {
    m_bDrawDebug = !m_bDrawDebug;
  } else if (key == '+' || key == '=') {
    addRandomLight();
  } else if (key == '-' ) {
    if (m_lights.size()) {
      m_lights.pop_back();
    }
  } else if (key == 'p') {
    m_bPulseLights = !m_bPulseLights;
  } else if (key == 'r') {
    randomizeLightColors();
  }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
