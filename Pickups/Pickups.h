// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Pickups.generated.h"

UCLASS()
class BLASTER_API APickups : public AActor
{
	GENERATED_BODY()
	
public:	
	APickups();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual  void OnSphereOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* Othercomp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;
private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem * PickupEffect;

	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

public:	

};
