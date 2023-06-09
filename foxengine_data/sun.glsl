input(vec3, inPosition, 0);
input(vec3, inNormal, 1);
input(vec2, inTexCoord, 2);

output(vec4, outColor, 0);
output(vec4, outBlack, 1);
output(vec4, outNormal, 2);
output(vec4, outPos, 3);

varying(vec2, vTexCoord);

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform vec3 uColor;

#ifdef FE_VERT

void main(void)
{
	mat4 viewMatNoPos = uView;
    viewMatNoPos[3][0] = 0;
    viewMatNoPos[3][1] = 0;
    viewMatNoPos[3][2] = 0;

	gl_Position = uProjection * viewMatNoPos * uModel * vec4(inPosition, 1.0);
	gl_Position.z = gl_Position.w;

	vTexCoord = inPosition.xy;

}

#elif defined FE_FRAG

void main(void)
{
	outColor = vec4(vec3(uColor), smoothstep(1.0, 0.8, length(vTexCoord)));
	outBlack = outColor;
	outNormal = vec4(0.0);
	outPos = vec4(0.0);
}

#endif