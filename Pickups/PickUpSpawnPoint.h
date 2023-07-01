// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "GameFramework/Actor.h"
#include "PickUpSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickUpSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickUpSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickups>> PickUpClasses;

	UPROPERTY()
	APickups *SpawnedPickup;

	void SpawnPicup();

	void SpawnPickupTimerFinished();

	UFUNCTION()
	void StartSpawnTimerPickup(AActor *DestroyedActor);

private:
	FTimerHandle SpawnTimerHandle;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax;
public:	

};
