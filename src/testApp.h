//  testApp.h
//
//  Created by James Acres on 12-08-15
//  http://www.jamesacres.com
//  http://github.com/jacres
//  @jimmyacres

#pragma once

#include "ofMain.h"
#include "gBuffer.h"
#include "ssaoPass.h"
#include "pointLight.h"

class testApp : public ofBaseApp {

  struct Box {
    ofVec3f pos;
    float size;
    float angle;
    float axis_x;
    float axis_y;
    float axis_z;
    
    Box(ofVec3f pos=ofVec3f(0.0f, 0.0f, 0.0f), float angle=0.0f, float ax=0.0f, float ay=0.0f, float az=0.0f, float size=2.0f) :
      pos(pos),
      size(size),
      angle(angle),
      axis_x(ax),
      axis_y(ay),
      axis_z(az)
    {}
  };
  
  static const int skNumLights = 100;
  static const int skRadius = 20;
  static const int skMaxPointLightRadius = 10;

  enum TEXTURE_UNITS {
    TEX_UNIT_ALBEDO,
    TEX_UNIT_NORMALS_DEPTH,
    TEX_UNIT_SSAO,
    TEX_UNIT_POINTLIGHT_PASS,
    TEX_UNIT_NUM_UNITS
  };

public:
  testApp();
  
  void setup();
  void update();
  void draw();
  
  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);
  
  void setupModel();
  void setupLights();
  void setupScreenQuad();
  
  void addRandomLight();
  void createRandomBoxes();
  void randomizeLightColors();
  
  void unbindGBufferTextures();
  void bindGBufferTextures();
  
  void drawScreenQuad();
  
  void geometryPass();
  void pointLightPass();
  void deferredRender();
  
  GBuffer m_gBuffer;
  SSAOPass m_ssaoPass;

  ofVbo m_quadVbo;

  ofShader m_shader;
  ofShader m_pointLightPassShader;
  
  ofEasyCam m_cam;
  
  ofImage m_texture;
  
  GLuint m_textureUnits[TEX_UNIT_NUM_UNITS];
  GLuint m_fbo;
  GLuint m_renderTex;
  
  float   m_angle;    
  
  bool    m_bDrawDebug;
  bool    m_bPulseLights;
  
  float m_windowWidth;
  float m_windowHeight;
  
  vector<Box> m_boxes;
  vector<PointLight> m_lights;
};
