#version 430

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;

uniform sampler2D texture0;


struct PointLight {
	vec4 Position;
	vec4 Color;
	float Radius;
	float Intensity;
	float Falloff;
	float Padding;
};

layout (std430, binding = 1) buffer LightBuffer
{
	PointLight List[];
} PointLights;

struct LightGrid {
	int Amount;
	int Start;
	vec2 Padding;
};

layout (std430, binding = 2) buffer LightGridBuffer
{
	LightGrid Data[];
} LightGrids;

layout (std430, binding = 4) buffer LightIndexBuffer
{
	int LightIndex[];
};


in VertexData{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoordinate;
	vec4 DiffuseColor;
}Input;

out vec4 fragmentColor;

vec4 scene_ambient = vec4(0.6,0.6,0.6,1);

struct LightResult {
	vec4 Diffuse;
	vec4 Specular;
};

float CalcAttenuation(float radius, float dist) {
	return 1.0 - smoothstep(radius * 1.0, radius, dist);
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

LightResult CalcPointLight(vec4 lightPos, float lightRadius, vec4 lightColor, float intensity, vec4 viewVec, vec4 position, vec4 normal)
{
	vec4 L = lightPos - position;
	float dist = length(L);
	L = normalize(L);

	float attenuation = CalcAttenuation(lightRadius, dist);

	LightResult result;
	result.Diffuse = CalcDiffuse(lightColor, L, normal) * attenuation * intensity;
	result.Specular = CalcSpecular(lightColor, viewVec, L, normal) * attenuation *  intensity;
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
	
	//for(int i = 0; i < 3; i++)
	for(int i = LightGrids.Data[int(tilePos.x + tilePos.y*80)].Start; i < LightGrids.Data[int(tilePos.x + tilePos.y*80)].Amount; i++)
	{
		int l = LightIndex[i];

		LightResult result = CalcPointLight(V * PointLights.List[l].Position, PointLights.List[l].Radius, PointLights.List[l].Color, PointLights.List[l].Intensity, viewVec, position, normal);

		totalLighting.Diffuse += result.Diffuse;
		totalLighting.Specular += result.Specular;
	}

	fragmentColor += Input.DiffuseColor * (totalLighting.Diffuse + totalLighting.Specular) * texel * Color;
	//fragmentColor = texel * Input.DiffuseColor * Color;
	if(int(gl_FragCoord.x)%16 == 0 || int(gl_FragCoord.y)%16 == 0 )
	{
		fragmentColor = vec4(0.5, 0, 0, 0);
	} else {
		//fragmentColor = vec4(LightGrids.Data[int(tilePos.x + tilePos.y*80)].Start/3600, 0, 0, 1);
	}

}


