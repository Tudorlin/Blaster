// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket&&World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 枪口到射线击中点
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnProjectile = nullptr;
/*
 *高延迟下客户端射击时子弹生成会有延迟，此段代码会让客户端在合适时生成本地并不会复制的子弹类，让子弹在客户端玩家自身看起来没有延迟
 */
		if(bUseServerSideRewind)      //使用服务器倒带
		{
			if(InstigatorPawn->HasAuthority())          //服务端，使用复制的子弹
			{
				if(InstigatorPawn->IsLocallyControlled())   //服务端玩家本人
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bShouldUseServerSideRewind = false;
					SpawnProjectile->Damage = Damage;
					SpawnProjectile->HeadShotDamage = HeadShotDamage;
				}
				else                            //服务端上显示的其他玩家,不用复制但需要服务器倒带,否则会双倍伤害
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSdideReWindProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
					SpawnProjectile->bShouldUseServerSideRewind = true;
				}
			}
			else           //客户端，使用服务器倒带
			{
				if(InstigatorPawn->IsLocallyControlled())     //客户端玩家本身，生成不复制的子弹，需要使用服务器倒带
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSdideReWindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
 					SpawnProjectile->bShouldUseServerSideRewind = true;
					SpawnProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnProjectile->InitialVelocity = SpawnProjectile->GetActorForwardVector() * SpawnProjectile->InitialSpeed;
				}
				else     //客户端中其他玩家的复制体，生成不复制的子弹，不需要服务器倒带
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSdideReWindProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
					SpawnProjectile->bShouldUseServerSideRewind = false;
				}
			}
		}
		else  //不使用服务器倒带
		{
			if(InstigatorPawn->HasAuthority())
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnProjectile->bShouldUseServerSideRewind = false;
				SpawnProjectile->Damage = Damage;
				SpawnProjectile->HeadShotDamage = HeadShotDamage;
			}
		}
	}
}
