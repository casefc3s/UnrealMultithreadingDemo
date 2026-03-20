// Copyright (c) 2026 Will Long


#include "MoverSpawnerWidget.h"
#include "MultithreadingDemo/Spawning/MoverBenchmarkSpawner.h"

void UMoverSpawnerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMoverSpawnerWidget::SetSpawner(AMoverBenchmarkSpawner* InSpawner)
{
	Spawner = InSpawner;
	OnSpawnerBound();
}

void UMoverSpawnerWidget::SpawnRequestedFromUI()
{
	if (!IsValid(Spawner))
	{
		UE_LOG(LogTemp, Display, TEXT("MoverSpawnerWidget: No spawner assigned."));
		return;
	}

	const int32 SafeCount = FMath::Clamp(RequestedSpawnCount, 1, 10000);
	Spawner->RequestSpawn(SafeCount);
	OnSpawnRequestSubmitted(SafeCount);
}

void UMoverSpawnerWidget::ClearRequestedFromUI()
{
	if (IsValid(Spawner))
	{
		Spawner->ClearSpawnedMovers();
	}
}

void UMoverSpawnerWidget::RandomizeRequestedFromUI()
{
	if (IsValid(Spawner))
	{
		Spawner->RandomizeDestinations();
	}
}
