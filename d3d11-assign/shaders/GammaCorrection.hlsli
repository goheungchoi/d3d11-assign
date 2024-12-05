#ifndef __GAMMA_CORRECTION__
#define __GAMMA_CORRECTION__

inline float sRGB2LinearExact(float value)
{
	if (value <= 0.04045f) 
		return value / 12.92f;
	else if (value < 1.f)
		return pow((value + 0.055f) / 1.055f, 2.4f);
	else
		return pow(value, 2.2f);
}

inline float3 sRGB2Linear(float3 sRGB)
{
	return pow(sRGB, 2.233333);
}

inline float3 Linear2sRGB(float3 linRGB)
{
	return pow(linRGB, 1.0/2.233333);
}

inline half3 sRGB2LinearAppr(half3 sRGB)
{
	// Approximate version from 
	// http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
	return sRGB * (sRGB * (sRGB * 0.305306011h + 0.682171111h) + 0.012522878h);
	// Precise version, useful for debugging.
	//return half3(GammaToLinearSpaceExact(sRGB.r), 
	//GammaToLinearSpaceExact(sRGB.g),
	//GammaToLinearSpaceExact(sRGB.b));
}


#endif
