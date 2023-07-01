// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "HealthPickups.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHealthPickups : public APickups
{
	GENERATED_BODY()

public:
	AHealthPickups();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* Othercomp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float HealAmout = 100.f;

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;
	
};
