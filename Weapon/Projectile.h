// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Particles/ParticleSystemComponent.h"
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	
	//同服务器倒带一起用
	bool bShouldUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	UPROPERTY(EditAnywhere)
	float Damage=20.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;
	
protected:
	virtual void BeginPlay() override;

	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* MyComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	UBoxComponent *CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;         //要加了class子类才能名字不一样

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem *Tracer;

	UPROPERTY()
    class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime =3.f;
public:
	


};
