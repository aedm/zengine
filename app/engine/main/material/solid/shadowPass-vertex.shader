:name "Default VS"
:returns void

:input vec3 aPosition;
:global mat4 gTransformation;
:global float gSkylightBias

SHADER
{
   vec4 pos = gTransformation * vec4(aPosition, 1.0); 
   pos.z += gSkylightBias * 0.01;
   gl_Position = pos;
}
