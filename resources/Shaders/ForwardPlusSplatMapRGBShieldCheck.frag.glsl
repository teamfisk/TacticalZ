#version 430

#include "Shaders/Util/CommonNormalFunc.glsl" 

#define MIN_AMBIENT_LIGHT 0.3
#define MAX_SPLITS 4
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec2 ScreenDimensions;
uniform float FillPercentage;
uniform vec4 DiffuseColor;
uniform vec4 FillColor;
uniform vec4 Color;
uniform vec4 AmbientColor;
uniform int SSAOQuality;

//Get bineded at the same time as the textures
uniform int NormalTextureType1;
uniform int NormalTextureType2;
uniform int NormalTextureType3;

uniform vec2 DiffuseUVRepeat1;
uniform vec2 DiffuseUVRepeat2;
uniform vec2 DiffuseUVRepeat3;
uniform vec2 NormalUVRepeat1;
uniform vec2 NormalUVRepeat2;
uniform vec2 NormalUVRepeat3;
uniform vec2 SpecularUVRepeat1;
uniform vec2 SpecularUVRepeat2;
uniform vec2 SpecularUVRepeat3;
uniform vec2 GlowUVRepeat1;
uniform vec2 GlowUVRepeat2;
uniform vec2 GlowUVRepeat3;
layout (binding = 0) uniform sampler2D AOTexture;
layout (binding = 1) uniform sampler2D SplatMapTexture;
layout (binding = 2) uniform sampler2D DiffuseTexture1;
layout (binding = 3) uniform sampler2D DiffuseTexture2;
layout (binding = 4) uniform sampler2D DiffuseTexture3;
layout (binding = 5) uniform sampler2D NormalMapTexture1;
layout (binding = 6) uniform sampler2D NormalMapTexture2;
layout (binding = 7) uniform sampler2D NormalMapTexture3;
layout (binding = 8) uniform sampler2D SpecularMapTexture1;
layout (binding = 9) uniform sampler2D SpecularMapTexture2;
layout (binding = 10) uniform sampler2D SpecularMapTexture3;
layout (binding = 11) uniform sampler2D GlowMapTexture1;
layout (binding = 12) uniform sampler2D GlowMapTexture2;
layout (binding = 13) uniform sampler2D GlowMapTexture3;
layout (binding = 31) uniform samplerCube ShieldBuffer;

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
	vec4 ViewSpacePosition;
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

float CalcAttenuation(float radius, float dist, float falloff) {
	return 1.0 - smoothstep(radius * 0.3, radius, dist);
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

vec4 CalcNormalMappedValue(vec3 normal, vec3 tangent, vec3 bitangent, vec2 textureCoordinate, sampler2D normalMap)
{
	mat3 TBN = mat3(tangent, bitangent, normal);
	vec3 NormalMap = texture(normalMap, textureCoordinate).xyz * 2.0 - vec3(1.0);
	return vec4(TBN * normalize(NormalMap), 0.0);
}

vec4 CalcBlendedTexel(vec4 blendValue, sampler2D R, sampler2D G, sampler2D B,
	 				  vec2 R_TileValues,  vec2 G_TileValues,  vec2 B_TileValues){
	vec4 R_Channel = texture2D(R, Input.TextureCoordinate * R_TileValues);
	vec4 G_Channel = texture2D(G, Input.TextureCoordinate * G_TileValues);
	vec4 B_Channel = texture2D(B, Input.TextureCoordinate * B_TileValues);

	float total = blendValue.r + blendValue.g + blendValue.b;
	float totalDiv = 1.0f / total;
	blendValue.r = blendValue.r * totalDiv;
	blendValue.g = blendValue.g * totalDiv;
	blendValue.b = blendValue.b * totalDiv;

	return blendValue.r * R_Channel
		 + blendValue.g * G_Channel
		 + blendValue.b * B_Channel;
}

vec4 CalcBlendedNormal(vec4 blendValue, sampler2D R, sampler2D G, sampler2D B,
					   vec2 R_TileValues,  vec2 G_TileValues,  vec2 B_TileValues
					   int R_TextureType, int G_TextureType, int B_TextureType) {
	mat3 TBN = mat3(Input.Tangent, Input.BiTangent, Input.Normal);
	vec3 R_Channel = NormalMapValue(Input.TextureCoordinate * R_TileValues, R, R_TextureType);
	vec3 G_Channel = NormalMapValue(Input.TextureCoordinate * G_TileValues, G, G_TextureType);
	vec3 B_Channel = NormalMapValue(Input.TextureCoordinate * B_TileValues, B, B_TextureType);

	float total = blendValue.r + blendValue.g + blendValue.b + blendValue.a;
	float totalDiv = 1 / total;
	blendValue.r = blendValue.r * totalDiv;
	blendValue.g = blendValue.g * totalDiv;
	blendValue.b = blendValue.b * totalDiv;

	vec3 Normal_result = blendValue.r * R_Channel
					   + blendValue.g * G_Channel 
		 			   + blendValue.b * B_Channel;

	return vec4(TBN * normalize(Normal_result), 0.0);
}

void main()
{
	float ao = texelFetch(AOTexture, ivec2(gl_FragCoord.xy) >> int(SSAOQuality), 0).r;
	ao = (clamp(1.0 - (1.0 - ao), 0.0, 1.0) + MIN_AMBIENT_LIGHT) /  (1.0 + MIN_AMBIENT_LIGHT);

	vec4 splatTexel = texture2D(SplatMapTexture, Input.TextureCoordinate);

	vec4 diffuseTexel = CalcBlendedTexel(splatTexel, DiffuseTexture1, DiffuseTexture2, DiffuseTexture3,
	 									 DiffuseUVRepeat1, DiffuseUVRepeat2, DiffuseUVRepeat3);
	vec4 glowTexel = CalcBlendedTexel(splatTexel, GlowMapTexture1, GlowMapTexture2, GlowMapTexture3,
	 									 GlowUVRepeat1, GlowUVRepeat2, GlowUVRepeat3);
	vec4 specularTexel = CalcBlendedTexel(splatTexel, SpecularMapTexture1, SpecularMapTexture2, SpecularMapTexture3,
										 SpecularUVRepeat1, SpecularUVRepeat2, SpecularUVRepeat3);
	//vec4 normal = V * CalcNormalMappedValue(Input.Normal, Input.Tangent, Input.BiTangent, Input.TextureCoordinate, SplatMapTexture);
	vec4 normal = V * CalcBlendedNormal(splatTexel, NormalMapTexture1, NormalMapTexture2, NormalMapTexture3,
		   								NormalUVRepeat1, NormalUVRepeat2, NormalUVRepeat3,
		   								NormalTextureType1, NormalTextureType2, NormalTextureType3);
	normal = normalize(normal);
	//vec4 normal = normalize(V  * vec4(Input.Normal, 0.0));
	vec4 viewVec = normalize(-Input.ViewSpacePosition); 

	vec2 tilePos;
	tilePos.x = int(gl_FragCoord.x/TILE_SIZE);
	tilePos.y = int(gl_FragCoord.y/TILE_SIZE);

	LightResult totalLighting;
	totalLighting.Diffuse = vec4(AmbientColor.rgb * ao, 1.0);
	int currentTile = int(floor(gl_FragCoord.x/TILE_SIZE) + (floor(gl_FragCoord.y/TILE_SIZE) * int(ScreenDimensions.x/TILE_SIZE)));

	int start = int(LightGrids.Data[currentTile].Start);
	int amount = int(LightGrids.Data[currentTile].Amount);

	for(int i = start; i < start + amount; i++) {

		int l = int(LightIndex[i]);
		LightSource light = LightSources.List[l];

		LightResult light_result;
		//These if statements should be removed.
		if(light.Type == 1) { // point
			light_result = CalcPointLightSource(V * light.Position, light.Radius, light.Color, light.Intensity, viewVec, Input.ViewSpacePosition, normal, light.Falloff);
		} else if (light.Type == 2) { //Directional
			light_result = CalcDirectionalLightSource(V * light.Direction, light.Color, light.Intensity, viewVec, normal);
		}
		totalLighting.Diffuse += vec4(light_result.Diffuse.rgb * ao, light_result.Diffuse.a);
		totalLighting.Specular += vec4(light_result.Specular.rgb * ao, light_result.Specular.a);
	}

	vec4 color_result = mix((Color * diffuseTexel * DiffuseColor), Input.ExplosionColor, Input.ExplosionPercentageElapsed);
	color_result = color_result * (totalLighting.Diffuse + (totalLighting.Specular * specularTexel));
	//vec4 color_result = (DiffuseColor + Input.ExplosionColor) * (totalLighting.Diffuse + (totalLighting.Specular * specularTexel)) * diffuseTexel * Color;
	

	float pos = ((P * vec4(Input.Position, 1)).y + 1.0)/2.0;

	if(pos <= FillPercentage) {
		color_result += FillColor;
	}
	sceneColor = vec4(color_result.xyz, clamp(color_result.a, 0, 1));
	color_result += glowTexel*3;

	bloomColor = vec4(clamp(color_result.xyz - 1.0, 0, 100), 1.0);

	//Tiled Debug Code
	/*
	if(int(gl_FragCoord.x)%16 == 0 || int(gl_FragCoord.y)%16 == 0 ) {
		sceneColor += vec4(0.5, 0, 0, 0);
	} else {
		sceneColor += vec4(LightGrids.Data[int(tilePos.x + tilePos.y*80)].Amount/LightSources.List.length(), 0, 0, 1);
	}
	*/
}


