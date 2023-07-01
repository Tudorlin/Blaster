// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "RocketMovementComponent.h"
#include "NiagaraComponent.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	virtual void Destroyed() override;
protected:
	virtual void OnHit(UPrimitiveComponent* MyComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)               //蓝图上传的Soundcue
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;          //c++中用于控制Souncue的组件

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;//声音衰减

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;
private:
};
