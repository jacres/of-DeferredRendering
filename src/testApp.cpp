//  testApp.cpp
//
//  Created by James Acres on 13-01-13
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#include "testApp.h"

testApp::testApp() :
m_angle(0),
m_windowWidth(0),
m_windowHeight(0),
m_bDrawDebug(true),
m_bPulseLights(false)
{};

//--------------------------------------------------------------
void testApp::setup() {
  ofSetVerticalSync(false); // can cause problems on some Linux implementations

  ofDisableArbTex();

  setupScreenQuad();

  m_cam.setupPerspective( false, 60.0f, 0.1f, 100.0f );
  m_cam.setDistance(40.0f);
  m_cam.setGlobalPosition( 0.0f, 0.0f, 35.0f );
  m_cam.lookAt( ofVec3f( 0.0f, 0.0f, 0.0f ) );

  m_shader.load("shaders/mainScene.vert", "shaders/mainScene.frag");
  m_pointLightPassShader.load("shaders/pointLightPass.vert", "shaders/pointLightPass.frag");
  m_pointLightStencilShader.load("shaders/pointLightStencil.vert", "shaders/pointLightStencil.frag");

  m_texture.loadImage("textures/concrete.jpg");

  setupLights();
  createRandomBoxes();

  ofMesh boxMesh = Primitives::getBoxMesh(1.0f, 1.0f, 1.0f);
  m_numBoxVerts = boxMesh.getNumVertices();
  m_boxVbo.setMesh(boxMesh, GL_STATIC_DRAW);

  ofMesh sphereMesh = Primitives::getSphereMesh(6);
  m_numSphereVerts = sphereMesh.getVertices().size();
  m_sphereVbo.setMesh(sphereMesh, GL_STATIC_DRAW);
}

void testApp::resizeBuffersAndTextures() {

  if (ofGetWindowMode() == OF_FULLSCREEN) {
    m_windowWidth = ofGetScreenWidth();
    m_windowHeight = ofGetScreenHeight();
  } else {
    m_windowWidth = ofGetWindowWidth();
    m_windowHeight = ofGetWindowHeight();
  }

  m_gBuffer.setup(m_windowWidth, m_windowHeight);
  m_ssaoPass.setup(m_windowWidth, m_windowHeight);

  // set our camera parameters for ssao pass - inverse proj matrix + far clip are used in shader to recreate position from linear depth
  m_ssaoPass.setCameraProperties(m_cam.getProjectionMatrix().getInverse(), m_cam.getFarClip());

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
  // create randomly rotated boxes
  for (unsigned int i=0; i<skNumBoxes; i++) {
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
  posOnSphere.scale(ofRandom(0.95f, 1.05f));

  ofVec3f orbitAxis = ofVec3f(ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f));
  orbitAxis.normalize();
  l.orbitAxis = orbitAxis;

  posOnSphere.scale(skRadius-1);

  l.setPosition(posOnSphere);
  l.setAmbient(0.0f, 0.0f, 0.0f);

  ofVec3f col = ofVec3f(ofRandom(0.3f, 0.5f), ofRandom(0.2f, 0.4f), ofRandom(0.7f, 1.0f));
  l.setDiffuse(col.x, col.y, col.z);
  l.setSpecular(col.x, col.y, col.z);
  l.setAttenuation(0.0f, 0.0f, 0.2f); // set constant, linear, and exponential attenuation
  l.intensity = 0.8f;

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
  m_textureUnits[TEX_UNIT_ALBEDO] = 15;
  m_textureUnits[TEX_UNIT_NORMALS_DEPTH] = 14;
  m_textureUnits[TEX_UNIT_SSAO] = 13;
  m_textureUnits[TEX_UNIT_POINTLIGHT_PASS] = 12;

  // bind all GBuffer textures
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_ALBEDO]);
  glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(GBuffer::GBUFFER_TEXTURE_TYPE_ALBEDO));

  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_NORMALS_DEPTH]);
  glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(GBuffer::GBUFFER_TEXTURE_TYPE_NORMALS_DEPTH));

  // bind SSAO texture
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_SSAO]);
  glBindTexture(GL_TEXTURE_2D, m_ssaoPass.getTextureReference());

  // point light pass texture
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_POINTLIGHT_PASS]);
  glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(GBuffer::GBUFFER_TEXTURE_TYPE_LIGHT_PASS));

  m_shader.begin();  // our final deferred scene shader
  m_shader.setUniform1i("u_albedoTex", m_textureUnits[TEX_UNIT_ALBEDO]);
  m_shader.setUniform1i("u_ssaoTex", m_textureUnits[TEX_UNIT_SSAO]);
  m_shader.setUniform1i("u_pointLightPassTex", m_textureUnits[TEX_UNIT_POINTLIGHT_PASS]);
  m_shader.end();

  m_pointLightPassShader.begin();  // our point light pass shader
  m_pointLightPassShader.setUniform1i("u_albedoTex", m_textureUnits[TEX_UNIT_ALBEDO]);
  m_pointLightPassShader.setUniform1i("u_normalAndDepthTex", m_textureUnits[TEX_UNIT_NORMALS_DEPTH]);
  m_pointLightPassShader.setUniform1i("u_ssaoTex", m_textureUnits[TEX_UNIT_SSAO]);
  m_pointLightPassShader.setUniform2f("u_inverseScreenSize", 1.0f/m_windowWidth, 1.0f/m_windowHeight);
  m_pointLightPassShader.end();

  glActiveTexture(GL_TEXTURE0);
}

void testApp::unbindGBufferTextures() {
  // unbind textures and reset active texture back to zero (OF expects it at 0 - things like ofDrawBitmapString() will break otherwise)
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_ALBEDO]); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_NORMALS_DEPTH]); glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0 + m_textureUnits[TEX_UNIT_SSAO]); glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);
}

void testApp::geometryPass() {

  glDisable(GL_STENCIL_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // CREATE GBUFFER
  // --------------
  // start our GBuffer for writing (pass in near and far so that we can create linear depth values in that range)
  m_gBuffer.bindForGeomPass(m_cam.getNearClip(), m_cam.getFarClip());

  m_cam.begin();

  glActiveTexture(GL_TEXTURE0); // bind concrete texture
  m_texture.getTextureReference().bind();

  m_boxVbo.bind();

  // draw our randomly rotate boxes
  for (vector<Box>::iterator it=m_boxes.begin() ; it < m_boxes.end(); it++) {
    ofPushMatrix();
    ofRotate(it->angle, it->axis_x, it->axis_y, it->axis_z);
    ofScale(it->size, it->size, it->size);
    glDrawArrays(GL_TRIANGLES, 0, m_numBoxVerts);
    ofPopMatrix();
  }

  m_boxVbo.unbind();

  // draw all of our light spheres so we can see where our lights are at
  m_sphereVbo.bind();

  for (vector<PointLight>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
    ofPushMatrix();
    ofTranslate(it->getGlobalPosition());
    ofScale(0.25f, 0.25f, 0.25f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_numSphereVerts);
    ofPopMatrix();
  }

  m_sphereVbo.unbind();

  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

  m_cam.end();

  m_gBuffer.unbindForGeomPass(); // done rendering out to our GBuffer
}

void testApp::pointLightStencilPass() {
}

void testApp::pointLightPass() {

  m_gBuffer.resetLightPass();

  m_pointLightPassShader.begin();
  // pass in lighting info
  int numLights = m_lights.size();
  m_pointLightPassShader.setUniform1i("u_numLights", numLights);
  m_pointLightPassShader.setUniform1f("u_farDistance", m_cam.getFarClip());
  m_pointLightPassShader.end();

  m_cam.begin();

    ofMatrix4x4 camModelViewMatrix = m_cam.getModelViewMatrix(); // need to multiply light positions by camera's modelview matrix to transform them from world space to view space (reason for this is our normals and positions in the GBuffer are in view space so we must do our lighting calculations in the same space). It's faster to do it here on CPU vs. in shader

    m_sphereVbo.bind();

    for (vector<PointLight>::iterator it = m_lights.begin(); it != m_lights.end(); it++) {
      ofVec3f lightPos = it->getPosition();
      float radius = it->intensity * skMaxPointLightRadius;

      // STENCIL
      // this pass creates a stencil so that only the affected pixels are shaded - prevents us shading things that are outside our light volume
      m_gBuffer.bindForStencilPass();
      m_pointLightStencilShader.begin();
        ofPushMatrix();
        ofTranslate(lightPos);
        ofScale(radius, radius, radius);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_numSphereVerts);
        ofPopMatrix();
      m_pointLightStencilShader.end();

      // SHADING/LIGHTING CALCULATION
      // this pass draws the spheres representing the area of influence each light has
      // in a fragment shader, only the pixels that are affected by the drawn geometry are processed
      // drawing light volumes (spheres for point lights) ensures that we're only running light calculations on
      // the areas that the spheres affect
      m_gBuffer.bindForLightPass();
      m_pointLightPassShader.begin();
        ofVec3f lightPosInViewSpace = it->getPosition() * camModelViewMatrix;

        m_pointLightPassShader.setUniform3fv("u_lightPosition", &lightPosInViewSpace.getPtr()[0]);
        m_pointLightPassShader.setUniform4fv("u_lightAmbient", it->ambient);
        m_pointLightPassShader.setUniform4fv("u_lightDiffuse", it->diffuse);
        m_pointLightPassShader.setUniform4fv("u_lightSpecular", it->specular);
        m_pointLightPassShader.setUniform1f("u_lightIntensity", it->intensity);
        m_pointLightPassShader.setUniform3fv("u_lightAttenuation", it->attenuation);

        m_pointLightPassShader.setUniform1f("u_lightRadius", radius);

        ofPushMatrix();
        ofTranslate(lightPos);
        ofScale(radius, radius, radius);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_numSphereVerts);
        ofPopMatrix();

      m_pointLightPassShader.end();
      glDisable(GL_STENCIL_TEST);
      glDisable(GL_BLEND);
      glCullFace(GL_BACK);
    }

  m_sphereVbo.unbind();

  m_gBuffer.unbind();
  m_cam.end();
}

void testApp::deferredRender() {

  // final deferred shading pass
  glDisable(GL_DEPTH_TEST);
  ofDisableLighting();

  m_shader.begin();
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
    it->rotateAround(percent * 0.2f + 0.1f, it->orbitAxis, ofVec3f(0.0f, 0.0f, 0.0f));

    if (m_bPulseLights) {
      it->intensity = 0.5f + 0.25f * (1.0f + cosf(time + percent*PI)); // pulse between 0.5 and 1
    }

    ++count;
  }
}

//--------------------------------------------------------------
void testApp::draw() {
  geometryPass();
  pointLightPass();

  // GENERATE SSAO TEXTURE
  // ---------------------
  // pass in texture units. ssaoPass.applySSAO() expects the required textures to already be bound at these units
  m_ssaoPass.applySSAO(m_textureUnits[TEX_UNIT_NORMALS_DEPTH]);

  deferredRender();

  if (m_bDrawDebug) {
    m_gBuffer.drawDebug(0, 0);
    m_ssaoPass.drawDebug(ofGetWidth()/4*3, 0);

    // draw our debug/message string
    ofEnableAlphaBlending();
    glDisable(GL_CULL_FACE); // need to do this to draw planes in OF because of vertex ordering

    ofSetColor(255, 255, 255, 255);
    char debug_str[255];
    sprintf(debug_str, "Framerate: %f\nNumber of lights: %li\nPress SPACE to toggle drawing of debug buffers\nPress +/- to add and remove lights\n'p' to toggle pulsing of light intensity\n'r' to randomize light colours", ofGetFrameRate(), m_lights.size());
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
  } else if (key == 'f') {
    ofToggleFullscreen();
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
  resizeBuffersAndTextures();
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
