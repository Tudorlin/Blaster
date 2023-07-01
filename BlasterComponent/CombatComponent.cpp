


#include "CombatComponent.h"

#include "../../../Plugins/Developer/RiderLink/Source/RD/thirdparty/spdlog/include/spdlog/fmt/bundled/format.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "TimerManager.h"
#include "Blaster/Weapon/ShotGun.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed =600.f;
	AimWalkSpeed=450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent,SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent,CarriedAmmo,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent,Grenades);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed=BaseWalkSpeed;
		
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(Character&&Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget=HitResult.ImpactPoint;
		
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombtState::ECS_Reloading:
		if(Character&&!Character->IsLocallyControlled())
			HandleRelod();
		break;
	case ECombtState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombtState::ECS_ThrowingGrenade:
		if(Character&&!Character->IsLocallyControlled())
		{
			Character->playThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombtState::ECS_SwappingWeapons:
		if(Character&&Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle,StartingAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher,StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol,StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun,SubmachineGunStartingAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun,ShotGunStartingAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle,SniperRifleStartingAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher,GranadeStartingAmmo);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed=bPressed;
	if(bFireButtonPressed&&EquippedWeapon)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire=false;
		if(EquippedWeapon)
		{
			CrosshairshootingFctor=0.75f;
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile :
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan :
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{	
	if(EquippedWeapon&&Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget,EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if(EquippedWeapon&&Character)
	{
		HitTarget = EquippedWeapon->FireType==EFireType::EFT_HitScan ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget,EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotGun* Shotgun = Cast<AShotGun>(EquippedWeapon);
	if(Shotgun&&Character)
	{
		TArray<FVector_NetQuantize>HitTargets;
		Shotgun->ShotgunTraceEndwithScatter(HitTarget,HitTargets);
		if(!Character->HasAuthority())
			ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets,EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotGun* Shotgun = Cast<AShotGun>(EquippedWeapon);
	if(Shotgun==nullptr||Character==nullptr)  return;
	if(CombatState==ECombtState::ECS_Reloading||CombatState==ECombtState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState=ECombtState::ECS_Unoccupied;
	}
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets,float FireDelay)
{
	MuticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)   //验证发到服务器请求的某个可能会被修改器修改的属性，文档关键词RPC的验证模块
{
	if(EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay,FireDelay,0.001f);
		return bNearlyEqual;
	}
	return true;
}


void UCombatComponent::MuticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if(Character&&Character->IsLocallyControlled()&&!Character->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon==nullptr) return;
	if(Character&&CombatState==ECombtState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::StartFireTimer()        //定时器
{
	if(EquippedWeapon==nullptr||Character==nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if(EquippedWeapon==nullptr) return;
	bCanFire=true;
	if(bFireButtonPressed&&EquippedWeapon->bAutomatic)
	{	
		Fire();
	}
	
ReloadEmptyWeapon();
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon==nullptr) return false;
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombtState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun) return true;
	if(bLocallyReloading) return false;
	return !EquippedWeapon->IsEmpty()&&bCanFire&&CombatState==ECombtState::ECS_Unoccupied;
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget,float FireDelay)
{
	MulticastFire(TraceHitTarget);                      //在服务器上调用多播函数
	if (Character && CombatState == ECombtState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombtState::ECS_Unoccupied;
		return;
	}
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if(EquippedWeapon)
	{
		bool bNearlyEquip = FMath::IsNearlyEqual(EquippedWeapon->FireDelay,FireDelay, 0.001f);
		return bNearlyEquip;
	}
	return true;
}


void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(Character&&Character->IsLocallyControlled()&&!Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if(Character==nullptr||Character->GetMesh()==nullptr||ActorToAttach==nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if(Character==nullptr||Character->GetMesh()==nullptr||ActorToAttach==nullptr||EquippedWeapon==nullptr) return;
	// bool bUsePistolSocket =
	// 	EquippedWeapon->GetWeaponType()==EWeaponType::EWT_Pistol||
	// 		EquippedWeapon->GetWeaponType()==EWeaponType::EWT_SubmachineGun;
	// FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("LeftHandSocket"));
	// FString Socketvalue = HandSocket? TEXT("True") : TEXT("False");
	// UKismetSystemLibrary::PrintString(this,Socketvalue);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if(EquippedWeapon==nullptr) return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo=CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller==nullptr?Cast<ABlasterPlayerController>(Character->Controller):Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if(Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation());
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if(WeaponToEquip==nullptr) return;
		DropEquippedWeapon();
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(WeaponToEquip);
		ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);
	
	SecondaryWeapon->SetOwner(Character);
	
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(EquippedWeapon);
		if (SecondaryWeapon->GetWeaponMesh())
		{
			SecondaryWeapon->GetWeaponMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
			SecondaryWeapon->GetWeaponMesh()->MarkRenderStateDirty();
		}
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if(EquippedWeapon&&EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombtState::ECS_Unoccupied) return;
	if(EquippedWeapon!=nullptr&&SecondaryWeapon==nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	Character->GetCharacterMovement()->bOrientRotationToMovement=false;
	Character->bUseControllerRotationYaw=true;
}

bool UCombatComponent::bShouldSwapWeapon()
{
	return(EquippedWeapon!=nullptr&&SecondaryWeapon!=nullptr);
}

void UCombatComponent::SwapWeapon()
{
	if(CombatState!=ECombtState::ECS_Unoccupied ||Character==nullptr||!Character->HasAuthority()) return;

	Character->PlaySwapMontage();
	CombatState =ECombtState::ECS_SwappingWeapons;
	Character->bFinishedSwapping = false;
	if(SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

void UCombatComponent::FinsihSwap()
{
	if(Character&&Character->HasAuthority())
	{
		CombatState = ECombtState::ECS_Unoccupied;
	}
	if(Character) Character->bFinishedSwapping = true;
}

void UCombatComponent::FinishSwapAttachWeapons()
{	
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon&&Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement=false;
		Character->bUseControllerRotationYaw=true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::Reload()
{
	if(Character->GetbElimed())  return;
	if(CarriedAmmo>0&&EquippedWeapon->GetAmmo()<EquippedWeapon->GetMagCapacity()&&CombatState==ECombtState::ECS_Unoccupied&&!bLocallyReloading)
	{
		ServerReload();
		HandleRelod();                       //客户端动画播放
		bLocallyReloading = true;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Character==nullptr||EquippedWeapon==nullptr) return;
	CombatState =ECombtState::ECS_Reloading;
	if(!Character->IsLocallyControlled())
	HandleRelod();                    //服务端动画播放
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon==nullptr) return 0;
	int32 RoomInMag=EquippedWeapon->GetMagCapacity()-EquippedWeapon->GetAmmo();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::HandleRelod()
{
	if(Character)
	{
		Character->PlayRloadMontage();
	}
}

void UCombatComponent::FinishedRelod()
{
	if(Character==nullptr) return;
	bLocallyReloading = false;
	if(Character->HasAuthority())
	{
		CombatState=ECombtState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if(Character&&Character->HasAuthority())
	{
		UpdateShotgunValues();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller==nullptr?Cast<ABlasterPlayerController>(Character->Controller):Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd = 
		CombatState == ECombtState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::UpdateShotgunValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1 ;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if(CarriedAmmo==0||EquippedWeapon->IsFull())
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller==nullptr?Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && Character->HasAuthority() && GrenadeClass && Character->GetAttachGrenade())
	{
		const FVector StartingLocation = Character->GetAttachGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if(Character&&Character->GetAttachGrenade())
	{
		Character->GetAttachGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::ThrowGrenade()
{
	if(Grenades==0)  return;
	if(CombatState!=ECombtState::ECS_Unoccupied||EquippedWeapon==nullptr) return;
	CombatState=ECombtState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->playThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(Grenades==0)  return;
	CombatState=ECombtState::ECS_ThrowingGrenade;
	if(Character)
	{
		Character->playThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades=FMath::Clamp(Grenades-1,0,MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombtState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if(Character)
		{
			float DistanceToCharacter=(Character->GetActorLocation()-Start).Size();
			Start+=CrosshairWorldDirection*(DistanceToCharacter+100.f);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		//DrawDebugSphere(GetWorld(),TraceHitResult.ImpactPoint,100.f,12,FColor::Red);
		if(TraceHitResult.GetActor()&&TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterfaces>())//瞄准时准心变色
			{
			HUDPackage.CrosshairColor=FLinearColor::Red;
			}
		else
		{
			HUDPackage.CrosshairColor=FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Character==nullptr||Character->Controller==nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter=EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft=EquippedWeapon->CrosshairsCenterLeft;
				HUDPackage.CrosshairsRight=EquippedWeapon->CrosshairsCenterRight;
				HUDPackage.CrosshairsTop=EquippedWeapon->CrosshairsCenterTop;
				HUDPackage.CrosshairsBottom=EquippedWeapon->CrosshairsCenterBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter=nullptr;
				HUDPackage.CrosshairsLeft=nullptr;
				HUDPackage.CrosshairsRight=nullptr;
				HUDPackage.CrosshairsTop=nullptr;
				HUDPackage.CrosshairsBottom=nullptr;
			}
//将步行速度0-600映射为0-1
		    FVector2d WalkSpeedRange(0.f,Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2d VelaocityMultiplierRange(0.f,1.f);
			FVector Velaocity=Character->GetVelocity();
			Velaocity.Z=0.f;

			CrosshairVelocityFctor=FMath::GetMappedRangeValueClamped(WalkSpeedRange,VelaocityMultiplierRange,Velaocity.Size());

			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFctor=FMath::FInterpTo(CrosshairInAirFctor,2.25f,DeltaTime,2.25f);
			}
			else
			{
				CrosshairInAirFctor=FMath::FInterpTo(CrosshairInAirFctor,0.f,DeltaTime,30.f);
			}
			if(bAiming)
			{
				CrosshairAimFctor=FMath::FInterpTo(CrosshairAimFctor,0.58f,DeltaTime,30.f);
			}
			else
			{
				CrosshairAimFctor=FMath::FInterpTo(CrosshairAimFctor,0.f,DeltaTime,30.f);
			}

			CrosshairshootingFctor=FMath::FInterpTo(CrosshairshootingFctor,0.f,DeltaTime,40.f);
			
            HUDPackage.CrosshairSpread=
            	0.5f+
            	CrosshairVelocityFctor+
            		CrosshairInAirFctor-
            			CrosshairAimFctor+
            				CrosshairshootingFctor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(Character==nullptr||EquippedWeapon==nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed=bIsAiming?AimWalkSpeed:BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	if(Character->IsLocallyControlled()) bAimingButtonPressed = bIsAiming;
}

void UCombatComponent::OnRep_Aiming()
{
	if(Character&&Character->IsLocallyControlled())
	{
		bAiming = bAimingButtonPressed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)       //在服务器上设置变量
{
	bAiming = bIsAiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed=bIsAiming?AimWalkSpeed:BaseWalkSpeed;
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if(CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType]=FMath::Clamp(CarriedAmmoMap[WeaponType]+AmmoAmount,0,MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	if(EquippedWeapon&&EquippedWeapon->IsEmpty()&&EquippedWeapon->GetWeaponType()==WeaponType)
	{
		Reload();
	}
}