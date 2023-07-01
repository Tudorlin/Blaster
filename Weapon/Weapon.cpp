// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/BlasterComponent//CombatComponent.h"
#include "Casing.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	/*PrimaryActorTick是一个结构体，用于管理Actor的定时器功能。它包含了许多常用的变量，以下是其中一些常用的变量：

	bCanEverTick：表示Actor是否允许定时器更新，默认为true。
	bStartWithTickEnabled：表示Actor是否在创建时立即开始定时器更新，默认为false。
	bTickEvenWhenPaused：表示Actor是否在游戏暂停时也能够定时器更新，默认为false。
	bAllowTickOnDedicatedServer：表示Actor在独立服务器上是否允许进行定时器更新，默认为false。
	bHighPriorityTickIsCritical：表示Actor是否将高优先级定时器视为关键定时器，默认为false。
	bRunOnAnyThread：表示Actor是否可以在任何线程上执行定时器更新，默认为false。
	PrimaryTickFunction：用于定义定时器的具体功能，包括定时器的更新间隔、函数回调等等。*/
	bReplicates = true;
	SetReplicateMovement(true);
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);   //对于所有类型的通道都设置为阻挡
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);  //对于pawn这个通道则是忽略
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);    //mesh本身没有碰撞

	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	/*OnComponentBeginOverlap等类似的变量是通过委托（Delegate）实现的。委托是一种事件处理机制，它允许在特定事件发生时执行一系列函数或方法。
	 *在UE4中，许多场景中都使用了委托来实现事件的触发和响应。OnComponentBeginOverlap是一个委托类型的变量，用于处理组件与其他物体重叠开始时的事件。
	 *当一个组件与其他物体开始重叠时，该委托会触发，并调用绑定给它的函数。通常，我们可以使用委托来注册事件处理函数，以便在重叠发生时执行自定义的逻辑。
	 *在使用OnComponentBeginOverlap时，我们可以通过调用AddDynamic()函数将特定函数绑定到委托上，从而在重叠事件发生时触发该函数*/
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind,COND_OwnerOnly);
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)  //ping太过高时禁用服务器倒带，ping不是非常高的时候启用服务器倒带
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner==nullptr)
	{
		BlasterOwnerCharacter=nullptr;
		BlasterOwnerController=nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}

void AWeapon::SpendRound()
{
	Ammo=FMath::Clamp(Ammo-1,0,MagCapcity);
	SetHUDAmmo();                                      //更新服务端子弹数变化
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}                               //服务器处理完请求减少子弹数
	else
	{
		++Sequence;               //服务器为处理的请求数将被记录下来
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)            //
{
	if(HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;                  //
	Ammo-=Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo=FMath::Clamp(Ammo+AmmoToAdd,0,MagCapcity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);	     //更新客户端更换的弹药数
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority()) return;
	Ammo=FMath::Clamp(Ammo+AmmoToAdd,0,MagCapcity);
	BlasterOwnerCharacter = BlasterOwnerCharacter==nullptr?Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(BlasterOwnerCharacter&&BlasterOwnerCharacter->GetCombat()&&IsFull())
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(BlasterOwnerCharacter&&bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController ==nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if(BlasterOwnerController&&HasAuthority()&&!BlasterOwnerController->HighPingDelegate.IsBound()) //委托中没有绑定函数时将设置服务器倒带的函数绑定	
		{
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	if(WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController ==nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if(BlasterOwnerController&&HasAuthority()&& BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController ==nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if(BlasterOwnerController&&HasAuthority()&&BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);   //丢弃武器时取消绑定
		}
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo<=0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapcity;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)             //弹道散布函数
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalize = (HitTarget-TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalize*DistanceToSphere;

	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	// DrawDebugLine(
	// 	GetWorld(),
	// 	TraceStart,
	// 	FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
	// 	FColor::Cyan,
	// 	true);

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation,false);
	}
	if(CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));   //弹壳插槽
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
					);
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter=nullptr;
	BlasterOwnerController=nullptr;
}

