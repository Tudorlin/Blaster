// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UBuffComponent::Heal(float HealAmout, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmout/HealingTime;
	AmoutToHeal += HealAmout;
}


void UBuffComponent::HealRampUp(float DeltaSecond)
{
	if(!bHealing||Character==nullptr||Character->IsEmiled()) return;

	const float HealthisFram = HealingRate*DeltaSecond;
	Character->SetHealth(FMath::Clamp(Character->GetHeath()+HealthisFram,0.f,Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmoutToHeal -= HealthisFram;

	if(AmoutToHeal <=0||Character->GetHeath()>=Character->GetMaxHealth())
	{
		bHealing = false;
		AmoutToHeal = 0;
	}
}

void UBuffComponent::ReplenishShield(float ReplenishAmout, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ReplenishAmout/ReplenishTime;
	ShieldReplenishAmout += ReplenishAmout;
}

void UBuffComponent::ReplenishRampUp(float DeltaTime)
{
	if(!bReplenishingShield||Character==nullptr||Character->IsEmiled()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmout -= ReplenishThisFrame;

	if (ShieldReplenishAmout <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmout = 0.f;
	}
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)             //在Character中初始化
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if(Character == nullptr)  return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeed,
		BuffTime);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}

	MulticastBuffSpeed(BuffBaseSpeed,BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeed()
{
	if(Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;

	MulticastBuffSpeed(InitialBaseSpeed,InitialCrouchSpeed);
}

void UBuffComponent::MulticastBuffSpeed_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if(Character&&Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::JumpBuff(float JumpBuffZVelocity, float BuffTime)
{
	if(Character==nullptr)  return;


	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJumpVelocity,
		BuffTime);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpBuffZVelocity;
	}

	MulticastBuffJump(JumpBuffZVelocity);
}

void UBuffComponent::ResetJumpVelocity()
{
	if(Character&&Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}

	MulticastBuffJump(InitialJumpVelocity);
}

void UBuffComponent::MulticastBuffJump_Implementation(float JumpVelocity)
{
	if(Character&&Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ReplenishRampUp(DeltaTime);
}

