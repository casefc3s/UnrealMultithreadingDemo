// Copyright (c) 2026 Will Long

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MoverSpawnerWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class AMoverBenchmarkSpawner;

/**
 * 
 */
UCLASS()
class MULTITHREADINGDEMO_API UMoverSpawnerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void SetSpawner(AMoverBenchmarkSpawner* InSpawner);

	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void SpawnRequestedFromUI();

	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void ClearRequestedFromUI();

	UFUNCTION(BlueprintCallable, Category="Benchmark")
	void RandomizeRequestedFromUI();

	UFUNCTION(BlueprintPure, Category="Benchmark")
	int32 GetRequestedSpawnCount() const { return RequestedSpawnCount; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Benchmark")
	void OnSpawnerBound();

	UFUNCTION(BlueprintImplementableEvent, Category="Benchmark")
	void OnSpawnRequestSubmitted(int32 SubmittedCount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Benchmark", meta=(ClampMin="1"))
	int32 RequestedSpawnCount = 1000;

	UPROPERTY(BlueprintReadOnly, Category="Benchmark")
	TObjectPtr<AMoverBenchmarkSpawner> Spawner = nullptr;
};
