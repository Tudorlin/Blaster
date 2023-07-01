// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;
	void Heal(float HealAmout, float HealingTime);
	void ReplenishShield(float ReplenishAmout, float ReplenishTime);
	
	void BuffSpeed(float BuffBaseSpeed,float BuffCrouchSpeed,float BuffTime);
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);

	void JumpBuff(float JumpBuffZVelocity , float BuffTime);
	void SetInitialJumpVelocity(float Velocity);

protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaSecond);
	void ReplenishRampUp(float DeltaTime);

	FTimerHandle SpeedBuffTimer;                                         //速度buff

	void ResetSpeed();                                              
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	UFUNCTION(NetMulticast,Reliable)
    void MulticastBuffSpeed(float BaseSpeed,float CrouchSpeed);

	FTimerHandle JumpBuffTimer;                              //跳跃buff
	
	void ResetJumpVelocity();
	float InitialJumpVelocity;
	UFUNCTION(NetMulticast,Reliable)
	void MulticastBuffJump(float JumpVelocity);

	

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	//生命回复
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmoutToHeal = 0.f;

	//护盾回复
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmout = 0.f;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
