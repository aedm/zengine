attribute vec3 aPosition;

uniform mat4 gTransformation;

void main()
{
	gl_Position =  vec4(aPosition, 1) * gTransformation;
}
