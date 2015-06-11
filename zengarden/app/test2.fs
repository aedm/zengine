:name "test"
:returns void

:output vec4 Color
:param float Green

SHADER
{
	Color = vec4(0.0, Green, 0.0, 1.0);
}
