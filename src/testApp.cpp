//  testApp.cpp
//
//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#include "testApp.h"

testApp::testApp() :
m_angle(0),
m_bDrawDebug(true)
{};

//--------------------------------------------------------------
void testApp::setup() {
//  ofSetVerticalSync(false); // can cause problems on some Linux implementations
  ofDisableArbTex();
  ofSetSphereResolution(6);
  
  // set up our gbuffer and ssao pass
  m_gBuffer.setup(ofGetWindowWidth(), ofGetWindowHeight());
  m_ssaoPass.setup(ofGetWindowWidth(), ofGetWindowHeight(), 12);

  setupScreenQuad();

  m_cam.setupPerspective( false, 45.0f, 0.1f, 100.0f );
  m_cam.setDistance(40.0f);
  m_cam.setGlobalPosition( 0.0f, 0.0f, 50.0f );
  m_cam.lookAt( ofVec3f( 0.0f, 0.0f, 0.0f ) );
  
  m_shader.load( "shaders/mainScene.vert", "shaders/mainScene.frag" );
  
  m_texture.loadImage("textures/concrete.jpg");
  
  setupLights();
  createRandomBoxes();
  
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
    float size = 20.0f;
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
  Light l;
  ofVec3f posOnSphere = ofVec3f(ofRandom(-1.0f, 1.0f), ofRandom(-1.0f, 1.0f), ofRandom(-1.0f, 1.0f));
  posOnSphere.normalize();
  
  ofVec3f orbitAxis = ofVec3f(ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f));
  orbitAxis.normalize();
  l.orbitAxis = orbitAxis;
  
  posOnSphere.scale(skRadius-1);
  
  l.setPosition(posOnSphere);
  l.setAmbient(0.0f, 0.0f, 0.0f);
  
  ofVec3f col = ofVec3f(ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f));
  l.setDiffuse(col.x, col.y, col.z);
  l.setSpecular(col.x, col.y, col.z);
  
  m_lights.push_back(l);
}

void testApp::bindGBufferTextures() {
  // set up the texture units we want to use - we're using them every frame, so we'll leave them bound to these units to save speed vs. binding/unbinding
  m_textureUnits[TEX_UNIT_ALBEDO] = 11;
  m_textureUnits[TEX_UNIT_POSITION] = 12;
  m_textureUnits[TEX_UNIT_NORMAL] = 13;
  m_textureUnits[TEX_UNIT_DEPTH] = 14;
  m_textureUnits[TEX_UNIT_SSAO] = 15;
  
  
  m_shader.begin();  // our final deferred scene shader
  
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
  
  m_shader.setUniform1i("u_albedoTex", m_textureUnits[TEX_UNIT_ALBEDO]);
  m_shader.setUniform1i("u_positionTex", m_textureUnits[TEX_UNIT_POSITION]);
  m_shader.setUniform1i("u_normalTex", m_textureUnits[TEX_UNIT_NORMAL]);
  m_shader.setUniform1i("u_ssaoTex", m_textureUnits[TEX_UNIT_SSAO]);

  m_shader.end();
  
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

//--------------------------------------------------------------
void testApp::update() {
  m_angle += 1.0f;
  
  int count = 0;
  
  // orbit our lights around (0, 0, 0) in a random orbit direction that was assigned when
  // we created them
  for (vector<Light>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
    float percent = count/(float)m_lights.size();
    it->rotateAround(percent * 0.25f + 0.1f, it->orbitAxis, ofVec3f(0.0f, 0.0f, 0.0f));
    ++count;
  }
}

//--------------------------------------------------------------
void testApp::draw() {
  
  glEnable( GL_DEPTH_TEST );

  glEnable(GL_CULL_FACE); // cull backfaces
  glCullFace(GL_BACK);
  
  glColor4f(1.0, 1.0, 1.0, 1.0);

  
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
    for (vector<Light>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
      ofSphere(it->getGlobalPosition(), 0.25f);
    }

    glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

    m_cam.end();
  
  m_gBuffer.unbindForWriting(); // done rendering out to our GBuffer
  
  
  // GENERATE SSAO TEXTURE
  // ---------------------
  // pass in texture units. ssaoPass.applySSAO() expects the required textures to already be bound at these units
  m_ssaoPass.applySSAO(m_textureUnits[TEX_UNIT_POSITION],
                       m_textureUnits[TEX_UNIT_NORMAL],
                       m_textureUnits[TEX_UNIT_DEPTH]);
  
  
  // FINAL DEFERRED RENDERING PASS
  // -----------------------------
  // put all light positions and colours into float arrays so they can be sent as uniforms
  // very inefficient way of doing this - UBOs would be better, or using arrays with pointers to each rgba block
  
  ofMatrix4x4 camModelViewMatrix = m_cam.getModelViewMatrix(); // need to multiply light positions by camera's modelview matrix to transform them from world space to view space (reason for this is our normals and positions in the GBuffer are in view space so we must do our lighting calculations in the same space). It's faster to do it here on CPU vs. in shader
  
  vector<float> lightPositions;
  vector<float> ambient;
  vector<float> diffuse;
  vector<float> specular;
  
  for (vector<Light>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
    ofVec3f lightPosInViewSpace = it->getPosition() * camModelViewMatrix;
    
    lightPositions.insert(lightPositions.end(), lightPosInViewSpace.getPtr(), lightPosInViewSpace.getPtr()+3);
    ambient.insert(ambient.end(), it->ambient, it->ambient+4);
    diffuse.insert(diffuse.end(), it->diffuse, it->diffuse+4);
    specular.insert(specular.end(), it->specular, it->specular+4);
  }
  
  // final deferred shading pass
  m_shader.begin();
  
    // pass in lighting info
    int numLights = m_lights.size();
    m_shader.setUniform1i("u_numLights", numLights);
    m_shader.setUniform3fv("u_lightPosition", &lightPositions[0], numLights);
    m_shader.setUniform4fv("u_lightAmbient", &ambient[0], numLights);
    m_shader.setUniform4fv("u_lightDiffuse", &diffuse[0], numLights);
    m_shader.setUniform4fv("u_lightSpecular", &specular[0], numLights);
    
    drawScreenQuad(); // draw full screen quad
  
  m_shader.end();
  
  // draw our debug/message string
  ofEnableAlphaBlending();
  ofDisableLighting();
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  if (m_bDrawDebug) {
    m_gBuffer.drawDebug(0, 0);
    m_ssaoPass.drawDebug(ofGetWindowWidth()/4 * 3, 0);
  }
  
  ofSetColor(255, 255, 255, 255);
  char debug_str[255];
  sprintf(debug_str, "Frame rate: %f\nNumber of lights: %li\nPress SPACE to toggle drawing of debug buffers\nPress +/- to add and remove lights", ofGetFrameRate(), m_lights.size());
  ofDrawBitmapString(debug_str, ofPoint(15, 20));
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
  } if (key == '+' || key == '=') {
    addRandomLight();
  } else if (key == '-' ) {
    if (m_lights.size()) {
      m_lights.pop_back();
    }
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
