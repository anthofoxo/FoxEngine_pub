input(vec3, inPosition, 0);
input(vec3, inNormal, 1);
input(vec2, inTexCoord, 2);

output(vec4, outColor, 0);

varying(vec2, vTexCoord);
varying(vec3, vNormal);

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform sampler2D uSampler;

#ifdef FE_VERT

void main(void)
{
	gl_Position = uProjection * uView * uModel * vec4(inPosition, 1.0);
	vNormal = transpose(inverse(mat3(uModel))) * inNormal;
	vTexCoord = inTexCoord;
}

#elif defined FE_FRAG

void main(void)
{
	outColor = texture(uSampler, vTexCoord);
	outColor.a = 1.0;
	outColor.rgb *= max(dot(normalize(vNormal), vec3(0.0, 0.0, 1.0)), 0.1);
}

#endif