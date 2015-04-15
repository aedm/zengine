
attribute vec3 aPosition;

uniform float gTime;

varying vec4 vColor; 
varying vec3 vNormal;
varying vec3 vEyeToTarget;

vec3 silk(vec3 pos)
{
	vec3 a = cos(pos * 2.0 - gTime*0.2)*0.3;
	vec3 b = vec3(a.y + sin(pos.y*8.0)*0.03, a.x+ cos(a.x*9.0)*0.4, a.y);
	vec3 c = vec3(b.x + cos(b.x*18.0)*0.16, b.x*2.0 + cos(a.x*11.0+b.x*13.0)*0.28, -pos.y + b.y*0.2 - a.y*0.1);
	return (pos + c*0.7) * vec3(1,0.7,1); 
}

void main()
{
	vec3 t = -1. + 2. * aPosition;
	float d = 0.01;
	vec3 p = silk(t);
	vec3 nx = silk(t+vec3(d,0,0)) - p;
	vec3 ny = silk(t+vec3(0,d,0)) - p;
	vNormal = cross(normalize(nx), normalize(ny));
	//vNormal = vec3(p.z+1);
	gl_Position = vec4(p.x*1.9, -p.y*0.7, p.z*0.2, 1);
	vEyeToTarget = normalize(gl_Position.xyz);
}
