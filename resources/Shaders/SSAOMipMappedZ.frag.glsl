#version 430

#define MIP_MAPS_LEVELS (5)

#define LOWER_MIP_LEVEL (2)
//Number of samples per pixel
uniform int           uNumOfSamples;
//#define uNumOfSamples (11)

//Number of turns around the cirle
uniform int           uNumOfTurns;
//#define uNumOfTurns (7)

layout (binding = 0) uniform sampler2D ViewSpaceZ;

uniform vec4 uProjInfo;

uniform float           uProjScale;
//#define ProjScale 500

uniform float           uRadius;
//#define Radius 1.0f

uniform float           uBias;
//#define Bias 0.012f

uniform float           uContrast;
//#define IntensityDivR6 1

uniform float           uIntensityScale;

out float				AO;

vec3 getVSPosition(ivec2 ScreenSpaceCoord, int mipmaplevel) {
    float z = texelFetch(ViewSpaceZ, ScreenSpaceCoord, mipmaplevel).r;
    //Get the xy view space coordinates and add the z value from ViewSpaceZ buffer.
    return vec3((uProjInfo[0] + (ScreenSpaceCoord.x * uProjInfo[1])) * z, (uProjInfo[2] + (ScreenSpaceCoord.y * uProjInfo[3])) * z, z);
}

vec3 getVSFaceNormal(vec3 ViewSpacePosition) {
	// Get tangets vector for the plane and ViewSpacePositin... don't ask how this functions works. It's pure magic. 
	// They do this and it just works... I would guess that they approximate the function of a plane from pixels close to the pixel were on now.
	return normalize(cross(dFdx(ViewSpacePosition), dFdy(ViewSpacePosition)));
}


vec3 getSampleViewSpacePos(ivec2 ScreenSpaceCoord, int SampleIndex, float RotationAngle, float ScreenSpaceSampleRadius){
	// SampleIndex + 0.5f so that we alpha != 0, since if alpha = 0 then screenSpaceSampleTexel will be the same texel as origin. 
	float alpha = float(SampleIndex + 0.5f) * (1.0f / uNumOfSamples);

	// Angle to where to sample
    float angle = alpha * (uNumOfTurns * 6.28f) + RotationAngle;

	//Lenght to were to sample
    ScreenSpaceSampleRadius = ScreenSpaceSampleRadius * alpha;

    vec2 screenSpaceSampleOffsetVecor = vec2(cos(angle), sin(angle));

    int mipmapLevel = clamp(int(floor(log2(-ScreenSpaceSampleRadius))) - LOWER_MIP_LEVEL, 0, MIP_MAPS_LEVELS);
    // Get texel coordinate on where to sample by going screenSpaceSampleOffsetVecor direction in ScreenSpaceSampleRadius units from ScreenSpaceCoord (the point being shaded);
    ivec2 screenSpaceSampleTexel = (ivec2(ScreenSpaceSampleRadius * screenSpaceSampleOffsetVecor) + ScreenSpaceCoord) >> mipmapLevel;

    return getVSPosition(screenSpaceSampleTexel, mipmapLevel);
}



float sampleAO(ivec2 ScreenSpaceCoord, vec3 Origin, vec3 OriginNormal, float ScreenSpaceSampleRadius, int SampleIndex, float RotationAngle, float Radius, float Radius2) {
	vec3 sampleViewSpacePosition = getSampleViewSpacePos(ScreenSpaceCoord, SampleIndex, RotationAngle, ScreenSpaceSampleRadius);
	vec3 sampleVector = Origin - sampleViewSpacePosition;
	// vv = sampleVectorLenght ^ 2
	float vv = dot(sampleVector, sampleVector);
	// vn = angle between sampleVector and Normal
	float vn = dot(sampleVector, OriginNormal);

	const float epsilon = 0.01f;

	// vv < radius2 if the vector is shorter then the radius;
	// vn - bias, offset the angle to reduse self occlusion. 
	// epsilon is here to make divison by 0 impossible.
	return float(vv < Radius2) * max((vn - uBias) / (epsilon + vv), 0.0);
}


void main() {
	ivec2 originScreenCoord = ivec2(gl_FragCoord.xy);

	// We always get the Orginin from the level 0 of the mipmaps
	vec3 origin = getVSPosition(originScreenCoord, 0);

	float radius;

	if(origin.z < uRadius){
		radius = origin.z;
	} else {
		radius = uRadius;
	}

	float radius2 = radius * radius;

	vec3 originNormal = getVSFaceNormal(origin);
	//AO = originNormal;
	//return;

	//screenSpaceSampleRadius is in pixels
	float screenSpaceSampleRadius = -uProjScale * radius / origin.z;

	//screenSpaceSampleRadius = clamp(screenSpaceSampleRadius, 0.0f, pow(2, MIP_MAPS_LEVELS * LOWER_MIP_LEVEL + 1));
	//AO = clamp(int(floor(log2(screenSpaceSampleRadius))) - LOWER_MIP_LEVEL, 0, MIP_MAPS_LEVELS);
	//return;

	float rotationAngleOffset = 30 * originScreenCoord.x ^ originScreenCoord.y + 10 * originScreenCoord.x * originScreenCoord.y;

	float sum = 0.0f;
    for(int i = 0; i < uNumOfSamples; i++) {
        sum += sampleAO(originScreenCoord, origin, originNormal, screenSpaceSampleRadius, i, rotationAngleOffset, radius, radius2);
    }

    //float A = max(0.0, 1.0 - sum * (2.0f / uNumOfSamples));
    float A = 1.0f - sum * (2.0f * uIntensityScale / float(uNumOfSamples));
    AO = clamp(pow(A, uContrast), 0.0f, 1.0f);
	//AO = vec4(originNormal, 1.0f);
}
