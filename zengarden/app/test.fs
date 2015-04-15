
varying vec3 vNormal;
varying vec3 vEyeToTarget;

uniform float gTime;

uniform float uColorFactor;
//! ui="Shine" min=0 max=5 def=1 scale=linear 

float noise(vec3 p) //Thx to Las^Mercury
{
	vec3 i = floor(p);
	vec4 a = dot(i, vec3(1., 57., 21.)) + vec4(0., 57., 21., 78.);
	vec3 f = cos((p-i)*acos(-1.))*(-.5)+.5;
	a = mix(sin(cos(a)*a),sin(cos(1.+a)*(1.+a)), f.x);
	a.xy = mix(a.xz, a.yw, f.y);
	return mix(a.x, a.y, f.z);
}


void main()
{
	//float angle = dot(normalize(vEyeToTarget), vNormal);
	float angle = abs(dot(normalize(vec3(0,1,0.0)), vNormal));
	float v = pow(angle, 04.5);
	float c = max(0.0, pow(v, 2.));
	float c2 = max(0.0, pow(vNormal.z+0.008, 4.));
	vec3 f1 = mix(vec3(0.00,0.00,0.00), vec3(0.7,0.2,0.5) * uColorFactor, c);
	//vec3 f1 = mix(vec3(0.00,0.00,0.00), vec3(0.7,0.2,0.5) * 2.0, c);
	vec3 f2 = mix(vec3(0.0,0.0,0.0), vec3(0.5,0.5,0.5)*0.2, c2);
	gl_FragColor = vec4(f1 + f2, 1);
	//gl_FragColor = vec4(f1, 1);
}
