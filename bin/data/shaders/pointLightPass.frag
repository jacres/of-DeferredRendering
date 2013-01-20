#version 120

// deferred g buffers
uniform sampler2D u_albedoTex;  // albedo (diffuse without lighting)
uniform sampler2D u_normalAndDepthTex;  // view space normal and linear depth

// LIGHTS
uniform int u_numLights;
uniform vec3 u_lightPosition;
uniform vec4 u_lightAmbient;
uniform vec4 u_lightDiffuse;
uniform vec4 u_lightSpecular;
uniform vec3 u_lightAttenuation;
uniform float u_lightIntensity;
uniform float u_lightRadius;

uniform vec2 u_inverseScreenSize;
uniform float u_farDistance;

varying vec4 v_vertex;

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
  vec2 texCoord = gl_FragCoord.xy * u_inverseScreenSize.xy;
  
  float linearDepth = texture2D(u_normalAndDepthTex, texCoord.st).a;
  
  // vector to far plane
  vec3 viewRay = vec3(v_vertex.xy * (-u_farDistance/v_vertex.z), -u_farDistance);
  // scale viewRay by linear depth to get view space position
  vec3 vertex = viewRay * linearDepth;
  
  vec3 normal = texture2D(u_normalAndDepthTex, texCoord.st).xyz;

  vec4 ambient = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

  vec3 lightDir = u_lightPosition - vertex;
  vec3 R = normalize(reflect(lightDir, normal));
  vec3 V = normalize(vertex);

  float lambert = max(dot(normal, normalize(lightDir)), 0.0);
  
  if (lambert > 0.0) {
    float distance = length(lightDir);
    
    if (distance <= u_lightRadius) {
      // different attenuation methods - we have to stay within bounding radius, so it's a bit trickier than forward rendering
//      float attenuation = 1.0 - distance/u_lightRadius;
//      float attenuation = 1.0 / (u_lightAttenuation.x + u_lightAttenuation.y * distance + u_lightAttenuation.z * distance * distance);
//      //attenuation = max(1.0, attenuation);

//      (1-(x/r)^2)^3
//      float attenuation = (1.0 - pow(pow(distance/u_lightRadius, 2), 3));
  
      float distancePercent = distance/u_lightRadius;
      float damping_factor = 1.0 - pow(distancePercent, 3);
      float attenuation = 1.0/(u_lightAttenuation.x +
                               u_lightAttenuation.y * distance +
                               u_lightAttenuation.z * distance * distance);
      attenuation *= damping_factor;
      
      vec4 diffuseContribution = material1.diffuse * u_lightDiffuse * lambert;
      diffuseContribution *= u_lightIntensity;
      diffuseContribution *= attenuation;
      
      vec4 specularContribution = material1.specular * u_lightSpecular * pow(max(dot(R, V), 0.0), material1.shininess);
      specularContribution *= u_lightIntensity;
      specularContribution *= attenuation;
      
      diffuse += diffuseContribution;
      specular += specularContribution;
    }
  }
    
  vec4 final_color = vec4(ambient + diffuse + specular);
  
  gl_FragColor = vec4(final_color.rgb, 1.0);
}
