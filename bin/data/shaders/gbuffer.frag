#version 120

uniform sampler2D u_texture;

varying vec3 v_normal;
varying vec2 v_texCoord;
varying float v_depth;

void main(void)
{
  vec3 diffuse = texture2D(u_texture, v_texCoord.st).rgb;
  gl_FragData[0] = vec4(diffuse, 1.0); // albedo
  gl_FragData[1] = vec4(normalize(v_normal), v_depth); // normals + depth
}