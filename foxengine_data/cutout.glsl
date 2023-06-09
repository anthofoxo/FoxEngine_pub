@pragma backface_nocull;

input(vec3, inPosition, 0);
input(vec3, inNormal, 1);
input(vec2, inTexCoord, 2);

output(vec4, outColor, 0);
output(vec4, outBlack, 1);
output(vec4, outNormal, 2);
output(vec4, outPos, 3);

varying(vec2, vTexCoord);
varying(vec3, vNormal);
varying(vec3, vPos);

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform sampler2D uSampler;

#ifdef FE_VERT

void main(void)
{
	vec4 worldSpace = uModel * vec4(inPosition, 1.0);
	gl_Position = uProjection * uView * worldSpace;
	vNormal = transpose(inverse(mat3(uModel))) * inNormal;
	vTexCoord = inTexCoord;
	vPos = worldSpace.xyz;
}

#elif defined FE_FRAG

void main(void)
{
	outColor = texture(uSampler, vTexCoord);
	if (outColor.a < 0.5) discard;
	outColor.a = 1.0;

	vec3 surfaceNormal = normalize(vNormal);
	if(!gl_FrontFacing) surfaceNormal *= -1.0;

	outBlack = vec4(0.0, 0.0, 0.0, 1.0);
	outNormal = vec4(surfaceNormal * 0.5 + 0.5, 1.0);
	outPos = vec4(vPos, 0.0);
}

#endif