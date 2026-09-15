#include "BackroomsNoise.h"

namespace
{
	int32 Perm[512];
	int32 PermSeed = INT32_MIN;
	FCriticalSection PermLock;

	void EnsurePerm(int32 Seed)
	{
		FScopeLock Lock(&PermLock);
		if (PermSeed == Seed)
		{
			return;
		}

		FRandomStream Stream(Seed);
		TArray<int32> P;
		P.SetNum(256);
		for (int32 i = 0; i < 256; ++i)
		{
			P[i] = i;
		}
		for (int32 i = 255; i > 0; --i)
		{
			int32 j = Stream.RandRange(0, i);
			P.Swap(i, j);
		}
		for (int32 i = 0; i < 512; ++i)
		{
			Perm[i] = P[i & 255];
		}
		PermSeed = Seed;
	}

	float Fade(float T)
	{
		return T * T * T * (T * (T * 6.0f - 15.0f) + 10.0f);
	}

	float Lerp(float A, float B, float T)
	{
		return A + T * (B - A);
	}

	float Grad2D(int32 Hash, float X, float Y)
	{
		switch (Hash & 7)
		{
		case 0: return X + Y;
		case 1: return X - Y;
		case 2: return -X + Y;
		case 3: return -X - Y;
		case 4: return X;
		case 5: return -X;
		case 6: return Y;
		default: return -Y;
		}
	}

	float Grad3D(int32 Hash, float X, float Y, float Z)
	{
		// 12 классических направлений куба для изотропного 3D-шума.
		switch (Hash & 15)
		{
		case 0:  return X + Y;
		case 1:  return X - Y;
		case 2:  return -X + Y;
		case 3:  return -X - Y;
		case 4:  return X + Z;
		case 5:  return X - Z;
		case 6:  return -X + Z;
		case 7:  return -X - Z;
		case 8:  return Y + Z;
		case 9:  return Y - Z;
		case 10: return -Y + Z;
		case 11: return -Y - Z;
		case 12: return X;
		case 13: return Y;
		default: return Z;
		}
	}
}

float BackroomsNoise::Perlin2D(float X, float Y, int32 Seed)
{
	EnsurePerm(Seed);

	int32 Xi = (int32)FMath::Floor(X);
	int32 Yi = (int32)FMath::Floor(Y);
	float Xf = X - (float)Xi;
	float Yf = Y - (float)Yi;
	float U = Fade(Xf);
	float V = Fade(Yf);

	int32 A = Perm[Xi & 255] + Yi;
	int32 B = Perm[(Xi + 1) & 255] + Yi;

	float N00 = Grad2D(Perm[A & 255], Xf, Yf);
	float N01 = Grad2D(Perm[(A + 1) & 255], Xf, Yf - 1.0f);
	float N10 = Grad2D(Perm[B & 255], Xf - 1.0f, Yf);
	float N11 = Grad2D(Perm[(B + 1) & 255], Xf - 1.0f, Yf - 1.0f);

	float NX0 = Lerp(N00, N10, U);
	float NX1 = Lerp(N01, N11, U);
	return Lerp(NX0, NX1, V);
}

float BackroomsNoise::Perlin3D(float X, float Y, float Z, int32 Seed)
{
	EnsurePerm(Seed);

	int32 Xi = (int32)FMath::Floor(X);
	int32 Yi = (int32)FMath::Floor(Y);
	int32 Zi = (int32)FMath::Floor(Z);
	float Xf = X - (float)Xi;
	float Yf = Y - (float)Yi;
	float Zf = Z - (float)Zi;
	float U = Fade(Xf);
	float V = Fade(Yf);
	float W = Fade(Zf);

	int32 A  = Perm[Xi     & 255] + Yi;
	int32 AA = Perm[A      & 255] + Zi;
	int32 AB = Perm[(A + 1) & 255] + Zi;
	int32 B  = Perm[(Xi + 1) & 255] + Yi;
	int32 BA = Perm[B      & 255] + Zi;
	int32 BB = Perm[(B + 1) & 255] + Zi;

	float N000 = Grad3D(Perm[AA & 255], Xf,     Yf,     Zf);
	float N001 = Grad3D(Perm[(AA + 1) & 255], Xf,     Yf,     Zf - 1.0f);
	float N010 = Grad3D(Perm[AB & 255], Xf,     Yf - 1.0f, Zf);
	float N011 = Grad3D(Perm[(AB + 1) & 255], Xf,     Yf - 1.0f, Zf - 1.0f);
	float N100 = Grad3D(Perm[BA & 255], Xf - 1.0f, Yf,     Zf);
	float N101 = Grad3D(Perm[(BA + 1) & 255], Xf - 1.0f, Yf,     Zf - 1.0f);
	float N110 = Grad3D(Perm[BB & 255], Xf - 1.0f, Yf - 1.0f, Zf);
	float N111 = Grad3D(Perm[(BB + 1) & 255], Xf - 1.0f, Yf - 1.0f, Zf - 1.0f);

	float NY00 = Lerp(N000, N100, U);
	float NY01 = Lerp(N001, N101, U);
	float NY10 = Lerp(N010, N110, U);
	float NY11 = Lerp(N011, N111, U);

	float NZ0 = Lerp(NY00, NY10, V);
	float NZ1 = Lerp(NY01, NY11, V);
	return Lerp(NZ0, NZ1, W);
}

float BackroomsNoise::Noise3D01(float X, float Y, float Z, bool bUseTime, float WorldTime, float ZScale, int32 Seed)
{
	const float ZZ = bUseTime ? WorldTime * ZScale : Z;
	const float V = Perlin3D(X, Y, ZZ, Seed);
	return FMath::Clamp(V * 0.5f + 0.5f, 0.0f, 1.0f);
}

float BackroomsNoise::TemporalNoise(float WorldTime, float TimeScale, float Offset, int32 Seed)
{
	const float T = WorldTime * TimeScale + Offset;
	const float V = Perlin2D(T * 0.7f, T * 0.5f + (float)(Seed % 997), Seed ^ 0xA7F3);
	return FMath::Clamp(V * 0.5f + 0.5f, 0.0f, 1.0f);
}

