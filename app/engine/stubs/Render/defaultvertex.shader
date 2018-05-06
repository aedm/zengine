:name "Default VS"
:returns void

:input vec3 aPosition;
:input vec2 aTexCoord;
:input vec3 aNormal;

:global mat4 gTransformation;
:global mat4 gView;

:output vec2 vTexCoord
:output vec3 vNormal
:output vec3 vPosition

SHADER
{
  gl_Position = vec4(aPosition, 1.0) * gTransformation; 
  vPosition = (vec4(aPosition, 1.0) * gView).xyz;
  vNormal = (vec4(aNormal, 0.0) * gView).xyz;
  vTexCoord = aTexCoord;
}
