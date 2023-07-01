// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property !=nullptr ? PropertyChangedEvent.GetPropertyName() : NAME_None;

	if(PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet,InitialSpeed))
	{
		ProjectileMovementComponent->InitialSpeed = InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = InitialSpeed;
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* MyComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	ABlasterCharacter* OwnerCharacter=Cast<ABlasterCharacter>(GetOwner());
	if(OwnerCharacter)
	{
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if(OwnerController)
		{
			if(OwnerCharacter->HasAuthority()&&!bShouldUseServerSideRewind)
			{
				const float DamageToCause = Hit.BoneName.ToString()==FString("head") ? HeadShotDamage : Damage;
				UGameplayStatics::ApplyDamage(OtherActor,DamageToCause,OwnerController,this,UDamageType::StaticClass());
				Super::OnHit(MyComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if(HitCharacter&&OwnerCharacter->GetLagCompensation()&&OwnerCharacter->IsLocallyControlled()&&bShouldUseServerSideRewind)
			{
				OwnerCharacter->GetLagCompensation()->ProjectileScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime()-OwnerController->SingleTripTime);
			}
		}
	}
	Super::OnHit(MyComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
/*
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector()*InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);                     //忽略子弹之间的影响	

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this,PathParams,PathResult);*/
}
