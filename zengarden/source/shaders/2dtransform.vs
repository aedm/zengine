attribute vec3 aPosition;

uniform vec2 gRenderTargetSizeRecip;

void main()
{
	gl_Position = vec4(aPosition.xy * gRenderTargetSizeRecip * vec2(2, -2) + vec2(-1, 1), 0, 1);
}
