#version 120

const int MAX_LIGHTS = 100;

uniform mat4 u_shadowTransMatrix;
uniform sampler2D	u_shadowMapTex;
uniform vec4 u_shadowLightPosition;

// deferred g buffers
uniform sampler2D u_albedoTex;  // albedo (diffuse without lighting)
uniform sampler2D u_positionTex;  // view space position
uniform sampler2D u_normalTex;  // view space normal and linear depth
uniform sampler2D u_linearDepthTex; // linear depth

uniform sampler2D u_ssaoTex; // SSAO values

uniform float u_linearDepthConstant;

// LIGHTS
uniform int u_numLights;
uniform vec3 u_lightPosition[MAX_LIGHTS];
uniform vec4 u_lightAmbient[MAX_LIGHTS];
uniform vec4 u_lightDiffuse[MAX_LIGHTS];
uniform vec4 u_lightSpecular[MAX_LIGHTS];

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

const float constant = 1.0;
const float linear = 1.0;
const float exponential = 0.06;

void main(void)
{
  vec4 albedo = texture2D(u_albedoTex, v_texCoord.st);
  vec3 vertex = texture2D(u_positionTex, v_texCoord.st).xyz;
  vec3 normal = normalize(texture2D(u_normalTex, v_texCoord.st).xyz);
  float ssao = texture2D(u_ssaoTex, v_texCoord.st).r;

  vec4 ambient = ambientGlobal + material1.ambient;
  vec4 diffuse;
  vec4 specular;

  // loop through all lights
  for (int i=0; i<u_numLights; i++)
  {
    vec3 lightDir = u_lightPosition[i] - vertex;
    vec3 R = normalize(reflect(lightDir, normal));
    vec3 V = normalize(vertex);

    float lambert = max(dot(normal, normalize(lightDir)), 0.0);
    float distance = length(lightDir);
    float attenuation = 1.0/(exponential * distance * distance);

    diffuse += attenuation * (material1.diffuse * u_lightDiffuse[i] * lambert);
    specular += attenuation * (material1.specular * u_lightSpecular[i] * pow(max(dot(R, V), 0.0), material1.shininess));
  }
 
  vec4 final_color = (ambient * ssao + diffuse + specular) * albedo;

  gl_FragColor = vec4(final_color.rgb, 1.0);
}
