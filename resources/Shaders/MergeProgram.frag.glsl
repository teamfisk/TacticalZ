#version 430

layout (binding = 1) uniform sampler2D DepthStencilTexture;
layout (binding = 2) uniform sampler2D SceneTexture;
layout (binding = 3) uniform sampler2D BloomTexture;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 sceneColor;
out vec4 bloomColor;

void main()
{
	float texel = texelFetch(DepthStencilTexture, ivec2(gl_FragCoord.xy / 8.0f), 0).g;
	if(texel >= 0.9f)
	{
		discard;
	}
	sceneColor = vec4(texture2D(SceneTexture, Input.TextureCoordinate).rgb * texel, 1.0f);
	bloomColor = vec4(texture2D(BloomTexture, Input.TextureCoordinate).rgb * texel, 1.0f);
	//sceneColor = texel;
	//bloomColor = vec4(1,0.5,0.7,1);
}


