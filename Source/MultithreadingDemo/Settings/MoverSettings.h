// Copyright (c) 2026 Will Long

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MultithreadingDemo/MoverWorker.h"
#include "MoverSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Mover Threading Settings"))
class MULTITHREADINGDEMO_API UMoverSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UMoverSettings();

	virtual FName GetCategoryName() const override
	{
		return TEXT("Game");
	}

public:
	UPROPERTY(EditAnywhere, Config, Category="Modes")
	bool bAllowPerMover = true;

	UPROPERTY(EditAnywhere, Config, Category="Modes")
	bool bAllowFixedChunk = true;

	UPROPERTY(EditAnywhere, Config, Category="Modes")
	bool bAllowFixedChunkParallelFor = true;

	UPROPERTY(EditAnywhere, Config, Category="Modes")
	bool bAllowSmartAuto = true;

	UPROPERTY(EditAnywhere, Config, Category="Modes")
	EMoverBatchMode ActiveMode = EMoverBatchMode::SmartAuto;

	UPROPERTY(EditAnywhere, Config, Category="Chunking", meta=(ClampMin="1"))
	int32 FixedChunkSize = 256;

	UPROPERTY(EditAnywhere, Config, Category="Chunking", meta=(ClampMin="1"))
	int32 SmartSmallCountThreshold = 256;

	UPROPERTY(EditAnywhere, Config, Category="Chunking", meta=(ClampMin="1"))
	int32 SmartMediumCountThreshold = 2048;

	UPROPERTY(EditAnywhere, Config, Category="Chunking", meta=(ClampMin="1"))
	int32 SmartLargeChunkSize = 512;

	UPROPERTY(EditAnywhere, Config, Category="Chunking", meta=(ClampMin="1"))
	int32 SmartMediumChunkSize = 256;

	UPROPERTY(EditAnywhere, Config, Category="Chunking", meta=(ClampMin="1"))
	int32 SmartSmallChunkSize = 64;

	UPROPERTY(EditAnywhere, Config, Category="ParallelFor", meta=(ClampMin="1"))
	int32 ParallelForMinBatchSize = 256;

	UPROPERTY(EditAnywhere, Config, Category="ParallelFor")
	bool bForceSingleThreadForParallelFor = false;
};
