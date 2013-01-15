#version 120

uniform sampler2D u_texture;
uniform float u_linearDepthConstant;

varying vec3 v_normal;
varying vec3 v_vertex;
varying vec2 v_texCoord;

void main(void)
{
  vec3 diffuse = texture2D(u_texture, v_texCoord.st).rgb;
  float linearDepth = length(v_vertex) * u_linearDepthConstant;

  gl_FragData[0] = vec4(diffuse, 1.0); // albedo
  gl_FragData[1] = vec4(v_vertex, 1.0); // position
  gl_FragData[2] = vec4(normalize(v_normal), 1.0); // normal
  gl_FragData[3] = vec4(linearDepth, 0.0, 0.0, 1.0); // linear depth
}