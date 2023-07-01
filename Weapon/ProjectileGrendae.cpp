// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrendae.h"

#include "Kismet/GameplayStatics.h"

AProjectileGrendae::AProjectileGrendae()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;                  //反弹
	
}


void AProjectileGrendae::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this,&AProjectileGrendae::OnBounce);   //反弹
}

void AProjectileGrendae::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();
}

void AProjectileGrendae::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if(BoundSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BoundSound,
			GetActorLocation());
	}
}
