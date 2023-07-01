// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups.h"
#include "JumpBuffPickups.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AJumpBuffPickups : public APickups
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
	float JumpZVelocitySpeed = 4000.f;

	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};
