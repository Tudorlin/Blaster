// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "ShieldPickups.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShieldPickups : public APickups
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* Othercomp,
		int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;             //护盾补充量

	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 5.f;
};
