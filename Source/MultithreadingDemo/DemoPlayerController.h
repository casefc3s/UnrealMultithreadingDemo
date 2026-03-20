// Copyright (c) 2026 Will Long

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DemoPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UMoverSpawnerWidget;
class AMoverBenchmarkSpawner;

/**
 * 
 */
UCLASS()
class MULTITHREADINGDEMO_API ADemoPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADemoPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleToggleSpawnerWidget();

	UFUNCTION()
	void HandleSpawnSmall();

	UFUNCTION()
	void HandleSpawnMedium();

	UFUNCTION()
	void HandleSpawnLarge();

	UFUNCTION()
	void HandleClearMovers();

private:
	void InitializeSpawnerWidget();
	void FindAndAssignSpawner();

protected:
	UPROPERTY(EditAnywhere, Category ="Input")
	TArray<UInputMappingContext*> InputMappingContexts;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> ToggleSpawnerWidgetAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SpawnSmallAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SpawnMediumAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SpawnLargeAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> ClearMoversAction;

	UPROPERTY(EditDefaultsOnly, Category="Benchmark")
	int32 SmallSpawnCount = 100;

	UPROPERTY(EditDefaultsOnly, Category="Benchmark")
	int32 MediumSpawnCount = 1000;

	UPROPERTY(EditDefaultsOnly, Category="Benchmark")
	int32 LargeSpawnCount = 10000;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UMoverSpawnerWidget> SpawnerWidgetClass;

	UPROPERTY()
	TObjectPtr<UMoverSpawnerWidget> SpawnerWidget;

	UPROPERTY()
	TObjectPtr<AMoverBenchmarkSpawner> CachedSpawner;
};
