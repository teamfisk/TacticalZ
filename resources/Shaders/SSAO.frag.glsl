#version 430

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
//#define uRadius 1.0f

uniform float           uBias;
//#define uBias 0.05f

uniform float           uContrast;
//#define uContrast 1.5f

uniform float           uIntensityScale;
//#define uIntensityScale 1.0f

out float				AO;

vec3 getVSPosition(ivec2 ScreenSpaceCoord) {
    float z = texelFetch(ViewSpaceZ, ScreenSpaceCoord, 0).r;
    //Get the xy view space coordinates and add the z value from ViewSpaceZ buffer.
    return vec3((uProjInfo[0] + (ScreenSpaceCoord.x * uProjInfo[1])) * z, (uProjInfo[2] + (ScreenSpaceCoord.y * uProjInfo[3])) * z, z);
}

vec3 getVSFaceNormal(vec3 ViewSpacePosition) {
	// Get tangets vector for the plane and ViewSpacePositin... don't ask how this functions works. It's pure magic. 
	// They do this and it just works... I would guess that they approximate the function of a plane from pixels close to the pixel were on now.
	return normalize(cross(dFdx(ViewSpacePosition), dFdy(ViewSpacePosition)));
}


vec3 getSampleViewSpacePos(ivec2 ScreenSpaceCoord, int SampleIndex, float RotationAngle, float ScreenSpaceSampleRadius){
	// Pure Magic...
	float alpha = float(SampleIndex) * (1.0 / uNumOfSamples);

	// Angle to where to sample
    float angle = alpha * (uNumOfTurns * 6.28) + RotationAngle;

	//Lenght to were to sample
    ScreenSpaceSampleRadius = ScreenSpaceSampleRadius * alpha;

    vec2 screenSpaceSampleOffsetVecor = vec2(cos(angle), sin(angle));

    // Get texel coordinate on where to sample by going screenSpaceSampleOffsetVecor direction in ScreenSpaceSampleRadius units from ScreenSpaceCoord (the point being shaded);
    ivec2 screenSpaceSampleTexel = ivec2(ScreenSpaceSampleRadius * screenSpaceSampleOffsetVecor) + ScreenSpaceCoord;

    return getVSPosition(screenSpaceSampleTexel);
}



float sampleAO(ivec2 ScreenSpaceCoord, vec3 Origin, vec3 OriginNormal, float ScreenSpaceSampleRadius, int SampleIndex, float RotationAngle, float Radius) {
	float radius2 = Radius * Radius;
	vec3 sampleViewSpacePosition = getSampleViewSpacePos(ScreenSpaceCoord, SampleIndex, RotationAngle, ScreenSpaceSampleRadius);

	vec3 sampleVector = Origin - sampleViewSpacePosition;

	// vv = sampleVectorLenght ^ 2
	float vv = dot(sampleVector, sampleVector);
	// vn = angle between sampleVector and Normal
	float vn = dot(sampleVector, OriginNormal);

	const float epsilon = 0.0001f;

	// vv < radius2 if the vector is shorter then the radius;
	// vn - bias, offset the angle to reduse self occlusion. 
	// epsilon is here to make divison by 0 impossible.
	return float(vv < radius2) * max((vn - uBias) / (epsilon + vv), 0.0);
	//float f = max(radius2 - vv, 0.0); 
	//return f * f * f * max((vn - uBias) / (epsilon + vv), 0.0);
}


void main() {
	ivec2 originScreenCoord = ivec2(gl_FragCoord.xy);

	vec3 origin = getVSPosition(originScreenCoord);

	float radius = min(origin.z, uRadius);

	vec3 originNormal = getVSFaceNormal(origin);

	float screenSpaceSampleRadius = -uProjScale * radius / origin.z;

	float rotationAngleOffset = 30 * originScreenCoord.x ^ originScreenCoord.y + 10 * originScreenCoord.x * originScreenCoord.y;

	float sum = 0.0;
    for (int i = 0; i < uNumOfSamples; i++) {
        sum += sampleAO(originScreenCoord, origin, originNormal, screenSpaceSampleRadius, i, rotationAngleOffset, radius);
    }

    //float A = max(0.0, 1.0 - sum * (2.0f / uNumOfSamples));
    float A = 1.0 - sum * (2.0f * uIntensityScale / float(uNumOfSamples));
    AO = clamp(pow(A, uContrast), 0.0f, 1.0f);
	//AO = vec4(originNormal, 1.0f);
}
