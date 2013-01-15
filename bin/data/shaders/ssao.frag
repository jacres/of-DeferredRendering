#version 120

uniform sampler2D u_randomJitterTex;  // Normalmap to randomize the sampling kernel
uniform sampler2D u_viewSpacePositionTex;  // view space position 
uniform sampler2D u_normalTex;  // view space normal and linear depth
uniform sampler2D u_linearDepthTex; // linear depth

uniform float u_texelWidth;
uniform float u_texelHeight;

uniform float u_occluderBias;
uniform float u_samplingRadius;
uniform vec2 u_attenuation; // .x constant, .y linear, .z quadratic (unused)

varying vec3 v_vertex;
varying vec2 v_texCoord;

/// Sample the ambient occlusion at the following UV coordinate.
float SamplePixels(vec3 srcPosition, vec3 srcNormal, vec2 uv)
{
  // Get the 3D position of the destination pixel
  vec3 dstPosition = texture2D(u_viewSpacePositionTex, uv).xyz;

  // Calculate ambient occlusion amount between these two points
  // It is simular to diffuse lighting. Objects directly above the fragment cast
  // the hardest shadow and objects closer to the horizon have minimal effect.
  vec3 positionVec = dstPosition - srcPosition;
  float intensity = max(dot(normalize(positionVec), srcNormal) - u_occluderBias, 0.0);

  // Attenuate the occlusion, similar to how you attenuate a light source.
  // The further the distance between points, the less effect AO has on the fragment.
  float dist = length(positionVec);
  float attenuation = 1.0 / (u_attenuation.x + (u_attenuation.y * dist));

  return intensity * attenuation;
}
                                
void main ()
{
  // Get position and normal vector for this fragment
  vec3 srcNormal = texture2D(u_normalTex, v_texCoord).xyz;
  vec2 randVec = normalize(texture2D(u_randomJitterTex, v_texCoord).xy * 2.0 - 1.0);
  
  float srcDepth = texture2D(u_linearDepthTex, v_texCoord).r;
  vec3 srcPosition = texture2D(u_viewSpacePositionTex, v_texCoord).xyz;

  // The following variable specifies how many pixels we skip over after each
  // iteration in the ambient occlusion loop. We can't sample every pixel within
  // the sphere of influence because that's too slow. We only need to sample
  // some random pixels nearby to apprxomate the solution.
  //
  // Pixels far off in the distance will not sample as many pixels as those close up.
  float kernelRadius = u_samplingRadius * (1.0 - srcDepth);

  // Sample neighbouring pixels
  vec2 kernel[4];
  kernel[0] = vec2(0.0, 1.0); // top
  kernel[1] = vec2(1.0, 0.0); // right
  kernel[2] = vec2(0.0, -1.0);    // bottom
  kernel[3] = vec2(-1.0, 0.0);    // left

  const float Sin45 = 0.707107;   // 45 degrees = sin(PI / 4)

  // Sample from 16 pixels, which should be enough to appromixate a result. You can
  // sample from more pixels, but it comes at the cost of performance.
  float occlusion = 0.0;
  
  for (int i = 0; i < 4; ++i)
  {
    vec2 k1 = reflect(kernel[i], randVec);
    
    vec2 k2 = vec2(k1.x * Sin45 - k1.y * Sin45,
                   k1.x * Sin45 + k1.y * Sin45);
    
    k1.x *= u_texelWidth;
    k1.y *= u_texelHeight;
    
    k2.x *= u_texelWidth;
    k2.y *= u_texelHeight;
    
    occlusion += SamplePixels(srcPosition, srcNormal, v_texCoord + k1 * kernelRadius);
    occlusion += SamplePixels(srcPosition, srcNormal, v_texCoord + k2 * kernelRadius * 0.75);
    occlusion += SamplePixels(srcPosition, srcNormal, v_texCoord + k1 * kernelRadius * 0.5);
    occlusion += SamplePixels(srcPosition, srcNormal, v_texCoord + k2 * kernelRadius * 0.25);
  }

  // Average and clamp ambient occlusion
  occlusion /= 16;
  occlusion = 1.0 - clamp(occlusion, 0.0, 1.0);

  gl_FragColor = vec4(occlusion, occlusion, occlusion, 1.0);
}
