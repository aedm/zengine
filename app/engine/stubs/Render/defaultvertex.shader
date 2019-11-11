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
  gl_Position = gTransformation * vec4(aPosition, 1.0); 
  vPosition = (gView * vec4(aPosition, 1.0)).xyz;
  vNormal = (gView * vec4(aNormal, 0.0)).xyz;
  vTexCoord = aTexCoord;
}
