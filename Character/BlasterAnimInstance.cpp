// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BlasterCharacter==nullptr)
	{
		BlasterCharacter=Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if(BlasterCharacter==nullptr)   return;

	FVector Velocity=BlasterCharacter->GetVelocity();
	Velocity.Z=0.0f;
	Speed=Velocity.Size();      //获取速度

	bIsInAir=BlasterCharacter->GetCharacterMovement()->IsFalling();    //判断玩家是否正在下落
	bIsAccelerating=BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size()>0.0f? true : false;  //判断玩家是否正在加速
    bEquippedWeapon=BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon=BlasterCharacter->GetEquippedWeapon();
	bIsCrouched=BlasterCharacter->bIsCrouched;      //后者疑似为自带函数
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace=BlasterCharacter->GetTurningInPlace();
	bRotateRootBone=BlasterCharacter->ShouldPostponePathUpdates();
	bElimmed=BlasterCharacter->IsEmiled();

	FRotator AimRotation =BlasterCharacter->GetBaseAimRotation();    //获取Yaw的值
	FRotator MovementRotation =UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	YawOffset=UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,AimRotation).Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	// if(!BlasterCharacter->HasAuthority()&&!BlasterCharacter->IsLocallyControlled())
	//  {
	//     UE_LOG(LogTemp,Warning,TEXT("AimRotation Yaw %f: "),AimRotation.Yaw);
	//     UE_LOG(LogTemp,Warning,TEXT("MovementRotation Yaw %f: "),MovementRotation.Yaw);      //debug用
	//  }
	AO_Yaw=BlasterCharacter->GetAO_Yaw();
	AO_Pitch=BlasterCharacter->GetAO_Pitch();

	if(bEquippedWeapon&&EquippedWeapon&&EquippedWeapon->GetWeaponMesh()&&BlasterCharacter->GetMesh())
	{
		LeftHandTransform=EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"),ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"),LeftHandTransform.GetLocation(),FRotator::ZeroRotator,OutPosition,OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation=UKismetMathLibrary::FindLookAtRotation(
				RightHandTransform.GetLocation(),
				RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}
	}
	
	bUseFABRIK=BlasterCharacter->GetCombatState()==ECombtState::ECS_Unoccupied;
	bool bFABBRIKOverried = BlasterCharacter->IsLocallyControlled()
	&&BlasterCharacter->GetCombatState()!=ECombtState::ECS_ThrowingGrenade
	&&BlasterCharacter->bFinishedSwapping;
	if(bFABBRIKOverried)
	{
		bUseFABRIK = !BlasterCharacter->IsLocalReloading();
	}
	bUseAimOffsets=BlasterCharacter->GetCombatState()==ECombtState::ECS_Unoccupied&&!BlasterCharacter->GetDisableGameplay();
	bTransformRighthand=BlasterCharacter->GetCombatState()==ECombtState::ECS_Unoccupied&&!BlasterCharacter->GetDisableGameplay();
}

