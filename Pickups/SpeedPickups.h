// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "SpeedPickups.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ASpeedPickups : public APickups
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* Othercomp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float BaseBuffSpeed = 1600.f;

	UPROPERTY(EditAnywhere)
	float CrouchBuffSpeed = 850.f;

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 5.f;
};
