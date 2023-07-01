// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;      //初始化函数,比如此工程中只需要初始化BlasterCharacter
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;  // 每帧更新动画阶段被调用的函数
private:
	UPROPERTY(BlueprintReadOnly,Category=Character,meta=(AllowPrivateAccess="true"))
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY(BlueprintReadOnly,Category=Movement,meta=(AllowPrivateAccess="true"));
	float Speed;

	UPROPERTY(BlueprintReadOnly,Category=Movement,meta=(AllowPrivateAccess="true"));
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly,Category=Movement,meta=(AllowPrivateAccess="true"));
	bool bIsAccelerating;
	//Meta = (EditCondition = “变量名”) 这里的变量名大多是布尔类型。用途是控制Detail面板上的变量能否修改。

	UPROPERTY(BlueprintReadOnly,Category=Movement,meta=(AllowPrivateAccess="true"));
	bool bEquippedWeapon;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly,Category=Movement,meta=(AllowPrivateAccess="true"));
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bElimmed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bTransformRighthand;
};
