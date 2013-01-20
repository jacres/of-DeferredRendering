#version 120

varying vec4 v_vertex;

void main(void)
{
  v_vertex = gl_ModelViewMatrix * gl_Vertex;
  
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
