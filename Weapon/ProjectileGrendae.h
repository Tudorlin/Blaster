// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Sound/SoundCue.h"
#include "ProjectileGrendae.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileGrendae : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileGrendae();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult,const FVector& ImpactVelocity);



private:
	UPROPERTY(EditAnywhere)
	USoundCue* BoundSound;
};
