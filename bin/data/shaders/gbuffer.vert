#version 120

uniform float u_farDistance;

varying vec3 v_normal;
varying vec2 v_texCoord;
varying float v_depth;

void main (void)
{
  vec4 viewSpaceVertex = gl_ModelViewMatrix * gl_Vertex;
  v_normal = gl_NormalMatrix * gl_Normal;
  v_texCoord = gl_MultiTexCoord0.st;
  v_depth = -viewSpaceVertex.z / u_farDistance;
	
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}