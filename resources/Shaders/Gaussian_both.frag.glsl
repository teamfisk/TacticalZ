#version 430
#extension GL_EXT_gpu_shader4 : enable

uniform int MipMapLevel;
uniform bool Horizontal;
uniform vec2 ScreenSize[5];

layout (binding = 0) uniform sampler2D Texture;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );

void main()
{
	//vec2 tex_offset = 1.0 / textureSize2D(Texture, 0);
	vec3 result = textureLod(Texture, vec2(gl_FragCoord)/ScreenSize[0], MipMapLevel ).rgb * weight[0];

	if(Horizontal) {
		for( int i = 1; i<3; i++) {
			result += textureLod( Texture, Input.TextureCoordinate + vec2(offset[i], 0.0), MipMapLevel ).rgb * weight[i];
			result += textureLod( Texture, Input.TextureCoordinate - vec2(offset[i], 0.0), MipMapLevel ).rgb * weight[i];
		}
	} else {
		for( int i = 1; i<3; i++) {
			result += textureLod( Texture, Input.TextureCoordinate + vec2(0.0, offset[i]), MipMapLevel ).rgb * weight[i];
			result += textureLod( Texture, Input.TextureCoordinate - vec2(0.0, offset[i]), MipMapLevel ).rgb * weight[i];
		}
	}
	fragmentColor = vec4(result, 1.0);
}