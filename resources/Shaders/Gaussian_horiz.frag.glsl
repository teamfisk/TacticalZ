#version 430
#extension GL_EXT_gpu_shader4 : enable

layout (binding = 0) uniform sampler2D Texture;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec2 tex_offset = 1.0 / textureSize2D(Texture, 0);
	vec3 center = texture(Texture, Input.TextureCoordinate).rgb;
	vec3 result = center * weight[0];
	for(int i = 1; i < 5; ++i) {
		vec4 eastFragments = texture(Texture, Input.TextureCoordinate + vec2(tex_offset.x * i, 0.0));
		vec4 westFragments = texture(Texture, Input.TextureCoordinate - vec2(tex_offset.x * i, 0.0));
	    result += eastFragments.rgb * weight[i] * eastFragments.a;
	    result += westFragments.rgb * weight[i] * westFragments.a;
	    result += center * weight[i] * (1.0 - westFragments.a);
	    result += center * weight[i] * (1.0 - eastFragments.a);
	}
	fragmentColor = vec4(result, 1.0);
}