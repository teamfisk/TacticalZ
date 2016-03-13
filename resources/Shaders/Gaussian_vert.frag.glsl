#version 430
#extension GL_EXT_gpu_shader4 : enable

layout (binding = 0) uniform sampler2D Texture;
uniform int Lod;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
//uniform float weight[3] = float[](0.265495, 0.226535, 0.140718);

void main()
{
	vec2 tex_offset = 1.0 / textureSize(Texture, Lod);
	vec3 center = texture(Texture, Input.TextureCoordinate).rgb;
	vec3 result = center * weight[0];

    for(int i = 1; i < 5; ++i) {
    	vec4 northFragments = texture(Texture, Input.TextureCoordinate + vec2(0.0, tex_offset.y * i));
    	vec4 southFragments = texture(Texture, Input.TextureCoordinate - vec2(0.0, tex_offset.y * i));
        result += northFragments.rgb * weight[i] * northFragments.a;
        result += southFragments.rgb * weight[i] * southFragments.a;
        result += center * weight[i] * (1.0 - northFragments.a);
	    result += center * weight[i] * (1.0 - southFragments.a);
    }
    fragmentColor = vec4(result, 1.0);
}