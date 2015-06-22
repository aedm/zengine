:name "2DTransform.vs"

:input vec3 aPosition
:global vec2 gRenderTargetSizeRecip

SHADER
{
    gl_Position = 
        vec4(aPosition.xy * gRenderTargetSizeRecip * vec2(2, -2) + vec2(-1, 1), 0, 1);
}
