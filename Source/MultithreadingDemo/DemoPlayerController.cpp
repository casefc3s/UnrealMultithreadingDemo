// Copyright (c) 2026 Will Long


#include "DemoPlayerController.h"

#include "MultithreadingDemo/Spawning/MoverBenchmarkSpawner.h"
#include "MultithreadingDemo/UI/MoverSpawnerWidget.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"

ADemoPlayerController::ADemoPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ADemoPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeSpawnerWidget();
	FindAndAssignSpawner();
}

void ADemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		return;
	}
	
	for (UInputMappingContext* Context : InputMappingContexts)
	{
		Subsystem->AddMappingContext(Context, 0);
	}
	
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (ToggleSpawnerWidgetAction)
	{
		EnhancedInput->BindAction(
			ToggleSpawnerWidgetAction,
			ETriggerEvent::Started,
			this,
			&ADemoPlayerController::HandleToggleSpawnerWidget);
	}

	if (SpawnSmallAction)
	{
		EnhancedInput->BindAction(
			SpawnSmallAction,
			ETriggerEvent::Started,
			this,
			&ADemoPlayerController::HandleSpawnSmall);
	}

	if (SpawnMediumAction)
	{
		EnhancedInput->BindAction(
			SpawnMediumAction,
			ETriggerEvent::Started,
			this,
			&ADemoPlayerController::HandleSpawnMedium);
	}

	if (SpawnLargeAction)
	{
		EnhancedInput->BindAction(
			SpawnLargeAction,
			ETriggerEvent::Started,
			this,
			&ADemoPlayerController::HandleSpawnLarge);
	}

	if (ClearMoversAction)
	{
		EnhancedInput->BindAction(
			ClearMoversAction,
			ETriggerEvent::Started,
			this,
			&ADemoPlayerController::HandleClearMovers);
	}
}

void ADemoPlayerController::HandleToggleSpawnerWidget()
{
    if (!SpawnerWidget)
    {
        return;
    }

    bShowMouseCursor = !bShowMouseCursor;
	if (!bShowMouseCursor)
	{
		SetInputMode(FInputModeGameOnly());
	}
    else
    {
	    SetInputMode(FInputModeGameAndUI());
    }
}

void ADemoPlayerController::HandleSpawnSmall()
{
    if (CachedSpawner)
    {
        CachedSpawner->RequestSpawn(SmallSpawnCount);
    }
}

void ADemoPlayerController::HandleSpawnMedium()
{
    if (CachedSpawner)
    {
        CachedSpawner->RequestSpawn(MediumSpawnCount);
    }
}

void ADemoPlayerController::HandleSpawnLarge()
{
    if (CachedSpawner)
    {
        CachedSpawner->RequestSpawn(LargeSpawnCount);
    }
}

void ADemoPlayerController::HandleClearMovers()
{
    if (CachedSpawner)
    {
        CachedSpawner->ClearSpawnedMovers();
    }
}

void ADemoPlayerController::InitializeSpawnerWidget()
{
    if (!SpawnerWidgetClass)
    {
    	UE_LOG(LogTemp, Warning, TEXT("DemoPlayerController: No SpawnerWidgetClass assigned"));
        return;
    }

    SpawnerWidget = CreateWidget<UMoverSpawnerWidget>(this, SpawnerWidgetClass);
    if (!SpawnerWidget)
    {
        return;
    }

    SpawnerWidget->AddToViewport();

    bShowMouseCursor = true;
    SetInputMode(FInputModeGameAndUI());
}

void ADemoPlayerController::FindAndAssignSpawner()
{
    if (!GetWorld())
    {
        return;
    }

    if (!CachedSpawner || !IsValid(CachedSpawner))
    {
        for (TActorIterator<AMoverBenchmarkSpawner> It(GetWorld()); It; ++It)
        {
            CachedSpawner = *It;
            break;
        }
    }

    if (SpawnerWidget && CachedSpawner)
    {
        SpawnerWidget->SetSpawner(CachedSpawner);
    }
}
