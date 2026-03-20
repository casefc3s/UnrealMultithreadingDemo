// Copyright (c) 2026 Will Long

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MoverWorldSubsystem.generated.h"

struct FMoverInstanceData;
enum class EMoverBatchMode : uint8;
class AMoverInstanceManager;
class FMoverWorker;
/**
 * 
 */
UCLASS()
class MULTITHREADINGDEMO_API UMoverWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Mover")
	void AddMovers(const TArray<FVector>& Origins, const TArray<FVector>& Destinations,
	               const TArray<float>& MoveSpeeds);

	UFUNCTION(BlueprintCallable, Category="Mover")
	void ClearMovers();

	UFUNCTION(BlueprintCallable, Category="Mover")
	void RandomizeDestinations(const FVector& DestinationBoxCenter, const FVector& DestinationBoxExtent,
	                           float MinMoveSpeed, float MaxMoveSpeed, int32 RandomSeed);

	UFUNCTION(BlueprintPure, Category="Mover")
	int32 GetMoverCount() const { return Movers.Num(); }

	UFUNCTION(BlueprintPure, Category="Mover")
	AMoverInstanceManager* GetInstanceManager() const { return InstanceManager.Get(); }

private:
	static EMoverBatchMode ResolveMode();
	static int32 ResolveChunkSize(int32 TotalMoverCount, EMoverBatchMode Mode);
	void ResolveInstanceManager();

	void CreateJobs(const float DeltaTime);
	void DrainResults();
	void ApplyTransformsToManager();

private:
	TArray<FMoverInstanceData> Movers;
	TWeakObjectPtr<AMoverInstanceManager> InstanceManager;
	TUniquePtr<FMoverWorker> Worker;
	uint64 FrameCount;
	bool bTransformsDirty;
};
