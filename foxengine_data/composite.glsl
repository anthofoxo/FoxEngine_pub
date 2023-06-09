input(vec3, inPosition, 0);
input(vec2, inTexCoord, 1);

output(vec4, outColor, 0);

varying(vec2, vTexCoord);

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uPosition;
uniform sampler2D uRadial;
uniform vec3 uLightColor;
uniform vec3 uLightDirection;

#ifdef FE_VERT

void main(void)
{
	gl_Position = vec4(inPosition, 1.0);
	vTexCoord = inPosition.xy * 0.5 + 0.5;
}

#elif defined FE_FRAG

// ACES fitted
// from https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

const mat3 ACESInputMat = mat3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mat3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = color * ACESInputMat;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = color * ACESOutputMat;

    // Clamp to [0, 1]
    color = clamp(color, 0.0, 1.0);

    return color;
}

void main(void)
{
	vec4 albedo = texture(uAlbedo, vTexCoord);
	vec4 normal = texture(uNormal, vTexCoord);
	normal.rgb = normalize(normal.rgb * 2.0 - 1.0);
	vec4 position = texture(uPosition, vTexCoord);
	vec4 radial = texture(uRadial, vTexCoord);

	

	vec3 toCamera = normalize((inverse(uView) * vec4(0.0, 0.0, 0.0, 1.0)).xyz - position.xyz);

	vec3 lightDir = normalize(uLightDirection);

	float diffuse = dot(normal.xyz, -lightDir);
	float specular = dot(reflect(lightDir, normal.xyz), toCamera);

	outColor = albedo;

	if(normal.a > 0.5)
	{
		outColor.rgb *= max(diffuse * uLightColor, 0.1);
		outColor.rgb += pow(max(specular, 0.0), 10.0) * uLightColor;
	}

	// Apply fog
	outColor = mix(vec4(0.75, 0.85, 0.95, 1.0), outColor, albedo.a);

	// Tonemap this garbage
	// https://www.shadertoy.com/view/ldcSRN
	// https://www.shadertoy.com/view/fsXcz4
	// https://www.shadertoy.com/view/4d3SR4

	outColor.rgb += radial.rgb * 1.5;
	outColor.rgb = ACESFitted(outColor.rgb);
	outColor.a = 1.0;
	
}

#endif