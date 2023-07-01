// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Weapon/WeaponType.h"
#include "Kismet/GameplayStatics.h"

APickups::APickups()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
    RootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Root"));

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));

	PickupMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void APickups::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&APickups::BindOverlapTimerFinished,
			BindOverlapTime);
	}
}

void APickups::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh)
	{
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}

}

void APickups::Destroyed()
{
	Super::Destroyed();

	if(PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation());
	}
	if(PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation());
	}
	
}

void APickups::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
	UPrimitiveComponent* Othercomp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APickups::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this,&APickups::OnSphereOverlap);
}

