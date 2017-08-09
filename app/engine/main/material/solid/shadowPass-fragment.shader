:name "Shadow map FS"
:returns void

:output vec4 Color

:input vec2 vTexCoord
:input float vDepth

SHADER
{
  Color = vec4(vTexCoord, vDepth, 1);
}
