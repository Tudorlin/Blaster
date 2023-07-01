// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);

	void Dropped();

	void AddAmmo(int32 AmmoToAdd);

	FVector TraceEndWithScatter(const FVector & HitTarget);     //霰弹弹道散布

	UPROPERTY(EditAnywhere,Category=Crosshairs)
	class UTexture2D* CrosshairsCenter;        //准心

	UPROPERTY(EditAnywhere,Category=Crosshairs)
	UTexture2D* CrosshairsCenterLeft;

	UPROPERTY(EditAnywhere,Category=Crosshairs)
	UTexture2D* CrosshairsCenterRight;

	UPROPERTY(EditAnywhere,Category=Crosshairs)
	UTexture2D* CrosshairsCenterTop;

	UPROPERTY(EditAnywhere,Category=Crosshairs)
	UTexture2D* CrosshairsCenterBottom;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere,Category= Cambat)
	float FireDelay=0.11f;

	UPROPERTY(EditAnywhere,Category=Cambat)
	bool bAutomatic=true;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
	bool bUseScatter = false;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	//霰弹散布相关参数
	UPROPERTY(EditAnywhere,Category="Weapom Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
	float SphereRadius =75.f;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated,EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset *FireAnimation;
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass; //选择弹壳

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client,Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client,Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
	void SpendRound(); 

	UPROPERTY(EditAnywhere)
	int32 MagCapcity;            //弹夹容量

	//服务器还未处理的弹药减少请求，在SpendRound()中增加，在ClientUpdateAmmo中减少
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	
public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapcity; }  //获取备弹
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
};