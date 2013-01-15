#version 120

varying vec2 v_texCoord;

void main(void)
{
  v_texCoord = gl_MultiTexCoord0.st;
  
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
