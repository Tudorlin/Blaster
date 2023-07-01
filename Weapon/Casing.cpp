// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

ACasing::ACasing()
{

	CasingMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);      //防止弹壳遮挡相机
	CasingMesh->SetSimulatePhysics(true);    //模拟物理
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse =10.f;

}

void ACasing::CasingRotation()
{
	FRotator CasingRotation=CasingMesh->GetComponentRotation();
	float Rotation_X=FMath::RandRange(-20,20);
	float Rotation_Z=FMath::RandRange(-15,15);
	CasingRotation.Pitch+=Rotation_X;
	CasingRotation.Roll+=Rotation_Z;
	SetActorRotation(CasingRotation);
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this,&ACasing::OnHit);  //需要在蓝图中勾选模拟命中事件
	CasingMesh->AddImpulse(GetActorForwardVector()*ShellEjectionImpulse);        //添加冲量
	
}

void ACasing::OnHit(UPrimitiveComponent* MyComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	CasingRotation();
	if(ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ShellSound,GetActorLocation());
	}
	Destroy();
}

