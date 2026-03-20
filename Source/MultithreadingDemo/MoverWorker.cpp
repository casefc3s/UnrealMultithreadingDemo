// Copyright (c) 2026 Will Long


#include "MoverWorker.h"

#include "Settings/MoverSettings.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

FMoverWorker::FMoverWorker() : WakeEvent(FPlatformProcess::GetSynchEventFromPool(false))
{
}

FMoverWorker::~FMoverWorker()
{
	Stop();

	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}

	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	if (WakeEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WakeEvent);
		WakeEvent = nullptr;
	}
}

bool FMoverWorker::Init()
{
	return true;
}

bool FMoverWorker::Start()
{
	if (Thread)
	{
		return false;
	}

	Thread = FRunnableThread::Create(this, TEXT("MoverWorker"));
	return Thread != nullptr;
}

void FMoverWorker::Enqueue(const FMoveBatchJob& Job)
{
	PendingJobs.Enqueue(Job);
	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}
}

bool FMoverWorker::DequeueResult(FMoveResult& OutResult)
{
	return CompletedResults.Dequeue(OutResult);
}

uint32 FMoverWorker::Run()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_Run);
	
	while (!bStopRequested)
	{
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_ProcessPending);
			ProcessPending();
		}

		if (!bStopRequested)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_Wait);
			WakeEvent->Wait();
		}
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_FinalFlush);
		ProcessPending();
	}
	
	return 0;
}

void FMoverWorker::Stop()
{
	bStopRequested = true;
	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}
}

void FMoverWorker::ProcessPending()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_ProcessPending);
	
	FMoveBatchJob Job;
	while (!bStopRequested && PendingJobs.Dequeue(Job))
	{
		switch (Job.Mode)
		{
			case EMoverBatchMode::FixedChunkParallelFor:
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_ProcessParallel);
				ProcessBatchParallelFor(Job);
				break;
			}
			case EMoverBatchMode::PerMover:
			case EMoverBatchMode::FixedChunk:
			case EMoverBatchMode::SmartAuto:
			default:
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_ProcessBatch);
				ProcessBatch(Job);
				break;
			}
		}
	}
}

void FMoverWorker::ProcessBatch(const FMoveBatchJob& Job)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_ProcessBatch);
	
	for (const FMoveElement& Element : Job.Entries)
	{
		FMoveResult Result;
		Result.Id = Element.Id;
		Result.FrameNumber = Job.FrameNumber;
		Result.Position = Element.Origin;

		Result.Position = DoMove(
			Element.Origin,
			Element.Destination,
			Job.DeltaTime,
			Element.MoveSpeed
		);

		CompletedResults.Enqueue(MoveTemp(Result));
	}
}

void FMoverWorker::ProcessBatchParallelFor(const FMoveBatchJob& Job)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_ProcessParallel);
	
	const UMoverSettings* Settings = GetDefault<UMoverSettings>();

	if (Job.Entries.Num() < Settings->ParallelForMinBatchSize)
	{
		ProcessBatch(Job);
		return;
	}

	TArray<FMoveResult> LocalResults;
	LocalResults.SetNum(Job.Entries.Num());

	const EParallelForFlags Flags = Settings->bForceSingleThreadForParallelFor
	                                ? EParallelForFlags::ForceSingleThread
	                                : EParallelForFlags::Unbalanced;

	ParallelFor(
		TEXT("MoverWorker_ProcessBatch"),
		Job.Entries.Num(),
		1,
		[&Job, &LocalResults](const int32 Index)
		{
			const FMoveElement& Item = Job.Entries[Index];
			FMoveResult& Result = LocalResults[Index];

			Result.Id = Item.Id;
			Result.FrameNumber = Job.FrameNumber;
			Result.Position = DoMove(
				Item.Origin,
				Item.Destination,
				Job.DeltaTime,
				Item.MoveSpeed
			);
		},
		Flags
	);

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoverWorker_EnqueueParallelResults);
		
		for (FMoveResult& Result : LocalResults)
		{
			CompletedResults.Enqueue(MoveTemp(Result));
		}
	}
}

FVector FMoverWorker::DoMove(const FVector& Origin, const FVector& Destination,
                             const float DeltaTime, const float MoveSpeed)
{
	const float DistanceSqr = FVector::DistSquared(Destination, Origin);
	const float MoveSqr = (MoveSpeed * DeltaTime) * (MoveSpeed * DeltaTime);
	if (DistanceSqr <= MoveSqr)
	{
		return Destination;
	}

	return FMath::VInterpConstantTo(Origin, Destination, DeltaTime, MoveSpeed);
}
