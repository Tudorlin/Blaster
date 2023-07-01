// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"

#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include"Engine/SkeletalMeshSocket.h"	
#include"Sound/SoundCue.h"
#include"Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void AShotGun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn * OwnPawn = Cast<APawn>(GetOwner());
	if(OwnPawn==nullptr)  return ;
	AController *InstigatorController = OwnPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		
		TMap<ABlasterCharacter*,uint32> HitMap;
		TMap <ABlasterCharacter*,uint32> HeadShotHitMap;
		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start,HitTarget,FireHit);

			ABlasterCharacter * BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());  //被击角色
			if(BlasterCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				if(bHeadShot)
				{
					if(HeadShotHitMap.Contains(BlasterCharacter)) 
					    HeadShotHitMap[BlasterCharacter]++;
					else
						HeadShotHitMap.Emplace(BlasterCharacter,1);
				}
				else
				{
					if(HitMap.Contains(BlasterCharacter))
						HitMap[BlasterCharacter]++;
					else
						HitMap.Emplace(BlasterCharacter,1);
				}
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
					);
				}
				if(HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f,.5f)
					);
				}
			}
		}
		
		TArray<ABlasterCharacter*> HitCharacters;
		TMap<ABlasterCharacter*,float> DamageMap;
		for(auto HitPair : HitMap)
		{
			if(HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key,HitPair.Value*Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
			for(auto HeadShotHitPair : HeadShotHitMap)
			{
				if(HeadShotHitPair.Key)
				{
					if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
					else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
					
					HitCharacters.AddUnique(HeadShotHitPair.Key);
				}
			}
			for(auto DamagePair : DamageMap)
			{
				if(DamagePair.Key&&InstigatorController)
				{
					bool bCauseAuthDamage = !bUseServerSideRewind||OwnPawn->IsLocallyControlled();
					if(HasAuthority()&&bCauseAuthDamage)
					{
						UGameplayStatics::ApplyDamage(
							DamagePair.Key,                                     //被击玩家
							DamagePair.Value,
							InstigatorController,                         //射击玩家控制器
							this,
							UDamageType::StaticClass()
							);
					}
				}
			}
		}
		if(!HasAuthority()&&bUseServerSideRewind)
		{
			BlasterOwnerCharacter = BlasterOwnerCharacter = nullptr ? Cast<ABlasterCharacter>(OwnPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController = nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;

			if(BlasterOwnerCharacter&&BlasterOwnerController&&BlasterOwnerCharacter->GetLagCompensation())
			{
				BlasterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BlasterOwnerController->GetServerTime()-BlasterOwnerController->SingleTripTime
					);
			}
		}
	}
}

void AShotGun::ShotgunTraceEndwithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalize = (HitTarget-TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalize*DistanceToSphere;

	for(uint32 i =0;i<NumberOfPellets;i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
	}
}
