
#pragma once

#pragma once
#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterfaces.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Blaster/Weapon/Weapon.h"
#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter,public IInteractWithCrosshairsInterfaces
{
	GENERATED_BODY()
public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;

	//蒙太奇播放 
	void PlayFireMontage(bool bAiming);
	void PlayRloadMontage();
	void PlayElimMontage();
	void playThrowGrenadeMontage();
	void PlayHitReactMontage();
	void PlaySwapMontage();

	
	void Elim(bool bPLayerLeftGame);
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void CalculateAO_Pitch();
	void AimOffest(float Deltime);
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void GrenadeButtonPressed();
	
	void DropOrDestroyWeapon(AWeapon *Weapon);
	void DropOrDestroyWeapons();

	UFUNCTION()
    void ReceiveDamge(AActor* DamageActor,float Damage,const UDamageType* DamageTupe,class AController* InstigatorCotroller,AActor* DamageCause);
	void PollInit();           //初始化得分

	void RotateInPlace(float DeltaTime);

	//Hit Box

	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess="true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;
	
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;                 //插值
	float AO_Pitch;
	FRotator StartingAimRotation;
 
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	//动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	void HideCameraIfCharacterClose();
	
	UPROPERTY(EditAnywhere)
	float CameraRhreshold=200.f;
	
	bool bRotateRootBone;
	float TurnThreshold=0.5f;
	FRotator ProxyRotationLastFram;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/**
	 *生命值     ----玩家状态
	 */
	UPROPERTY(EditAnywhere,Category="Player State")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Health,EditAnywhere,Category="Player State")
	float Health=100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
		 *护盾值     ----玩家状态
		 */
	
	UPROPERTY(EditAnywhere,Category="Player State")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Shield,EditAnywhere,Category="Player State")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed=false;

	FTimerHandle ElimTimer;                          
 
	UPROPERTY(EditDefaultsOnly)           
	float ElimDely=3.f;                 //重生时间

	void ElimTimerFinished();

	bool bLeftGame = false;      //判断玩家是否退出游戏

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;     //曲线
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	//运行时更改的动态实例
	UPROPERTY(VisibleAnywhere,Category=Elim)           //只可见
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//蓝图中设置的动态实例
	UPROPERTY(VisibleAnywhere,Category=Elim)
	UMaterialInstance* DissolveMaterialInstance;


	/*Team Color*/
	UPROPERTY(EditAnywhere ,Category = Elim)
	UMaterialInstance* RedTeamColor;

	UPROPERTY(EditAnywhere , Category=Elim)
	UMaterialInstance* RedTeamDissolveMaterialInstance;

	UPROPERTY(EditAnywhere , Category=Elim)
	UMaterialInstance* BlueTeamColor;

	UPROPERTY(EditAnywhere , Category=Elim)
	UMaterialInstance* BlueTeamDissolveMaterialInstance;

	UPROPERTY(EditAnywhere , Category= Elim)
	UMaterialInstance* OriginalTeamColor;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;

	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	UPROPERTY(VisibleAnywhere)            //手雷
	UStaticMeshComponent* AttachedGrenade;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	UPROPERTY(Replicated)
	bool bDisableGameplay =false;                    //准备阶段禁止除视角转动以外的输入

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();

	void UpdateHUDShield();

	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast,Reliable)
	void MulticastLostTheLead();
    void SetTeamColor(ETeam Team);
	
	UFUNCTION(Server,Reliable)
	void ServerLeaveGame();

	UPROPERTY()
	TMap<FName,class UBoxComponent*> HitBoxCollisionBoxes;     //客户端启用SSR射击命中闪退不是不加宏的原因

	bool bFinishedSwapping = false;
	
	
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }    //内联函数：为了在频繁调用的情况下减少函数调用的开销
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEmiled() const { return bElimmed; }
	FORCEINLINE float GetHeath() const { return Health; }
	FORCEINLINE void SetHealth(float Amout) { Health = Amout; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amout) { Shield = Amout;  }
	FORCEINLINE float GetMaxShield() const { return MaxShield; };
	ECombtState GetCombatState() const;
	FORCEINLINE UCombatComponent *GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE bool GetbElimed() const { return bElimmed; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE ULagCompensationComponent *GetLagCompensation() const { return LagCompensation; }

	bool IsLocalReloading();
};

