// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpSpawnPoint.h"

APickUpSpawnPoint::APickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

}

void APickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnTimerPickup((AActor*)nullptr);
}

void APickUpSpawnPoint::SpawnPicup()
{
	int32 NumPickupClasses = PickUpClasses.Num();
	if(NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0,PickUpClasses.Num()-1);
		SpawnedPickup = GetWorld()->SpawnActor<APickups>(PickUpClasses[Selection],GetActorTransform());
	}

	if(HasAuthority()&&SpawnedPickup)
	{
		SpawnedPickup->OnDestroyed.AddDynamic(this,&APickUpSpawnPoint::StartSpawnTimerPickup);
	}
}

void APickUpSpawnPoint::SpawnPickupTimerFinished()
{
	SpawnPicup();
}

void APickUpSpawnPoint::StartSpawnTimerPickup(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickupTimeMin,SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&APickUpSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime);
}

void APickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

