// Copyright (c) 2026 Will Long


#include "MoverWorldSubsystem.h"

#include "MultithreadingDemo/MoverWorker.h"
#include "MultithreadingDemo/Movers/MoverInstanceManager.h"
#include "MultithreadingDemo/Settings/MoverSettings.h"

#include "EngineUtils.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(Mover_Count, TEXT("Mover/Registered Count"));
TRACE_DECLARE_INT_COUNTER(Mover_Active_Count, TEXT("Mover/Active Count"));
TRACE_DECLARE_INT_COUNTER(Mover_Snapshot_Count, TEXT("Mover/Snapshot Count"));
TRACE_DECLARE_INT_COUNTER(Mover_Batch_Count, TEXT("Mover/Batch Count"));
TRACE_DECLARE_INT_COUNTER(Mover_Result_Count, TEXT("Mover/Result Count"));
TRACE_DECLARE_INT_COUNTER(Mover_Chunk_Size, TEXT("Mover/Chunk Size"));

void UMoverWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Worker = MakeUnique<FMoverWorker>();
	check(Worker->Start());

	ResolveInstanceManager();
}

void UMoverWorldSubsystem::Deinitialize()
{
	if (Worker.IsValid())
	{
		Worker.Reset();
	}

	if (InstanceManager.IsValid())
	{
		InstanceManager.Reset();
	}

	Movers.Reset();

	Super::Deinitialize();
}

void UMoverWorldSubsystem::AddMovers(const TArray<FVector>& Origins, const TArray<FVector>& Destinations,
                                     const TArray<float>& MoveSpeeds)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_AddMovers);

	ResolveInstanceManager();

	if (!InstanceManager.IsValid())
	{
		return;
	}

	const int32 Count = Origins.Num();
	if (Count <= 0 || Destinations.Num() != Count || MoveSpeeds.Num() != Count)
	{
		return;
	}

	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Reserve(Count);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		InstanceTransforms.Add(FTransform(FRotator::ZeroRotator, Origins[Index]));
	}

	TArray<int32> InstanceIndices;
	if (!InstanceManager->AddMoverInstances(InstanceTransforms, InstanceIndices) || InstanceIndices.Num() != Count)
	{
		return;
	}

	const int32 StartingMoverId = Movers.Num();
	Movers.Reserve(StartingMoverId + Count);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		FMoverInstanceData& Mover = Movers.AddDefaulted_GetRef();
		Mover.Id = StartingMoverId + Index;
		Mover.InstanceIndex = InstanceIndices[Index];
		Mover.Origin = Origins[Index];
		Mover.Destination = Destinations[Index];
		Mover.MoveSpeed = MoveSpeeds[Index];
		Mover.bHasActiveJob = false;
		Mover.bInstanceActive = true;
	}

	TRACE_COUNTER_SET(Mover_Count, Movers.Num());
	bTransformsDirty = true;
}

void UMoverWorldSubsystem::ClearMovers()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_ClearMovers);

	Movers.Reset();

	if (InstanceManager.IsValid())
	{
		InstanceManager->ClearAllInstances();
	}

	TRACE_COUNTER_SET(Mover_Count, 0);
	TRACE_COUNTER_SET(Mover_Active_Count, 0);
	TRACE_COUNTER_SET(Mover_Batch_Count, 0);
	TRACE_COUNTER_SET(Mover_Result_Count, 0);

	bTransformsDirty = false;
}

void UMoverWorldSubsystem::RandomizeDestinations(const FVector& DestinationBoxCenter,
                                                 const FVector& DestinationBoxExtent, const float MinMoveSpeed,
                                                 const float MaxMoveSpeed, const int32 RandomSeed)
{
	const FRandomStream Random(RandomSeed);

	for (FMoverInstanceData& Mover : Movers)
	{
		Mover.Destination = FVector(
			Random.FRandRange(DestinationBoxCenter.X - DestinationBoxExtent.X,
			                  DestinationBoxCenter.X + DestinationBoxExtent.X),
			Random.FRandRange(DestinationBoxCenter.Y - DestinationBoxExtent.Y,
			                  DestinationBoxCenter.Y + DestinationBoxExtent.Y),
			Random.FRandRange(DestinationBoxCenter.Z - DestinationBoxExtent.Z,
			                  DestinationBoxCenter.Z + DestinationBoxExtent.Z));

		Mover.MoveSpeed = Random.FRandRange(MinMoveSpeed, MaxMoveSpeed);
	}
}

void UMoverWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_Tick);

	++FrameCount;

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_DrainResults);
		DrainResults();
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_ApplyTransforms);
		ApplyTransformsToManager();
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_CreateJobs);
		CreateJobs(DeltaTime);
	}
}

TStatId UMoverWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMoverWorldSubsystem, STATGROUP_Tickables);
}

EMoverBatchMode UMoverWorldSubsystem::ResolveMode()
{
	const UMoverSettings* Settings = GetDefault<UMoverSettings>();

	switch (Settings->ActiveMode)
	{
	case EMoverBatchMode::PerMover:
		return Settings->bAllowPerMover ? EMoverBatchMode::PerMover : EMoverBatchMode::SmartAuto;
	case EMoverBatchMode::FixedChunk:
		return Settings->bAllowFixedChunk ? EMoverBatchMode::FixedChunk : EMoverBatchMode::SmartAuto;
	case EMoverBatchMode::FixedChunkParallelFor:
		return Settings->bAllowFixedChunkParallelFor
			       ? EMoverBatchMode::FixedChunkParallelFor
			       : EMoverBatchMode::SmartAuto;
	case EMoverBatchMode::SmartAuto:
	default:
		return Settings->bAllowSmartAuto ? EMoverBatchMode::SmartAuto : EMoverBatchMode::PerMover;
	}
}

int32 UMoverWorldSubsystem::ResolveChunkSize(const int32 TotalMoverCount, const EMoverBatchMode Mode)
{
	const UMoverSettings* Settings = GetDefault<UMoverSettings>();

	if (Mode == EMoverBatchMode::PerMover)
	{
		return 1;
	}

	if (Mode == EMoverBatchMode::FixedChunk || Mode == EMoverBatchMode::FixedChunkParallelFor)
	{
		return FMath::Max(1, Settings->FixedChunkSize);
	}

	// SmartAuto
	if (TotalMoverCount <= Settings->SmartSmallCountThreshold)
	{
		return FMath::Max(1, Settings->SmartSmallChunkSize);
	}

	if (TotalMoverCount <= Settings->SmartMediumCountThreshold)
	{
		return FMath::Max(1, Settings->SmartMediumChunkSize);
	}

	return FMath::Max(1, Settings->SmartLargeChunkSize);
}

void UMoverWorldSubsystem::ResolveInstanceManager()
{
	if (!GetWorld() || InstanceManager.IsValid())
	{
		return;
	}

	for (TActorIterator<AMoverInstanceManager> It(GetWorld()); It; ++It)
	{
		InstanceManager = *It;
		break;
	}
}

void UMoverWorldSubsystem::CreateJobs(const float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_CreateJobs);

	if (!Worker.IsValid() || Movers.IsEmpty())
	{
		return;
	}

	TArray<FMoveElement> SnapshotElements;
	int32 ActiveCount = 0;

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_BuildSnapshot);
		SnapshotElements.Reserve(Movers.Num());

		for (FMoverInstanceData& Mover : Movers)
		{
			if (!Mover.bInstanceActive)
			{
				continue;
			}

			if (Mover.bHasActiveJob)
			{
				++ActiveCount;
				continue;
			}

			FMoveElement& Entry = SnapshotElements.AddDefaulted_GetRef();
			Entry.Id = Mover.Id;
			Entry.Origin = Mover.Origin;
			Entry.Destination = Mover.Destination;
			Entry.MoveSpeed = Mover.MoveSpeed;
		}
	}

	TRACE_COUNTER_SET(Mover_Snapshot_Count, SnapshotElements.Num());

	if (SnapshotElements.IsEmpty())
	{
		TRACE_COUNTER_SET(Mover_Batch_Count, 0);
		TRACE_COUNTER_SET(Mover_Active_Count, ActiveCount);
		return;
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_EnqueueBatches);
		const EMoverBatchMode Mode = ResolveMode();
		const int32 ChunkSize = ResolveChunkSize(SnapshotElements.Num(), Mode);

		TRACE_COUNTER_SET(Mover_Chunk_Size, ChunkSize);

		int32 BatchCount = 0;
		int32 StartIndex = 0;

		while (StartIndex < SnapshotElements.Num())
		{
			const int32 Count = FMath::Min(ChunkSize, SnapshotElements.Num() - StartIndex);

			FMoveBatchJob Job;
			Job.DeltaTime = DeltaTime;
			Job.FrameNumber = FrameCount;
			Job.Mode = Mode == EMoverBatchMode::SmartAuto
				           ? (Count >= GetDefault<UMoverSettings>()->ParallelForMinBatchSize
					              ? EMoverBatchMode::FixedChunkParallelFor
					              : EMoverBatchMode::FixedChunk)
				           : Mode;

			Job.Entries.Reserve(Count);

			for (int32 LocalIndex = 0; LocalIndex < Count; ++LocalIndex)
			{
				const FMoveElement& Entry = SnapshotElements[StartIndex + LocalIndex];
				Job.Entries.Add(Entry);

				if (Movers.IsValidIndex(Entry.Id))
				{
					Movers[Entry.Id].bHasActiveJob = true;
					++ActiveCount;
				}
			}

			Worker->Enqueue(Job);

			++BatchCount;
			StartIndex += Count;
		}

		TRACE_COUNTER_SET(Mover_Batch_Count, BatchCount);
		TRACE_COUNTER_SET(Mover_Active_Count, ActiveCount);
	}
}

void UMoverWorldSubsystem::DrainResults()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_DrainResults);

	if (!Worker.IsValid())
	{
		return;
	}

	int32 ResultsApplied = 0;
	int32 ActiveCount = 0;

	FMoveResult Result;
	while (Worker->DequeueResult(Result))
	{
		if (!Movers.IsValidIndex(Result.Id))
		{
			continue;
		}

		FMoverInstanceData& Mover = Movers[Result.Id];
		if (!Mover.bInstanceActive)
		{
			continue;
		}

		Mover.Origin = Result.Position;
		Mover.bHasActiveJob = false;
		++ResultsApplied;
		bTransformsDirty = true;
	}

	for (const FMoverInstanceData& Mover : Movers)
	{
		if (Mover.bHasActiveJob)
		{
			++ActiveCount;
		}
	}

	TRACE_COUNTER_SET(Mover_Result_Count, ResultsApplied);
	TRACE_COUNTER_SET(Mover_Active_Count, ActiveCount);
}

void UMoverWorldSubsystem::ApplyTransformsToManager()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoverSubsystem_ApplyTransforms);

	if (!bTransformsDirty || !InstanceManager.IsValid() || Movers.IsEmpty())
	{
		return;
	}

	// assumes mover append order == HISM add order and Movers only clear/reset globally to keep indices stable
	TArray<FTransform> Transforms;
	Transforms.Reserve(Movers.Num());

	for (const FMoverInstanceData& Mover : Movers)
	{
		if (!Mover.bInstanceActive)
		{
			continue;
		}

		Transforms.Add(FTransform(FRotator::ZeroRotator, Mover.Origin));
	}

	if (!Transforms.IsEmpty())
	{
		InstanceManager->BatchUpdateMoverInstanceTransforms(0, Transforms, true, true);
	}

	bTransformsDirty = false;
}
