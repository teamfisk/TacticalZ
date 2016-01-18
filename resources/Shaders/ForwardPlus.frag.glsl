#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;
uniform vec2 ScreenDimensions;
uniform sampler2D texture0;

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
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
}Input;

out vec4 fragmentColor;

vec4 scene_ambient = vec4(0.3,0.3,0.3,1);

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

LightResult CalcDirectionalLightSource(vec4 direction, vec4 color, float intensity, vec4 viewVec, vec4 vertPosition, vec4 vertNormal)
{
	vec4 L = normalize( -direction );

	LightResult result;
	result.Diffuse = CalcDiffuse(color, L, vertNormal) * intensity;
	result.Specular = CalcSpecular(color, viewVec, L, vertNormal) * intensity;
	return result;
}


void main()
{
	vec4 texel = texture2D(texture0, Input.TextureCoordinate);
	vec4 position = V * M * vec4(Input.Position, 1.0); 
	vec4 normal = V  * vec4(Input.Normal, 0.0);
	vec4 viewVec = normalize(-position); 

	vec2 tilePos;
	tilePos.x = int(gl_FragCoord.x/16);
	tilePos.y = int(gl_FragCoord.y/16);

	LightResult totalLighting;
	totalLighting.Diffuse = scene_ambient;
	int currentTile = int(floor(gl_FragCoord.x/TILE_SIZE) + (floor(gl_FragCoord.y/TILE_SIZE) * int(ScreenDimensions.x/TILE_SIZE)));

	int start = int(LightGrids.Data[currentTile].Start);
	int amount = int(LightGrids.Data[currentTile].Amount);
	//for(int i = 0; i < 3; i++)
	for(int i = start; i < start + amount; i++) {
		int l = int(LightIndex[i]);
		LightResult result;

		if(LightSources.List[i].Type == 0) { // point
			//result = CalcPointLightSource(V * LightSources.List[l].Position, LightSources.List[l].Radius, LightSources.List[l].Color, LightSources.List[l].Intensity, viewVec, position, normal, LightSources.List[i].Falloff);
		} else if (LightSources.List[i].Type == 1) { //Directional
			result = CalcDirectionalLightSource(V * LightSources.List[l].Direction, LightSources.List[i].Color, LightSources.List[i].Intensity, viewVec, position, normal);
		}
		totalLighting.Diffuse += result.Diffuse;
		totalLighting.Specular += result.Specular;
	}


	fragmentColor += Input.DiffuseColor;
	//fragmentColor += Input.DiffuseColor * (totalLighting.Diffuse + totalLighting.Specular) * texel * Color;
	//fragmentColor += Input.DiffuseColor * (totalLighting.Diffuse) * texel * Color;
	//fragmentColor += Input.DiffuseColor + vec4(0.0, LightGrids.Data[currentTile].Amount/3.0, 0, 1);
	if(LightSources.List[0].Type == 0) {
		fragmentColor +=  vec4(1,0,0,1);
	}
	if(LightSources.List[0].Type == 1) {
		fragmentColor +=  vec4(0,1,0,1);
	}
	if(LightSources.List[0].Type == 2) {
		fragmentColor +=  vec4(0,0,1,1);
	}
	//fragmentColor = texel * Input.DiffuseColor * Color;
	if(int(gl_FragCoord.x)%16 == 0 || int(gl_FragCoord.y)%16 == 0 ) {
		//fragmentColor += vec4(0.5, 0, 0, 0);
	} else {
		//fragmentColor += vec4(LightGrids.Data[int(tilePos.x + tilePos.y*80)].Amount/3.0, 0, 0, 1);


	}

}


