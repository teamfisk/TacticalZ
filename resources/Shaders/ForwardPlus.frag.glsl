#version 430

#include "Shaders/Util/CommonNormalFunc.glsl" 

#define MIN_AMBIENT_LIGHT 0.3
#define MAX_SPLITS 4


uniform mat4 VM;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;
uniform vec4 DiffuseColor;
uniform vec2 ScreenDimensions;
uniform vec4 FillColor;
uniform vec4 AmbientColor;
uniform float FillPercentage;
uniform float GlowIntensity;
uniform vec3 CameraPosition;
uniform int SSAOQuality;
uniform float FarDistance[MAX_SPLITS];

uniform int NormalTextureType;
uniform vec2 DiffuseUVRepeat;
uniform vec2 NormalUVRepeat;
uniform vec2 SpecularUVRepeat;
uniform vec2 GlowUVRepeat;
layout (binding = 0) uniform sampler2D AOTexture;
layout (binding = 1) uniform sampler2D DiffuseTexture;
layout (binding = 2) uniform sampler2D NormalMapTexture;
layout (binding = 3) uniform sampler2D SpecularMapTexture;
layout (binding = 4) uniform sampler2D GlowMapTexture;
layout (binding = 5) uniform samplerCube CubeMap;
layout (binding = 30) uniform sampler2DArrayShadow DepthMap;

#define TILE_SIZE 16

struct LightSource {
	vec4 Position;
	vec4 Direction;
	vec4 Color;
	float Radius;
	float Intensity;
	float Falloff;
	int Type;
};

layout (std430, binding = 1) buffer LightBuffer
{
	LightSource List[];
} LightSources;

struct LightGrid {
	float Start;
	float Amount;
	vec2 Padding;
};

layout (std430, binding = 2) buffer LightGridBuffer
{
	LightGrid Data[];
} LightGrids;

layout (std430, binding = 4) buffer LightIndexBuffer
{
	float LightIndex[];
};

in VertexData{
	vec3 Position;
	vec3 Normal;
	vec3 Tangent;
	vec3 BiTangent;
	vec2 TextureCoordinate;
	vec4 ExplosionColor;
	float ExplosionPercentageElapsed;
	vec4 PositionLightSpace[MAX_SPLITS];
}Input;

out vec4 sceneColor;
out vec4 bloomColor;

struct LightResult {
	vec4 Diffuse;
	vec4 Specular;
};

vec2 poissonDisk[16] = vec2[]( 
	vec2( -0.94201624, -0.39906216 ), 
	vec2( 0.94558609, -0.76890725 ), 
	vec2( -0.094184101, -0.92938870 ), 
	vec2( 0.34495938, 0.29387760 ), 
	vec2( -0.91588581, 0.45771432 ), 
	vec2( -0.81544232, -0.87912464 ), 
	vec2( -0.38277543, 0.27676845 ), 
	vec2( 0.97484398, 0.75648379 ), 
	vec2( 0.44323325, -0.97511554 ), 
	vec2( 0.53742981, -0.47373420 ), 
	vec2( -0.26496911, -0.41893023 ), 
	vec2( 0.79197514, 0.19090188 ), 
	vec2( -0.24188840, 0.99706507 ), 
	vec2( -0.81409955, 0.91437590 ), 
	vec2( 0.19984126, 0.78641367 ), 
	vec2( 0.14383161, -0.14100790 ) 
	);

float CalcAttenuation(float radius, float dist, float falloff) {
	return 1.0 - smoothstep(radius * falloff, radius, dist);
}

vec4 CalcSpecular(vec4 lightColor, vec4 viewVec,  vec4 lightVec, vec4 normal) {
	vec4 R = normalize( reflect(-lightVec, normal));
	float RdotV = max( dot(R, viewVec), 0.0);
	return lightColor * pow(RdotV, 90.0);
}

vec4 CalcDiffuse(vec4 lightColor, vec4 lightVec, vec4 normal) {
	float power = max( dot(normal, lightVec), 0.0);
	return lightColor * power;
}

LightResult CalcPointLightSource(vec4 lightPos, float lightRadius, vec4 lightColor, float intensity, vec4 viewVec, vec4 position, vec4 normal, float falloff)
{
	vec4 L = lightPos - position;
	float dist = length(L);
	L = normalize(L);

	float attenuation = CalcAttenuation(lightRadius, dist, falloff);

	LightResult result;
	result.Diffuse = CalcDiffuse(lightColor, L, normal) * attenuation * intensity;
	result.Specular = CalcSpecular(lightColor, viewVec, L, normal) * attenuation *  intensity;
	return result;
}

LightResult CalcDirectionalLightSource(vec4 direction, vec4 color, float intensity, vec4 viewVec, vec4 vertNormal)
{
	vec4 L = normalize( -vec4(direction.xyz, 0) );

	LightResult result;
	result.Diffuse = CalcDiffuse(color, L, vertNormal) * intensity;
	result.Specular = CalcSpecular(color, viewVec, L, vertNormal) * intensity;
	return result;
}

// Returns a "random" value.
float Random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

int getShadowIndex(float far_distance[1])
{
	return 0;
}

int getShadowIndex(float far_distance[2])
{
	float depth = gl_FragCoord.z / gl_FragCoord.w;
	
	int index = 1;
	if ( depth < far_distance[0] )
	{
		index = 0;
	}

	return index;
}

int getShadowIndex(float far_distance[3])
{
	float depth = gl_FragCoord.z / gl_FragCoord.w;
	
	int index = 2;
	if ( depth < far_distance[0] )
	{
		index = 0;
	}
	else if ( depth < far_distance[1] && depth > far_distance[0] )
	{
		index = 1;
	}

	return index;
}

int getShadowIndex(float far_distance[4])
{
	float depth = gl_FragCoord.z / gl_FragCoord.w;
	
	int index = 3;
	if ( depth < far_distance[0] )
	{
		index = 0;
	}
	else if ( depth < far_distance[1] && depth > far_distance[0] )
	{
		index = 1;
	}
	else if ( depth < far_distance[2] && depth > far_distance[1] )
	{
		index = 2;
	}

	return index;
}

// Standard hardware-calculated PCF method
float PCFShadow(sampler2DArrayShadow depth_texture_array, vec3 projection_coords, int layer_index)
{
	return texture(depth_texture_array, vec4(projection_coords.xy, layer_index, projection_coords.z));
}

// PCF + Poisson model method
float PoissonShadow(sampler2DArrayShadow depth_texture_array, vec3 projection_coords, int layer_index, int taps, float spread)
{
	int loop;
	float multiplier = 1.0 / float(taps);
	float shadowMapDepth;
	
    for (int i = 0; i < taps; i++)
	{
		loop = i;
		vec3 newProjCoords = projection_coords + vec3(poissonDisk[loop], 0.0) / (spread * (1.0 + layer_index));
		shadowMapDepth += multiplier * texture(depth_texture_array, vec4(newProjCoords.xy, layer_index, newProjCoords.z));
	}
	
	return shadowMapDepth;
}

// PCF + Poisson + RandomSample model method
float PoissonDotShadow(sampler2DArrayShadow depth_texture_array, vec3 projection_coords, int layer_index, int taps, float spread)
{
	int loop;
	float multiplier = 1.0 / float(taps);
	float shadowMapDepth;
	
    for (int i = 0; i < taps; i++)
	{
		loop = int(16.0 * Random(gl_FragCoord.xyy, i)) % 16;
		vec3 newProjCoords = projection_coords + vec3(poissonDisk[loop], 0.0) / (spread * (1.0 + layer_index));
		shadowMapDepth += multiplier * texture(depth_texture_array, vec4(newProjCoords.xy, layer_index, newProjCoords.z));
	}
	
	return shadowMapDepth;
}

// Hardware PCF + Additional software PCF method
float SoftwarePCF(sampler2DArrayShadow depth_texture_array, vec3 projection_coords, int layer_index, float bias)
{
	float shadow = 0.0;
	
	vec3 texelSize = 1.0 / textureSize(depth_texture_array, 0);
	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			shadow += texture(depth_texture_array, vec4(projection_coords.xy + vec2(x, y) * texelSize.xy / (1.0 + layer_index), layer_index, projection_coords.z)); 
		}    
	}
	
	return shadow / 9.0;
}

float CalcShadowValue(vec4 light_space_pos, vec3 normal, vec3 light_dir, sampler2DArrayShadow depth_texture_array, int layer_index)
{
	float shadowMapDepth;
	float bias = 0.005;
	
	// Various bias methods.
	
	//bias = max(0.05 * (1.0 - dot(normal, light_dir)), bias);
	//bias = bias * tan(acos(clamp(dot(normal, -light_dir), 0.0, 1.0)));
	bias = bias + bias * tan(acos(clamp(dot(normal, -light_dir), 0.0, 1.0)));
	
	// Calculate coordinates in projection space.
	
    vec3 projCoords = vec3(light_space_pos.xy, light_space_pos.z + bias) / light_space_pos.w;
    projCoords = projCoords * 0.5 + 0.5;
	//projCoords = (floor(projCoords * 255.0)) / 255.0;
	
	// Various methods for shadow calculation in fastest to slowest order.
	
	//shadowMapDepth = PCFShadow(depth_texture_array, projCoords, layer_index);
	//shadowMapDepth = PoissonShadow(depth_texture_array, projCoords, layer_index, 4, 25.0 * FarDistance[MAX_SPLITS - 1]);
	//shadowMapDepth = PoissonDotShadow(depth_texture_array, projCoords, layer_index, 4, 25.0 * FarDistance[MAX_SPLITS - 1]);
	shadowMapDepth = SoftwarePCF(depth_texture_array, projCoords, layer_index, bias);

    return shadowMapDepth;
}

void main()
{
	float ao = texelFetch(AOTexture, ivec2(gl_FragCoord.xy) >> int(SSAOQuality), 0).r;
	ao = (clamp(1.0 - (1.0 - ao), 0.0, 1.0) + MIN_AMBIENT_LIGHT) /  (1.0 + MIN_AMBIENT_LIGHT);
	vec4 diffuseTexel = texture2D(DiffuseTexture, Input.TextureCoordinate * DiffuseUVRepeat);
	vec4 glowTexel = texture2D(GlowMapTexture, Input.TextureCoordinate * GlowUVRepeat);
	vec4 specularTexel = texture2D(SpecularMapTexture, Input.TextureCoordinate * SpecularUVRepeat);
	vec4 position = VM * vec4(Input.Position, 1.0); 
	vec4 normal = V * CalcNormalMappedValue(Input.Normal, Input.Tangent, Input.BiTangent, Input.TextureCoordinate * NormalUVRepeat, NormalMapTexture, NormalTextureType);
	normal = normalize(normal);
	//vec4 normal = normalize(V  * vec4(Input.Normal, 0.0));
	vec4 viewVec = normalize(-position);
	vec3 I = normalize(vec3(M * vec4(Input.Position, 1.0)) - CameraPosition);
	vec3 R = reflect(-I, Input.Normal);
	//R = vec3(P * vec4(R, 1.0));
	vec4 reflectionColor = texture(CubeMap, R);

	vec2 tilePos;
	tilePos.x = int(gl_FragCoord.x/TILE_SIZE);
	tilePos.y = int(gl_FragCoord.y/TILE_SIZE);

	LightResult totalLighting;
	totalLighting.Diffuse = vec4(AmbientColor.rgb * ao, 1.0);
	int currentTile = int(floor(gl_FragCoord.x/TILE_SIZE) + (floor(gl_FragCoord.y/TILE_SIZE) * int(ScreenDimensions.x/TILE_SIZE)));

	int start = int(LightGrids.Data[currentTile].Start);
	int amount = int(LightGrids.Data[currentTile].Amount);
	
	float shadowFactor = 0.0;

	for(int i = start; i < start + amount; i++) {

		int l = int(LightIndex[i]);
		LightSource light = LightSources.List[l];

		LightResult light_result;
		//These if statements should be removed.
		if(light.Type == 1) { // point
			light_result = CalcPointLightSource(V * light.Position, light.Radius, light.Color, light.Intensity, viewVec, position, normal, light.Falloff);
		} else if (light.Type == 2) { //Directional
			int DepthMapIndex = getShadowIndex(FarDistance);
			light_result = CalcDirectionalLightSource(V * light.Direction, light.Color, light.Intensity, viewVec, normal);
			shadowFactor = CalcShadowValue(Input.PositionLightSpace[DepthMapIndex], Input.Normal, vec3(light.Direction), DepthMap, DepthMapIndex);
		}
		totalLighting.Diffuse += vec4(light_result.Diffuse.rgb * ao, light_result.Diffuse.a);
		totalLighting.Specular += vec4(light_result.Specular.rgb * ao, light_result.Specular.a);
	}
	
	totalLighting.Diffuse *= vec4(min(vec3(shadowFactor) + AmbientColor.xyz, vec3(1.0)), 1.0);
	totalLighting.Specular *= vec4(min(vec3(shadowFactor) + AmbientColor.xyz, vec3(1.0)), 1.0);

	vec4 color_result = mix((Color * diffuseTexel * DiffuseColor), Input.ExplosionColor, Input.ExplosionPercentageElapsed);
	color_result = color_result * (totalLighting.Diffuse + (totalLighting.Specular * specularTexel));
	float specularResult = (specularTexel.r + specularTexel.g + specularTexel.b)/3.0;
	vec4 reflectionTotal = reflectionColor * (1-specularTexel.a) * color_result.a * (totalLighting.Diffuse + (totalLighting.Specular * specularTexel));
	color_result = color_result * clamp(1/specularTexel.a, 0, 1) + reflectionTotal;
	//vec4 color_result = (DiffuseColor + Input.ExplosionColor) * (totalLighting.Diffuse + (totalLighting.Specular * specularTexel)) * diffuseTexel * Color;
	

	float pos = ((P * vec4(Input.Position, 1)).y + 1.0)/2.0;

	if(pos <= FillPercentage) {
		color_result += FillColor;
	}
	sceneColor = vec4(color_result.xyz, clamp(color_result.a, 0, 1));
	//sceneColor = CommonUniforms.testColour;
	//sceneColor = vec4(reflectionColor.xyz, 1);
	color_result.xyz += glowTexel.xyz*GlowIntensity;

	bloomColor = vec4(max(color_result.xyz - 1.0, 0.0), clamp(color_result.a, 0, 1));

	//Tiled Debug Code
	/*
	if(int(gl_FragCoord.x)%16 == 0 || int(gl_FragCoord.y)%16 == 0 ) {
		sceneColor += vec4(0.5, 0, 0, 0);
	} else {
		sceneColor += vec4(LightGrids.Data[int(tilePos.x + tilePos.y*80)].Amount/LightSources.List.length(), 0, 0, 1);
	}
	*/
}


