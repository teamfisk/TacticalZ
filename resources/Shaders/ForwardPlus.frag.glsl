#version 430

#define MIN_AMBIENT_LIGHT 0.3

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;
uniform vec4 DiffuseColor;
uniform vec2 ScreenDimensions;
uniform vec4 FillColor;
uniform vec4 AmbientColor;
uniform float FillPercentage;
uniform float GlowIntensity = 10;
uniform vec3 CameraPosition;

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
}Input;

out vec4 sceneColor;
out vec4 bloomColor;

struct LightResult {
	vec4 Diffuse;
	vec4 Specular;
};

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

vec4 CalcNormalMappedValue(vec3 normal, vec3 tangent, vec3 bitangent, vec2 textureCoordinate, sampler2D normalMap)
{
	mat3 TBN = mat3(tangent, bitangent, normal);
	vec3 NormalMap = texture(normalMap, textureCoordinate).xyz * 2.0 - vec3(1.0);
	return vec4(TBN * normalize(NormalMap), 0.0);
}

void main()
{
	float ao = texelFetch(AOTexture, ivec2(gl_FragCoord.xy), 0).r;
	ao = (clamp(1.0 - (1.0 - ao), 0.0, 1.0) + MIN_AMBIENT_LIGHT) /  (1.0 + MIN_AMBIENT_LIGHT);
	vec4 diffuseTexel = texture2D(DiffuseTexture, Input.TextureCoordinate * DiffuseUVRepeat);
	vec4 glowTexel = texture2D(GlowMapTexture, Input.TextureCoordinate * GlowUVRepeat);
	vec4 specularTexel = texture2D(SpecularMapTexture, Input.TextureCoordinate * SpecularUVRepeat);
	vec4 position = V * M * vec4(Input.Position, 1.0); 
	vec4 normal = V * CalcNormalMappedValue(Input.Normal, Input.Tangent, Input.BiTangent, Input.TextureCoordinate * NormalUVRepeat, NormalMapTexture);
	normal = normalize(normal);
	//vec4 normal = normalize(V  * vec4(Input.Normal, 0.0));
	vec4 viewVec = normalize(-position);
	vec3 I = normalize(vec3(M * vec4(Input.Position, 1.0)) - CameraPosition);
	vec3 R = reflect(I, Input.Normal);
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

	for(int i = start; i < start + amount; i++) {

		int l = int(LightIndex[i]);
		LightSource light = LightSources.List[l];

		LightResult light_result;
		//These if statements should be removed.
		if(light.Type == 1) { // point
			light_result = CalcPointLightSource(V * light.Position, light.Radius, light.Color, light.Intensity, viewVec, position, normal, light.Falloff);
		} else if (light.Type == 2) { //Directional
			light_result = CalcDirectionalLightSource(V * light.Direction, light.Color, light.Intensity, viewVec, normal);
		}
		totalLighting.Diffuse += vec4(light_result.Diffuse.rgb * ao, light_result.Diffuse.a);
		totalLighting.Specular += vec4(light_result.Specular.rgb * ao, light_result.Specular.a);
	}

	vec4 color_result = mix((Color * diffuseTexel * DiffuseColor), Input.ExplosionColor, Input.ExplosionPercentageElapsed);
	color_result = color_result * (totalLighting.Diffuse + (totalLighting.Specular * specularTexel));
	float specularResult = (specularTexel.r + specularTexel.g + specularTexel.b)/3.0;
	//color_result = color_result * clamp(1/specularTexel, 0, 1) + reflectionColor * clamp(specularTexel, 0, 1);
	//vec4 color_result = (DiffuseColor + Input.ExplosionColor) * (totalLighting.Diffuse + (totalLighting.Specular * specularTexel)) * diffuseTexel * Color;
	

	float pos = ((P * vec4(Input.Position, 1)).y + 1.0)/2.0;

	if(pos <= FillPercentage) {
		color_result += FillColor;
	}
	sceneColor = vec4(color_result.xyz, clamp(color_result.a, 0, 1));
	//sceneColor = vec4(reflectionColor.xyz, 1);
	color_result += glowTexel*GlowIntensity;

	bloomColor = vec4(clamp(color_result.xyz - 1.0, 0, 100), clamp(color_result.a, 0, 1));

	//Tiled Debug Code
	/*
	if(int(gl_FragCoord.x)%16 == 0 || int(gl_FragCoord.y)%16 == 0 ) {
		sceneColor += vec4(0.5, 0, 0, 0);
	} else {
		sceneColor += vec4(LightGrids.Data[int(tilePos.x + tilePos.y*80)].Amount/LightSources.List.length(), 0, 0, 1);
	}
	*/
}


