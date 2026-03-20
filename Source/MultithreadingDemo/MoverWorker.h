#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Containers/Queue.h"

UENUM(BlueprintType)
enum class EMoverBatchMode : uint8
{
	SmartAuto UMETA(DisplayName="Smart Auto"),
	PerMover UMETA(DisplayName="Per Mover"),
	FixedChunk UMETA(DisplayName="Fixed Chunk"),
	FixedChunkParallelFor UMETA(DisplayName="Fixed Chunk + ParallelFor")
};

struct FMoveElement
{
	int32 Id = INDEX_NONE;
	FVector Origin = FVector::ZeroVector;
	FVector Destination = FVector::ZeroVector;
	float MoveSpeed = 0.f;
};

struct FMoveBatchJob
{
	TArray<FMoveElement> Entries;
	uint64 FrameNumber = 0;
	float DeltaTime = 0.f;
	EMoverBatchMode Mode = EMoverBatchMode::SmartAuto;
};

struct FMoveResult
{
	int32 Id = INDEX_NONE;
	uint64 FrameNumber = 0;
	FVector Position = FVector::ZeroVector;
};

struct FMoverInstanceData
{
	int32 Id = INDEX_NONE;
	int32 InstanceIndex = INDEX_NONE;
	FVector Origin = FVector::ZeroVector;
	FVector Destination = FVector::ZeroVector;
	float MoveSpeed = 0.0f;
	bool bHasActiveJob = false;
	bool bInstanceActive = true;
};


class FMoverWorker final : public FRunnable
{
public:
	FMoverWorker();
	virtual ~FMoverWorker() override;

public:
	bool Start();
	void Enqueue(const FMoveBatchJob& Job);
	bool DequeueResult(FMoveResult& OutResult);
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	void ProcessPending();
	void ProcessBatch(const FMoveBatchJob& Job);
	void ProcessBatchParallelFor(const FMoveBatchJob& Job);

	static FVector DoMove(const FVector& Origin, const FVector& Destination, const float DeltaTime,
	                      const float MoveSpeed);

private:
	FRunnableThread* Thread = nullptr;
	FEvent* WakeEvent = nullptr;

	TQueue<FMoveBatchJob, EQueueMode::Mpsc> PendingJobs;
	TQueue<FMoveResult, EQueueMode::Spsc> CompletedResults;

	TAtomic<bool> bStopRequested = false;
};
