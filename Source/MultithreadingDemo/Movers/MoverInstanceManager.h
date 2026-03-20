// Copyright (c) 2026 Will Long

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoverInstanceManager.generated.h"

class USceneComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class MULTITHREADINGDEMO_API AMoverInstanceManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMoverInstanceManager();

public:
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category="Mover Instances")
	void ClearAllInstances();

	UFUNCTION(BlueprintCallable, Category="Mover Instances")
	bool AddMoverInstances(const TArray<FTransform>& WorldTransforms, TArray<int32>& OutInstanceIndices);

	UFUNCTION(BlueprintCallable, Category="Mover Instances")
	bool BatchUpdateMoverInstanceTransforms(int32 StartInstanceIndex, const TArray<FTransform>& NewWorldTransforms,
	                                        bool bMarkRenderStateDirty = true, bool bTeleport = true);

	UFUNCTION(BlueprintPure, Category="Mover Instances")
	int32 GetMoverInstanceCount() const;

	UFUNCTION(BlueprintPure, Category="Mover Instances")
	UHierarchicalInstancedStaticMeshComponent* GetHISM() const { return HISMComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ApplyConfiguredMeshSettings();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HISMComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<UStaticMesh> InstanceMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh")
	TArray<TObjectPtr<UMaterialInterface>> OverrideMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rendering")
	bool bCastShadow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rendering")
	bool bEnableCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rendering", meta=(ClampMin="0.0"))
	float InstanceStartCullDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rendering", meta=(ClampMin="0.0"))
	float InstanceEndCullDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rendering")
	bool bPreviewSingleInstanceInEditor = true;
};
