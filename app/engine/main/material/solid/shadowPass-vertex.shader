:name "Default VS"
:returns void

:input vec3 aPosition;
:global mat4 gTransformation;
:global float gSkylightBias

SHADER
{
   vec4 pos = vec4(aPosition, 1.0) * gTransformation; 
   pos.z += gSkylightBias * 0.01;
   gl_Position = pos;
}
