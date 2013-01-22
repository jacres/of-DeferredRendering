#version 120

void main(void)
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
