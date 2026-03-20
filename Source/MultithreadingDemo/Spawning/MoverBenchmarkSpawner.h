// Copyright (c) 2026 Will Long

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoverBenchmarkSpawner.generated.h"

UCLASS()
class MULTITHREADINGDEMO_API AMoverBenchmarkSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMoverBenchmarkSpawner();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void RequestSpawn(int32 InSpawnCount);

	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void ClearSpawnedMovers();

	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void RandomizeDestinations();

	UFUNCTION(BlueprintPure, Category="Benchmark")
	int32 GetPendingSpawnCount() const { return PendingSpawnCount; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FVector GetRandomSpawnLocation() const;
	FVector GetRandomDestination() const;
	float GetRandomMoveSpeed() const;
	void SpawnBatch(int32 SpawnCount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark", meta=(ClampMin="1"))
	int32 DefaultSpawnCount = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark", meta=(ClampMin="1"))
	int32 SpawnPerTick = 250;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark")
	bool bSpawnOverMultipleFrames = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark")
	bool bAutoSpawnOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark")
	FVector SpawnBoxCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark")
	FVector SpawnBoxExtent = FVector(5000.f, 5000.f, 200.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark")
	FVector DestinationBoxCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark")
	FVector DestinationBoxExtent = FVector(5000.f, 5000.f, 200.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark", meta=(ClampMin="1.0"))
	float MinMoveSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark", meta=(ClampMin="1.0"))
	float MaxMoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Benchmark")
	int32 RandomSeed = 42;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Benchmark")
	int32 PendingSpawnCount;

	mutable FRandomStream RandomStream;
};
