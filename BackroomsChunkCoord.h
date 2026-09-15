#pragma once

#include "CoreMinimal.h"
#include "BackroomsChunkCoord.generated.h"

// Стабильная координата чанка в мировой 2D-сетке (по осям X/Y карты уровней).
// В отличие от Akтора (который может пережить пересоздание), координата —
// чистое value-тип, однозначно идентифицирующий «какое это поколение мира».
USTRUCT(BlueprintType)
struct FChunkCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkCoord")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkCoord")
	int32 Y = 0;

	FChunkCoord() = default;
	FChunkCoord(int32 InX, int32 InY) : X(InX), Y(InY) {}

	bool operator==(const FChunkCoord& Other) const { return X == Other.X && Y == Other.Y; }
	bool operator!=(const FChunkCoord& Other) const { return !(*this == Other); }

	bool operator<(const FChunkCoord& Other) const
	{
		return (Y != Other.Y) ? (Y < Other.Y) : (X < Other.X);
	}

	FChunkCoord operator+(const FChunkCoord& Other) const { return FChunkCoord(X + Other.X, Y + Other.Y); }
	FChunkCoord operator-(const FChunkCoord& Other) const { return FChunkCoord(X - Other.X, Y - Other.Y); }
};

FORCEINLINE uint32 GetTypeHash(const FChunkCoord& C)
{
	return HashCombine(GetTypeHash(C.X), GetTypeHash(C.Y));
}

// Хорошее число для FMath::RandInit / FRandomStream: стандартный взлом Vuillemin
// (как в vec2( seed, level ).mix) — даёт равномерный разброс соседних чанков.
inline uint64 BackroomsChunkSeed(uint64 WorldSeed, uint64 LevelIndex, const FChunkCoord& Coord)
{
	const uint64 PX = 0x9E3779B97F4A7C15ull * (uint64)(uint32)Coord.X;
	const uint64 PY = 0xBF58476D1CE4E5B9ull * (uint64)(uint32)Coord.Y;
	const uint64 PL = 0x94D049BB133111EBull * LevelIndex;
	uint64 H = WorldSeed ^ PX ^ PY ^ PL;
	H ^= H >> 30;
	H *= 0xBF58476D1CE4E5B9ull;
	H ^= H >> 27;
	H *= 0x94D049BB133111EBull;
	H ^= H >> 31;
	return H;
}