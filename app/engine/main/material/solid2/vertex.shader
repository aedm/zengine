:name "Default VS"
:returns void

:input vec3 aPosition;
:input vec2 aTexCoord;
:input vec3 aNormal;
:input vec3 aTangent

:global mat4 gTransformation
:global mat4 gProjection
:global mat4 gView
:global mat4 gWorld

:global mat4 gSkylightTransformation
:global mat4 gSkylightCamera
:global mat4 gSkylightProjection

:output vec2 vTexCoord
:output vec3 vNormal
:output vec3 vTangent
:output vec3 vModelSpacePos
:output vec3 vViewSpacePos

:output vec3 vSkyLightCoord
:output vec3 vSkyLightNormal
:output vec3 vSkyLightTangent
:output vec3 vSkyLightBinormal

:param float Row
:param float Spacing
:param float Time
:param vec4 A
:param vec4 B
:param vec4 C

float displace(vec2 xy, vec4 b, float time) {
  float dist = length(xy);
  return sin(dist * b.y + time * b.z + b.w) * b.x;
}

SHADER 
{
  int row = int(Row);
  int zc = gl_InstanceID / row;
  int xc = gl_InstanceID - zc * row - (row - 1) / 2;
  zc -= (row-1) / 2;

  vec3 move = vec3(float(xc), 0.0, float(zc)) * Spacing;
  float dist = length(move.xz);
  move.y += sin(dist * A.x + Time * A.y + A.w + C.x * (gl_InstanceID)) * A.z;
  vec3 pos = aPosition + move;

  float d = displace(pos.xz, B, Time);
  pos.y += d;

  float eps = 0.01;
  float dx = displace(vec2(pos.x + eps, pos.z), B, Time) - d;
  float dz = displace(vec2(pos.x, pos.z + eps), B, Time) - d;
  vec3 norm2 = normalize(vec3(-dx, C.y, -dz));

  vec3 normal = mix(aNormal, norm2, aNormal.y);

  vec3 binormalOrig = cross(aNormal, aTangent);
  vec3 tangent = cross(binormalOrig, normal);
  vec3 binormal = cross(normal, tangent);

  vTexCoord = aTexCoord;
  vModelSpacePos = pos;

  gl_Position = vec4(pos, 1.0) * gTransformation;
  vNormal = (vec4(normal, 0.0) * gView).xyz;
  vTangent = (vec4(tangent, 0.0) * gView).xyz;
  vViewSpacePos = (vec4(pos, 1.0) * gView).xyz; 

  mat4 skyView = gSkylightCamera * gWorld;

  vSkyLightCoord = (vec4(pos, 1.0) * gSkylightTransformation).xyz; 
  vSkyLightNormal = (vec4(normal, 0.0) * skyView).xyz; 
  vSkyLightTangent = (vec4(tangent, 0.0) * skyView).xyz; 
  vSkyLightBinormal = (vec4(binormal, 0.0) * skyView).xyz; 
}
