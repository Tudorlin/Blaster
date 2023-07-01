// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponType.h"
#include "Components/ActorComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/Weapon/Projectile.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapon();
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishedRelod();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();
	
	UFUNCTION(BlueprintCallable)
	void FinsihSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
    void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType , int32 AmmoAmount);

	bool bLocallyReloading = false;

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget , float FireDelay);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets,float FireDelay);

	UFUNCTION(NetMulticast,Reliable)
	void MuticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);              //射线检测

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server,Reliable)
	void ServerReload();

	void HandleRelod();

	int32 AmountToReload();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	void ThrowGrenade();

	UFUNCTION(Server,Reliable)
	void ServerThrowGrenade();

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);
	void EquipPrimaryWeapon(AWeapon *WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing=OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimingButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	float CrosshairVelocityFctor;     //移动时的准心散布参数
	float CrosshairInAirFctor;        //在空中的准心散布参数
	float CrosshairAimFctor;
	float CrosshairshootingFctor;

	FVector HitTarget;

	FHUDPackage HUDPackage;
	
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	FTimerHandle FireTimer;
	
	bool bCanFire=true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	TMap<EWeaponType,int32>CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingAmmo = 30;              //初始弹药

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 SubmachineGunStartingAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 SniperRifleStartingAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 ShotGunStartingAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 GranadeStartingAmmo = 0;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombtState CombatState=ECombtState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

	void UpdateShotgunValues();

	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades(); 
public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool bShouldSwapWeapon();
};
 