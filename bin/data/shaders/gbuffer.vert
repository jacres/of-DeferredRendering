#version 120

varying vec3 v_normal;
varying vec3 v_vertex;
varying vec2 v_texCoord;

void main (void)
{
  v_vertex = vec3(gl_ModelViewMatrix * gl_Vertex);
  v_normal = gl_NormalMatrix * gl_Normal;
  v_texCoord = gl_MultiTexCoord0.st;
	
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}