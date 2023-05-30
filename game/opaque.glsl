input(vec3, inPosition, 0);
input(vec3, inNormal, 1);
output(vec4, outColor, 0);

varying(vec3, vNormal);

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

#ifdef FE_VERT

void main(void)
{
	gl_Position = uProjection * uView * uModel * vec4(inPosition, 1.0);
	vNormal = inNormal;
}

#elif defined FE_FRAG

void main(void)
{
	outColor = vec4(vec3(1.0, 0.5, 0.0) * max(dot(normalize(vNormal), vec3(0.0, 0.0, 1.0)), 0.1), 1.0);
}

#endif