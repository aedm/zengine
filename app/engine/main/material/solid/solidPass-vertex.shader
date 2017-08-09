:name "Default VS"
:returns void

:input vec3 aPosition;
:input vec2 aTexCoord;
:input vec3 aNormal;
:input vec3 aTangent

:global mat4 gTransformation
:global mat4 gView
:global mat4 gSkylightTransformation

:output vec2 vTexCoord
:output vec3 vNormal
:output vec3 vTangent
:output vec3 vSkyLightCoord
:output vec3 vModelSpacePos
:output vec3 vViewSpacePos

SHADER
{
  vec4 pos = vec4(aPosition, 1.0) * gTransformation; 

  vTexCoord = aTexCoord;
  vNormal = (vec4(aNormal, 0.0) * gView).xyz;
  vTangent = (vec4(aTangent, 0.0) * gView).xyz;
  vSkyLightCoord = (vec4(aPosition, 1.0) * gSkylightTransformation).xyz; 
  vModelSpacePos = aPosition;
  vViewSpacePos = (vec4(aPosition, 1.0) * gView).xyz;

  gl_Position = pos;
}
