#version 120

// deferred g buffers
uniform sampler2D u_albedoTex;  // albedo (diffuse without lighting)
uniform sampler2D u_ssaoTex; // SSAO values
uniform sampler2D u_pointLightPassTex;

varying vec2 v_texCoord;

struct material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

const material material1 = material(
  vec4(0.1, 0.1, 0.1, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  127.0
);

const vec4 ambientGlobal = vec4(0.05, 0.05, 0.05, 1.0);

void main(void)
{
  vec4 albedo = texture2D(u_albedoTex, v_texCoord.st);
  float ssao = texture2D(u_ssaoTex, v_texCoord.st).r;
  
  vec4 pointLightContribution = texture2D(u_pointLightPassTex, v_texCoord.st);

  vec4 ambient = ambientGlobal + material1.ambient;
  
  vec4 lightContribution = vec4(0.0, 0.0, 0.0, 1.0);
  lightContribution += pointLightContribution;
  
  vec4 final_color = (ambient * ssao + lightContribution) * albedo;
  
  gl_FragColor = vec4(final_color.rgb, 1.0);
}
