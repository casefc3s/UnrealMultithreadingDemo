// Copyright (c) 2026 Will Long


#include "MoverInstanceManager.h"

#include "Components/SceneComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"


// Sets default values
AMoverInstanceManager::AMoverInstanceManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	HISMComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("MoverHISM"));
	HISMComponent->SetupAttachment(SceneRoot);

	HISMComponent->SetMobility(EComponentMobility::Movable);
	HISMComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	HISMComponent->SetGenerateOverlapEvents(false);
	HISMComponent->SetCanEverAffectNavigation(false);
	HISMComponent->CastShadow = bCastShadow;
	HISMComponent->bAutoRebuildTreeOnInstanceChanges = true;
}

// Called when the game starts or when spawned
void AMoverInstanceManager::BeginPlay()
{
	Super::BeginPlay();

	ApplyConfiguredMeshSettings();

#if WITH_EDITOR
	if (bPreviewSingleInstanceInEditor && HISMComponent->GetInstanceCount() == 1)
	{
		HISMComponent->ClearInstances();
	}
#endif
}

void AMoverInstanceManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyConfiguredMeshSettings();

#if WITH_EDITOR
	if (!GetWorld() || GetWorld()->IsGameWorld())
	{
		return;
	}

	if (bPreviewSingleInstanceInEditor && InstanceMesh)
	{
		if (HISMComponent->GetInstanceCount() == 0)
		{
			HISMComponent->AddInstance(FTransform::Identity, false);
		}
	}
	else
	{
		HISMComponent->ClearInstances();
	}
#endif
}

void AMoverInstanceManager::ApplyConfiguredMeshSettings()
{
	if (!HISMComponent)
	{
		return;
	}

	HISMComponent->SetStaticMesh(InstanceMesh);
	HISMComponent->SetCastShadow(bCastShadow);

	if (bEnableCollision)
	{
		HISMComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HISMComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		HISMComponent->SetGenerateOverlapEvents(true);
	}
	else
	{
		HISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HISMComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		HISMComponent->SetGenerateOverlapEvents(false);
	}

	HISMComponent->InstanceStartCullDistance = static_cast<int32>(InstanceStartCullDistance);
	HISMComponent->InstanceEndCullDistance = static_cast<int32>(InstanceEndCullDistance);

	for (int32 MaterialIndex = 0; MaterialIndex < OverrideMaterials.Num(); ++MaterialIndex)
	{
		if (OverrideMaterials[MaterialIndex])
		{
			HISMComponent->SetMaterial(MaterialIndex, OverrideMaterials[MaterialIndex]);
		}
	}
}

void AMoverInstanceManager::ClearAllInstances()
{
	if (HISMComponent)
	{
		HISMComponent->ClearInstances();
	}
}

bool AMoverInstanceManager::AddMoverInstances(const TArray<FTransform>& WorldTransforms,
                                              TArray<int32>& OutInstanceIndices)
{
	OutInstanceIndices.Reset();

	if (!HISMComponent || !InstanceMesh || WorldTransforms.IsEmpty())
	{
		return false;
	}

	const bool bAutoRebuildTemp = HISMComponent->bAutoRebuildTreeOnInstanceChanges;
	HISMComponent->bAutoRebuildTreeOnInstanceChanges = false;

	OutInstanceIndices = HISMComponent->AddInstances(WorldTransforms, true, false, true);

	HISMComponent->bAutoRebuildTreeOnInstanceChanges = bAutoRebuildTemp;

	// force tree rebuild and mark dirty, workaround for issue with needing to click out of viewport to render
	HISMComponent->BuildTreeIfOutdated(false, true);
	HISMComponent->MarkRenderStateDirty();

	return !OutInstanceIndices.IsEmpty();
}

bool AMoverInstanceManager::BatchUpdateMoverInstanceTransforms(int32 StartInstanceIndex,
                                                               const TArray<FTransform>& NewWorldTransforms,
                                                               bool bMarkRenderStateDirty, bool bTeleport)
{
	if (!HISMComponent || StartInstanceIndex < 0 || NewWorldTransforms.IsEmpty())
	{
		return false;
	}

	return HISMComponent->BatchUpdateInstancesTransforms(StartInstanceIndex, NewWorldTransforms, true,
	                                                     bMarkRenderStateDirty, bTeleport);
}

int32 AMoverInstanceManager::GetMoverInstanceCount() const
{
	return HISMComponent ? HISMComponent->GetInstanceCount() : 0;
}
