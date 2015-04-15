attribute vec3 aPosition;
attribute vec2 aTexCoord;

uniform mat4 gTransformation;

varying vec2 vTexCoord;

void main()
{
	gl_Position = vec4(aPosition, 1) * gTransformation;
	vTexCoord = aTexCoord;
}
