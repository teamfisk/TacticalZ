#version 430

//Number of samples per pixel
#define NUM_SAMPLES (24)

//Number of turns around the cirle
#define NUM_TURNS (7)

uniform sampler2D ViewSpaceZ;

uniform vec4 ProjInfo;

uniform float           ProjScale;
//#define ProjScale 500

uniform float           Radius;
//#define Radius 1.0f

uniform float           Bias;
//#define Bias 0.012f

uniform float           IntensityDivR6;
//#define IntensityDivR6 1

out vec4				fragmentColor;

vec3 reconstructVSPosition(vec2 ScreenSpaceCoord, float z){
	return vec3((ScreenSpaceCoord * ProjInfo.xy + ProjInfo.zw) * z, z);
}

vec3 getPosition(ivec2 ScreenSpaceCoord) {
	vec3 P;
    P.z = texelFetch(ViewSpaceZ, ScreenSpaceCoord, 0).r;
    //Get the xy view space coordinates and add the z value from ViewSpaceZ buffer.
    return reconstructVSPosition(vec2(ScreenSpaceCoord) + vec2(0.5), P.z);
}

vec3 getVSFaceNormal(vec3 ViewSpacePosition) {
	// Get tangets vector for the plane and ViewSpacePositin... don't ask how this functions works. It's pure magic. 
	// They do this and it just works... I would guess that they approximate the function of a plane from pixels close to the pixel were on now.
	return normalize(cross(dFdx(ViewSpacePosition), dFdy(ViewSpacePosition)));
}


vec3 getSampleViewSpacePos(ivec2 ScreenSpaceCoord, int SampleIndex, float RotationAngle, float ScreenSpaceSampleRadius){
	// Pure Magic...
	float alpha = float(SampleIndex + 0.5) * (1.0 / NUM_SAMPLES);

	// Angle to where to sample
    float angle = alpha * (NUM_TURNS * 6.28) + RotationAngle;

	//Lenght to were to sample
    ScreenSpaceSampleRadius = ScreenSpaceSampleRadius * alpha;

    vec2 screenSpaceSampleOffsetVecor = vec2(cos(angle), sin(angle));

    // Get texel coordinate on where to sample by going screenSpaceSampleOffsetVecor direction in ScreenSpaceSampleRadius units from ScreenSpaceCoord (the point being shaded);
    ivec2 screenSpaceSampleTexel = ivec2(ScreenSpaceSampleRadius * screenSpaceSampleOffsetVecor) + ScreenSpaceCoord;

    return getPosition(screenSpaceSampleTexel);
}

float Radius2 = Radius * Radius;

float sampleAO(ivec2 ScreenSpaceCoord, vec3 ShadedViewSpacePosition, vec3 ViewSpaceNormal, float ScreenSpaceSampleRadius, int SampleIndex, float RotationAngle) {
	vec3 sampleViewSpacePosition = getSampleViewSpacePos(ScreenSpaceCoord, SampleIndex, RotationAngle, ScreenSpaceSampleRadius);

	vec3 sampleVector = ShadedViewSpacePosition - sampleViewSpacePosition;

	// vv = sampleVectorLenght ^ 2
	float vv = dot(sampleVector, sampleVector);
	// vn = angle between sampleVector and Normal
	float vn = dot(sampleVector, ViewSpaceNormal);

	const float epsilon = 0.01f;

	// vv < radius2 if the vector is shorter then the radius;
	// vn - bias, offset the angle to reduse self occlusion. 
	// epsilon is here to make divison by 0 impossible.
	return float(vv < Radius2) * max((vn - Bias) / (epsilon + vv), 0.0);
	//float f = max(Radius2 - vv, 0.0); 
	//return f * f * f * max((vn - Bias) / (epsilon + vv), 0.0);
}


void main() {
	ivec2 originScreenCoord = ivec2(gl_FragCoord.xy);

	vec3 origin = getPosition(originScreenCoord);

	vec3 viewSpaceNormal = getVSFaceNormal(origin);

	float screenSpaceSampleRadius = ProjScale * Radius / origin.z;

	//Offset on what angle to start on so that not evry pixel start sampling in the same direction, AlchemyAO
	float rotationAngleOffset = (3 * originScreenCoord.x ^ originScreenCoord.y + originScreenCoord.x * originScreenCoord.y) * 10;

	float sum = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += sampleAO(originScreenCoord, origin, viewSpaceNormal, screenSpaceSampleRadius, i, rotationAngleOffset);
    }

    float A = max(0.0, 1.0 - sum * (2.0f / NUM_SAMPLES));
	//fragmentColor= vec4(viewSpaceNormal, 1.0f);
	fragmentColor = vec4(A, A, A, 1.0f);
}
