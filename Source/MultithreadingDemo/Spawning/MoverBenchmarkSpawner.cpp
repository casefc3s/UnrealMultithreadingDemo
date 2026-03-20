// Copyright (c) 2026 Will Long


#include "MoverBenchmarkSpawner.h"

#include "MoverWorldSubsystem.h"

// Sets default values
AMoverBenchmarkSpawner::AMoverBenchmarkSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMoverBenchmarkSpawner::BeginPlay()
{
	Super::BeginPlay();

	RandomStream.Initialize(RandomSeed);

	if (bAutoSpawnOnBeginPlay)
	{
		RequestSpawn(DefaultSpawnCount);
	}
}

// Called every frame
void AMoverBenchmarkSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PendingSpawnCount <= 0)
	{
		return;
	}

	const int32 NumToSpawn = bSpawnOverMultipleFrames
		                         ? FMath::Min(SpawnPerTick, PendingSpawnCount)
		                         : PendingSpawnCount;

	SpawnBatch(NumToSpawn);
	PendingSpawnCount -= NumToSpawn;
}

void AMoverBenchmarkSpawner::RequestSpawn(int32 InSpawnCount)
{
	if (InSpawnCount <= 0)
	{
		return;
	}

	PendingSpawnCount += InSpawnCount;
}

void AMoverBenchmarkSpawner::ClearSpawnedMovers()
{
	if (UWorld* World = GetWorld())
	{
		if (UMoverWorldSubsystem* Subsystem = World->GetSubsystem<UMoverWorldSubsystem>())
		{
			Subsystem->ClearMovers();
		}
	}

	PendingSpawnCount = 0;
}

void AMoverBenchmarkSpawner::RandomizeDestinations()
{
	if (UWorld* World = GetWorld())
	{
		if (UMoverWorldSubsystem* Subsystem = World->GetSubsystem<UMoverWorldSubsystem>())
		{
			Subsystem->RandomizeDestinations(DestinationBoxCenter, DestinationBoxExtent, MinMoveSpeed, MaxMoveSpeed,
			                                 RandomSeed++);
		}
	}
}

FVector AMoverBenchmarkSpawner::GetRandomSpawnLocation() const
{
	return FVector(
		RandomStream.FRandRange(SpawnBoxCenter.X - SpawnBoxExtent.X, SpawnBoxCenter.X + SpawnBoxExtent.X),
		RandomStream.FRandRange(SpawnBoxCenter.Y - SpawnBoxExtent.Y, SpawnBoxCenter.Y + SpawnBoxExtent.Y),
		RandomStream.FRandRange(SpawnBoxCenter.Z - SpawnBoxExtent.Z, SpawnBoxCenter.Z + SpawnBoxExtent.Z)
	);
}

FVector AMoverBenchmarkSpawner::GetRandomDestination() const
{
	return FVector(
		RandomStream.FRandRange(DestinationBoxCenter.X - DestinationBoxExtent.X,
		                        DestinationBoxCenter.X + DestinationBoxExtent.X),
		RandomStream.FRandRange(DestinationBoxCenter.Y - DestinationBoxExtent.Y,
		                        DestinationBoxCenter.Y + DestinationBoxExtent.Y),
		RandomStream.FRandRange(DestinationBoxCenter.Z - DestinationBoxExtent.Z,
		                        DestinationBoxCenter.Z + DestinationBoxExtent.Z)
	);
}

float AMoverBenchmarkSpawner::GetRandomMoveSpeed() const
{
	return RandomStream.FRandRange(MinMoveSpeed, MaxMoveSpeed);
}

void AMoverBenchmarkSpawner::SpawnBatch(const int32 SpawnCount)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UMoverWorldSubsystem* Subsystem = World->GetSubsystem<UMoverWorldSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	TArray<FVector> Origins;
	TArray<FVector> Destinations;
	TArray<float> MoveSpeeds;

	Origins.Reserve(SpawnCount);
	Destinations.Reserve(SpawnCount);
	MoveSpeeds.Reserve(SpawnCount);

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		Origins.Add(GetRandomSpawnLocation());
		Destinations.Add(GetRandomDestination());
		MoveSpeeds.Add(GetRandomMoveSpeed());
	}

	Subsystem->AddMovers(Origins, Destinations, MoveSpeeds);
}
