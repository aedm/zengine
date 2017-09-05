:name "Default FS"
:returns void

:output vec4 FragColor
:input vec2 vTexCoord
:input vec3 vNormal
:input vec3 vTangent
:input vec3 vSkyLightCoord
:input vec3 vModelSpacePos
:input vec3 vViewSpacePos

:param vec3 Color
:param float Specular
:param float SpecularPower min=0 max=50 def=5
:param vec3 Emissive
:param sampler2D Texture
:param sampler2D NormalMap
:param float NormalMapIntensity
:param vec3 TextureScale

:global vec3 gSkylightDirection
:global vec3 gSkylightColor
:global float gSkylightAmbient
:global float gSkylightSpread
:global float gSkylightSampleSpread
:global mat4 gCamera
:global mat4 gView

SHADER
{
  vec3 lightDir = (vec4(normalize(gSkylightDirection), 0) * gCamera).xyz;
  vec2 texCoord = vTexCoord * TextureScale.xy;
   
  #if defined NormalMap_CONNECTED
    vec3 normal = ApplyNormalMap(vNormal, vTangent, NormalMap, texCoord, NormalMapIntensity);
  #else 
    vec3 normal = normalize(vNormal);
  #endif

  float light = CalculateLight(Specular, SpecularPower, normal, lightDir, vViewSpacePos);
  float shadow = CalculateSoftShadow(vSkyLightCoord, gSkylightSampleSpread, gSkylightSpread); 

  #ifdef Texture_CONNECTED
    vec3 color = TexturedMaterial(Color, gSkylightColor, Emissive, gSkylightAmbient, light, shadow, 
	Texture, texCoord); 
  #else 
    vec3 color = SolidMaterial(Color, gSkylightColor, Emissive, gSkylightAmbient, light, shadow); 
  #endif  

  FragColor = vec4(color, 1);
}
