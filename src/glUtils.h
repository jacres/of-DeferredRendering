//
//  glUtils.h
//  deferred
//
//  Created by james on 13-01-14.
//
//

#ifndef deferred_glUtils_h
#define deferred_glUtils_h

#include "ofMain.h"

using namespace glUtils;

class
const ofVec2f s_quadVerts[]

const ofVec2f ShadowMapLight::s_quadVerts[] = {
  ofVec2f( -1.0f, -1.0f ),
  ofVec2f( 1.0f, -1.0f ),
  ofVec2f( 1.0f, 1.0f ),
  ofVec2f( -1.0f, 1.0f )
};

const ofVec2f ShadowMapLight::s_quadTexCoords[] = {
  ofVec2f(0.0f, 0.0f),
  ofVec2f(1.0f, 0.0f),
  ofVec2f(1.0f, 1.0f),
  ofVec2f(0.0f, 1.0f)
};

// full viewport quad vbo
s_quadVbo.setVertexData( &s_quadVerts[0], 4, GL_STATIC_DRAW );
s_quadVbo.setTexCoordData( &s_quadTexCoords[0], 4, GL_STATIC_DRAW );


#endif
