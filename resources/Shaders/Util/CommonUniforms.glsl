vec4 CalcNormalMappedValue(vec3 normal, vec3 tangent, vec3 bitangent, vec2 textureCoordinate, sampler2D normalMap)
{
	mat3 TBN = mat3(tangent, bitangent, normal);
	vec3 NormalMap = texture(normalMap, textureCoordinate).xyz * 2.0 - vec3(1.0);
	return vec4(TBN * normalize(NormalMap), 0.0);
}