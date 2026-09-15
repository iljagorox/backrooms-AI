#include "BackroomsAuthoredRuntimeSubsystem.h"
#include "BackroomsAuthoredChunk.h"
#include "BackroomsAuthoredChunkLibrary.h"
#include "BackroomsChunkActor.h"
#include "BackroomsItemPickup.h"
#include "PropDatabase.h"
#include "Components/RectLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

namespace
{
static TAutoConsoleVariable<int32> CVarAuthoredEnabled(TEXT("BR.Authored.Enabled"), 1, TEXT("1 = use authored chunks when the configured library exists; 0 = leave legacy generation untouched."), ECVF_Default);
static const TCHAR* AuthoredLibraryPath = TEXT("/Game/Data/BackroomsAuthoredChunks.BackroomsAuthoredChunks");
static constexpr uint32 SocketPurpose = 0x534F434B;

uint32 MakeSocketPurpose(const FBackroomsAuthoredSocket& Socket)
{
    uint32 H = ::GetTypeHash(Socket.Id);
    H = HashCombineFast(H, ::GetTypeHash(static_cast<uint8>(Socket.Kind)));
    H = HashCombineFast(H, ::GetTypeHash(Socket.Tag));
    return HashCombineFast(SocketPurpose, H);
}

bool HasRequiredTag(const FPropRule& Rule, FName Tag)
{
    if (Tag.IsNone()) return false;
    for (const FName Required : Rule.RequiredRoomTags) if (Required == Tag) return true;
    return false;
}
}

bool UBackroomsAuthoredRuntimeSubsystem::ShouldCreateSubsystem(UObject* Outer) const { return Outer != nullptr; }

TStatId UBackroomsAuthoredRuntimeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UBackroomsAuthoredRuntimeSubsystem, STATGROUP_Tickables);
}

UBackroomsAuthoredChunkLibrary* UBackroomsAuthoredRuntimeSubsystem::LoadLibrary()
{
    if (CachedLibrary) return CachedLibrary;
    CachedLibrary = Cast<UBackroomsAuthoredChunkLibrary>(StaticLoadObject(UBackroomsAuthoredChunkLibrary::StaticClass(), nullptr, AuthoredLibraryPath));
    if (CachedLibrary) UE_LOG(LogTemp, Display, TEXT("BR Authored: loaded chunk library %s"), AuthoredLibraryPath);
    return CachedLibrary;
}

void UBackroomsAuthoredRuntimeSubsystem::Tick(float DeltaTime)
{
    if (CVarAuthoredEnabled->GetInt() == 0 || !GetWorld() || !LoadLibrary()) return;
    for (TActorIterator<ABackroomsChunkActor> It(GetWorld()); It; ++It)
    {
        ABackroomsChunkActor* Chunk = *It;
        if (IsValid(Chunk) && !Chunk->bSpawnPlatform && !ProcessedChunks.Contains(Chunk)) ProcessChunk(Chunk);
    }
}

void UBackroomsAuthoredRuntimeSubsystem::ProcessChunk(ABackroomsChunkActor* Chunk)
{
    UBackroomsAuthoredChunkLibrary* Library = LoadLibrary();
    if (!Library || !Chunk) return;
    const int32 WorldSeed = Chunk->WorldSeed != 0 ? Chunk->WorldSeed : Chunk->Seed;
    const FIntPoint Coord(Chunk->ChunkX, Chunk->ChunkY);
    const int32 Level = static_cast<int32>(Chunk->LevelStyle);
    UBackroomsAuthoredChunk* Authored = Library->Select(WorldSeed, Level, Coord, 0x43484B56);
    if (!Authored) { ProcessedChunks.Add(Chunk, NAME_None); return; }
    if (!BuildAuthoredChunk(Chunk, Authored)) return;
    FillSockets(Chunk, Authored);
    ProcessedChunks.Add(Chunk, Authored->ChunkId);
}

bool UBackroomsAuthoredRuntimeSubsystem::BuildAuthoredChunk(ABackroomsChunkActor* Chunk, UBackroomsAuthoredChunk* Authored)
{
    if (!Chunk || !Authored || Authored->SizeCells.X <= 0 || Authored->SizeCells.Y <= 0) return false;
    if (Chunk->GeometryMesh)
    {
        Chunk->GeometryMesh->ClearAllMeshSections();
        Chunk->GeometryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    for (UActorComponent* Component : Chunk->GetComponents())
        if (URectLightComponent* RectLight = Cast<URectLightComponent>(Component)) RectLight->SetVisibility(false);
    ClearLegacyOwnedActors(Chunk);

    UStaticMesh* Mesh = Authored->PreviewMesh.LoadSynchronous();
    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("BR Authored: %s has no PreviewMesh; authored chunk geometry is incomplete."), *Authored->GetName());
        return false;
    }
    UStaticMeshComponent* AuthoredMesh = NewObject<UStaticMeshComponent>(Chunk, NAME_None, RF_Transient);
    if (!AuthoredMesh) return false;
    AuthoredMesh->SetStaticMesh(Mesh);
    AuthoredMesh->SetMobility(EComponentMobility::Static);
    AuthoredMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    AuthoredMesh->SetCollisionProfileName(TEXT("BlockAll"));
    AuthoredMesh->AttachToComponent(Chunk->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
    const FVector ChunkOrigin(static_cast<double>(Chunk->ChunkX) * Authored->SizeCells.X * Authored->CellSize, static_cast<double>(Chunk->ChunkY) * Authored->SizeCells.Y * Authored->CellSize, Authored->FloorZ);
    AuthoredMesh->SetWorldLocation(ChunkOrigin);
    AuthoredMesh->RegisterComponent();
    UE_LOG(LogTemp, Display, TEXT("BR Authored: chunk (%d,%d) -> %s / %s"), Chunk->ChunkX, Chunk->ChunkY, *Authored->ChunkId.ToString(), *Mesh->GetName());
    return true;
}

void UBackroomsAuthoredRuntimeSubsystem::ClearLegacyOwnedActors(ABackroomsChunkActor* Chunk) const
{
    if (!GetWorld() || !Chunk) return;
    TArray<AActor*> OwnedActors;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->GetOwner() == Chunk) OwnedActors.Add(Actor);
    }
    for (AActor* Actor : OwnedActors) if (IsValid(Actor)) Actor->Destroy();
}

void UBackroomsAuthoredRuntimeSubsystem::FillSockets(ABackroomsChunkActor* Chunk, UBackroomsAuthoredChunk* Authored)
{
    if (!Chunk || !Authored || !GetWorld()) return;
    const int32 WorldSeed = Chunk->WorldSeed != 0 ? Chunk->WorldSeed : Chunk->Seed;
    const FBackroomsSeedContext Context{WorldSeed, Authored->LevelIndex, FIntPoint(Chunk->ChunkX, Chunk->ChunkY)};
    const FVector ChunkOrigin(static_cast<double>(Chunk->ChunkX) * Authored->SizeCells.X * Authored->CellSize, static_cast<double>(Chunk->ChunkY) * Authored->SizeCells.Y * Authored->CellSize, Authored->FloorZ);

    for (const FBackroomsAuthoredRoom& Room : Authored->Rooms)
    {
        for (const FBackroomsAuthoredSocket& Socket : Room.Sockets)
        {
            if (!Socket.bEnabled) continue;
            FTransform WorldTransform = Socket.LocalTransform;
            WorldTransform.AddToTranslation(ChunkOrigin);
            FRandomStream Stream(Context.MakeStreamSeed(MakeSocketPurpose(Socket)));

            if (Socket.Kind == EBackroomsSocketKind::Important)
            {
                if (!Chunk->ItemPickupClass || Socket.Tag.IsNone()) continue;
                FActorSpawnParameters Params;
                Params.Owner = Chunk;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                ABackroomsItemPickup* Pickup = GetWorld()->SpawnActor<ABackroomsItemPickup>(Chunk->ItemPickupClass, WorldTransform, Params);
                if (Pickup) { Pickup->ItemId = Socket.Tag; Pickup->Count = 1; }
                continue;
            }
            if (Socket.Kind == EBackroomsSocketKind::Light)
            {
                URectLightComponent* Light = NewObject<URectLightComponent>(Chunk, NAME_None, RF_Transient);
                if (!Light) continue;
                Light->SetMobility(EComponentMobility::Movable);
                Light->SetIntensity(900.0f);
                Light->SetAttenuationRadius(850.0f);
                Light->SetSourceWidth(80.0f);
                Light->SetSourceHeight(80.0f);
                Light->AttachToComponent(Chunk->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
                Light->SetWorldTransform(WorldTransform);
                Light->RegisterComponent();
                continue;
            }
            if ((Socket.Kind != EBackroomsSocketKind::Trash && Socket.Kind != EBackroomsSocketKind::Furniture) || !Chunk->Database || Socket.Tag.IsNone()) continue;

            TArray<const FPropRule*> Candidates;
            for (const FPropRule& Rule : Chunk->Database->Props)
                if (!Rule.Mesh.IsNull() && HasRequiredTag(Rule, Socket.Tag)) Candidates.Add(&Rule);
            if (Candidates.IsEmpty()) continue;
            const FPropRule* Rule = Candidates[Stream.RandRange(0, Candidates.Num() - 1)];
            if (!Rule || Stream.FRand() > FMath::Clamp(Rule->Probability, 0.0f, 1.0f)) continue;
            UStaticMesh* PropMesh = Rule->Mesh.LoadSynchronous();
            if (!PropMesh) continue;
            UStaticMeshComponent* PropComponent = NewObject<UStaticMeshComponent>(Chunk, NAME_None, RF_Transient);
            if (!PropComponent) continue;
            const float ScaleX = FMath::Lerp(Rule->ScaleMin.X, Rule->ScaleMax.X, Stream.FRand());
            const float ScaleY = FMath::Lerp(Rule->ScaleMin.Y, Rule->ScaleMax.Y, Stream.FRand());
            WorldTransform.SetScale3D(WorldTransform.GetScale3D() * (0.5f * (ScaleX + ScaleY)));
            PropComponent->SetStaticMesh(PropMesh);
            PropComponent->SetMobility(EComponentMobility::Static);
            PropComponent->SetCollisionEnabled(Rule->bBlocking ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
            PropComponent->AttachToComponent(Chunk->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            PropComponent->SetWorldTransform(WorldTransform);
            PropComponent->RegisterComponent();
        }
    }
}
