:name "test"
:returns float

:param vec4 Color
:param sampler2d Texture
:param float Ratio

:input vec3 Normal
:input vec3 EyeToTarget
:output vec3 Color

SHADER
{
	outColor = Color * inNormal;
}
