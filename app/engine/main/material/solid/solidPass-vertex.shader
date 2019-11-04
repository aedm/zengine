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
  vec4 pos = gTransformation * vec4(aPosition, 1.0); 

  vTexCoord = aTexCoord;
  vNormal = (gView * vec4(aNormal, 0.0)).xyz;
  vTangent = (gView * vec4(aTangent, 0.0)).xyz;
  vSkyLightCoord = (gSkylightTransformation * vec4(aPosition, 1.0)).xyz; 
  vModelSpacePos = aPosition;
  vViewSpacePos = (gView * vec4(aPosition, 1.0)).xyz;

  gl_Position = pos;
}
