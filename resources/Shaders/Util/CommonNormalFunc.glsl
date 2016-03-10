#define NORMAL_DDS 1
#define NORMAL_PNG 2

vec3 NormalMapValue(vec2 textureCoordinate, sampler2D normalMap, int normalTextureType){
	if(normalTextureType == NORMAL_DDS){
		vec3 normal;
		normal.xy = texture(normalMap, textureCoordinate).wy * 2.0 - vec2(1.0);
		normal.z = sqrt(1.0 -  clamp(dot(normal.xy, normal.xy), 0.0, 1.0));
		return normal;
	} else {
		//should be PNG or anything that doesn't need special support
		return texture(normalMap, textureCoordinate).xyz * 2.0 - vec3(1.0);
	}
}

vec4 CalcNormalMappedValue(vec3 normal, vec3 tangent, vec3 bitangent, vec2 textureCoordinate, sampler2D normalMap, int normalTextureType)
{
	mat3 TBN = mat3(tangent, bitangent, normal);
	vec3 NormalMap = NormalMapValue(textureCoordinate, normalMap, normalTextureType);
	return vec4(TBN * normalize(NormalMap), 0.0);
}