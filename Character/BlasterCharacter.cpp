// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponent/BuffComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"             //包含了网络同步相关的类和函数
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/BlasterGameState/BlasterGameState.h"
#include "Blaster/BlasterPlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	/*bUsePawnControlRotation是一个APawn类的成员变量，它决定了Pawn是否应该跟随控制器（Controller）的旋转。
	 如果设置为true，Pawn的旋转将跟随控制器的旋转。如果设置为false，则Pawn将使用其自身的旋转，而不跟随控制器的旋转。这个成员变量通常用于控制摄像机是否跟随Pawn的旋转。
     GetCharacterMovement()->bOrientRotationToMovement是一个UCharacterMovementComponent类的成员变量，它决定了角色移动时是否应该沿着移动方向旋转。
     如果设置为true，当角色开始移动时，其朝向将随着移动方向旋转。如果设置为false，则角色的朝向将保持不变，而不受移动方向的影响。这个成员变量通常用于控制角色在移动时的朝向。*/
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	OverheadWidget->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Ignore);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));
	
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(),FName("GrenadeSocket"));

	//角色HitBox
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitBoxCollisionBoxes.Add(FName("head"),head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitBoxCollisionBoxes.Add(FName("pelvis"),pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitBoxCollisionBoxes.Add(FName("spine_02"),spine_02);
	
	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitBoxCollisionBoxes.Add(FName("spine_03"),spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitBoxCollisionBoxes.Add(FName("upperarm_l"),upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitBoxCollisionBoxes.Add(FName("upperarm_r"),upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitBoxCollisionBoxes.Add(FName("lowerarm_l"),lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitBoxCollisionBoxes.Add(FName("lowerarm_r"),lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitBoxCollisionBoxes.Add(FName("hand_l"),hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitBoxCollisionBoxes.Add(FName("hand_r"),hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitBoxCollisionBoxes.Add(FName("blanket"),blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitBoxCollisionBoxes.Add(FName("backpack"),backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitBoxCollisionBoxes.Add(FName("thigh_l"),thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitBoxCollisionBoxes.Add(FName("thigh"),thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitBoxCollisionBoxes.Add(FName("calf_l"),calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitBoxCollisionBoxes.Add(FName("calf_r"),calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitBoxCollisionBoxes.Add(FName("foot_l"),foot_l);	

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitBoxCollisionBoxes.Add(FName("foot_r"),foot_r);

	for(auto Box : HitBoxCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	
	TurningInPlace=ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency=66.f;
	MinNetUpdateFrequency=33.f;

	DissolveTimeline=CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade",IE_Pressed,this,&ABlasterCharacter::GrenadeButtonPressed);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const   //复制变量
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter,Health);
	DOREPLIFETIME(ABlasterCharacter,bDisableGameplay);
	DOREPLIFETIME(ABlasterCharacter,Shield);
	/*条件属性复制：
	*
	COND_InitialOnly - 此属性将仅尝试发送初始束
	COND_OwnerOnly - 这个属性只会发送给actor的所有者
	COND_SkipOwner - 此属性发送到除所有者之外的每个连接
	COND_SimulatedOnly - 此属性只会发送给模拟演员
	COND_AutonomousOnly - 此属性将仅发送给自治参与者
	COND_SimulatedOrPhysics - 此属性将发送到模拟的 OR bRepPhysics 演员
	COND_InitialOrOwner - 此属性将在初始数据包上发送，或发送给参与者所有者
	COND_Custom - 此属性没有特定条件，但希望能够通过 SetCustomIsActiveOverride 打开/关闭
	GetLifetimeReplicatedProps函数是用于获取需要网络同步的属性列表的虚函数。
	它会返回一个TArray类型的属性列表，这个列表描述了需要进行网络同步的属性和它们的同步规则，如同步方式（复制、RPC、可靠的RPC等）和同步频率等。
*/
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	if(GetLocalRole()==ENetRole::ROLE_SimulatedProxy)
	{
		SimProxiesTurn();
	}
	TimeSinceLastMovementReplication=0.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this,&ABlasterCharacter::ReceiveDamge);
	}
	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}	

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPlayerState==nullptr)
	{
		BlasterPlayerState=GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
			SetTeamColor(BlasterPlayerState->GetTeam());

			ABlasterGameState * BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));  //打平时在所有分数相同的玩家头上生成王冠
			if(BlasterGameState&&BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::Elim(bool bPLayerLeftGame)
{
	DropOrDestroyWeapons();
	MulticastElim(bPLayerLeftGame);
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed=true;
	PlayElimMontage();

	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance=UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this);
		GetMesh()->SetMaterial(0,DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Emissive"),200.f);
	}
	StartDissolve();

	//禁用移动
	bDisableGameplay=true;
	GetCharacterMovement()->DisableMovement();

	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);                     //取消碰撞
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X,GetActorLocation().Y,GetActorLocation().Z+200);
		ElimBotComponent=UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}

	bool bHideSniperScope = IsLocallyControlled() && 
		Combat && 
		Combat->bAiming && 
		Combat->EquippedWeapon && 
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(
	ElimTimer,
	this,
	&ABlasterCharacter::ElimTimerFinished,
	ElimDely);
	/*SetTimer()
	第一个参数：把时间控制权交给谁，ElimTimer
	第二个参数： 哪个类的对象
	第三个参数： 间隔一段时间之后去干什么事情，（去执行ElimTimerFinished（）函数）
	第四个参数： 间隔多久（例. 1s）去执行EmimTimerFinished（）函数
	第五个参数： 间隔1s 执行完ElimTimerFinished()函数之后，还要不要循环再去执行
*/
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this,Controller);
	}
	if(bLeftGame&&IsLocallyControlled())
	{
		OnLeftGame.Broadcast();    //将该委托广播给所有绑定的对象，已经过期的对象可能会除外
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this,&ABlasterCharacter::UpdateDissolveMaterial);
	if(DissolveCurve&&DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve,DissolveTrack);
		DissolveTimeline->Play(); 
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(Combat&&Combat->EquippedWeapon&&BlasterGameMode&&BlasterGameMode->GetMatchState()!=MatchState::InProgress)
	{
		Combat->EquippedWeapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if(Weapon==nullptr) return;
	if(Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if(Combat)
	{
		if(Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if(Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void ABlasterCharacter::PostInitializeComponents()              //初始化组件
/*PostInitializeComponents()是在UE4的Actor类中定义的一个虚函数。它是在Actor的组件初始化完成后被调用的函数。
* PostInitializeComponents()函数在这些组件初始化完成后被调用，它是一个可以被重写的虚函数。开发人员可以在派生的Actor类中重写这个函数，以便在组件初始化完成后执行一些额外的逻辑或初始化操作。
一些常见的在PostInitializeComponents()函数中执行的操作包括：
	配置和初始化组件属性。
	注册事件处理函数。
	设置初始状态或变量。
	进行一些与组件相关的计算或处理。
需要注意的是，PostInitializeComponents()函数是在Actor组件的初始化过程中调用的，因此它的执行时间点在BeginPlay()函数之前。
如果需要在组件初始化完成后进行一些逻辑操作，但又早于BeginPlay()函数，那么可以使用PostInitializeComponents()来实现。
 */
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if(Buff)
	{
		Buff->Character = this;

		Buff->SetInitialSpeed(
			GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched);

		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if(LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance&&FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName=bAiming?FName("RifleAim"):FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if(bDisableGameplay==true) return;
	if(Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::PlayRloadMontage()
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance&&ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
			case  EWeaponType::EWT_AssaultRifle :
			SectionName=FName("Rifle");
			break;

		case  EWeaponType::EWT_RocketLauncher :
			SectionName=FName("RocketLauncher");
			break;
		case  EWeaponType::EWT_Pistol :
			SectionName=FName("Pistol");
			break;
		case  EWeaponType::EWT_SubmachineGun :
			SectionName=FName("Pistol");
			break;
		case EWeaponType::EWT_ShotGun :
			SectionName=FName("ShotGun");
			break;
		case  EWeaponType::EWT_SniperRifle :
			SectionName=FName("SniperRifle");
			break;
		case  EWeaponType::EWT_GrenadeLauncher :
			SectionName=FName("GrenadeLauncher");
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance&&ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if(Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::playThrowGrenadeMontage()
{
	UAnimInstance* AninInstance = GetMesh()->GetAnimInstance();
	if(AninInstance&&ThrowGrenadeMontage)
	{
		AninInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance&&HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance&&SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterCharacter::ReceiveDamge(AActor* DamageActor, float Damage, const UDamageType* DamageTupe,
                                     AController* InstigatorCotroller, AActor* DamageCause)
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(bElimmed||BlasterGameMode == nullptr) return;
	Damage = BlasterGameMode->CalculateDamage(InstigatorCotroller,Controller,Damage);
	float DameageToHealth = Damage;
	if(Shield> 0)
	{
		if(Shield >=Damage)
		{
			Shield = FMath::Clamp(Shield-Damage,0.f,MaxShield);
			DameageToHealth = 0.f;
		}
		else
		{
			DameageToHealth = FMath::Clamp(DameageToHealth - Shield,0.f,MaxHealth);
			Shield = 0.f;
		}
	}
	Health=FMath::Clamp(Health-DameageToHealth,0.f,MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();
	
	if(Health==0)
	{ 
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorCotroller);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
    UpdateHUDHealth();
	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController=BlasterPlayerController==nullptr?Cast<ABlasterPlayerController>(Controller):BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health,MaxHealth);
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if(Shield<LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController=BlasterPlayerController==nullptr?Cast<ABlasterPlayerController>(Controller):BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield,MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController=BlasterPlayerController==nullptr?Cast<ABlasterPlayerController>(Controller):BlasterPlayerController;
	if(BlasterPlayerController&&Combat&&Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	UWorld *World = GetWorld();
	if(BlasterGameMode && World &&!bElimmed && DefaultWeaponClass)
	{
		AWeapon * StartingWeapon = World -> SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if(Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if(GetMesh() == nullptr || OriginalTeamColor == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh() ->SetMaterial(0,OriginalTeamColor);
		DissolveMaterialInstance = BlueTeamDissolveMaterialInstance;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0,RedTeamColor);
		DissolveMaterialInstance = RedTeamDissolveMaterialInstance;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0,BlueTeamColor);
		DissolveMaterialInstance = BlueTeamDissolveMaterialInstance;
		break;
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if(CrownSystem==nullptr) return;
	if(CrownComponent==nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation()+FVector(0.f,0.f,115.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false);
	}
	if(CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{

	BlasterGameMode = BlasterGameMode ==nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
    BlasterPlayerState = BlasterPlayerState ==nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if(BlasterGameMode&&BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if(bDisableGameplay)
	{
		bUseControllerRotationYaw=false;
		TurningInPlace=ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if(GetLocalRole()>ENetRole::ROLE_SimulatedProxy&&IsLocallyControlled()) //本地大于代理
		{
		AimOffest(DeltaTime);
		}
	else
	{
		TimeSinceLastMovementReplication+=DeltaTime;
		if(TimeSinceLastMovementReplication>0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if(bDisableGameplay==true) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));  //GetUnitAxis作用为获取该方向的单位轴，即Direction为Yaw的归一化向量
		AddMovementInput(Direction, Value);
	}
}
void ABlasterCharacter::MoveRight(float Value)
{
	if(bDisableGameplay==true) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}
void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}
void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
void ABlasterCharacter::EquipButtonPressed()
{
	if(bDisableGameplay) return;
	if (Combat)
	{
		if(Combat->CombatState==ECombtState::ECS_Unoccupied) ServerEquipButtonPressed();
		bool bSwap = Combat->bShouldSwapWeapon()&&
			!HasAuthority()&&
				Combat->CombatState==ECombtState::ECS_Unoccupied&&
					OverlappingWeapon == nullptr;
		if(bSwap)
		{
			PlaySwapMontage();
			Combat->CombatState=ECombtState::ECS_SwappingWeapons;
			bFinishedSwapping = false;     //换武器完成重置布尔
		}
	}
}
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (OverlappingWeapon)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
	else if(Combat->bShouldSwapWeapon())
	{
		Combat->SwapWeapon();
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bDisableGameplay==true) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)  //根据是否重叠设置pick upUI是否可视
/*具体来说，函数会先判断是否已经有重叠的武器，如果有的话，会调用OverlappingWeapon对象的ShowPickupWidget方法，将提示UI隐藏起来。
 *接着，将OverlappingWeapon赋为传入的Weapon对象，代表此时角色与该武器重叠。
 *如果当前角色是本地控制的，再对OverlappingWeapon进行一次判断，如果不为null，则调用其ShowPickupWidget方法，将提示UI显示出来。*/
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	/*如果当前有旧的武器与角色重叠，那么就需要先调用旧武器的ShowPickupWidget方法将提示UI隐藏，避免在切换到新武器时，旧武器的提示UI还显示在屏幕上。
	 * 然后再将OverlappingWeapon赋值为新武器对象，并根据情况再次调用新武器的ShowPickupWidget方法，显示提示UI。
	 *这种先隐藏再显示的方式，可以保证在重叠的两个武器之间切换时，提示UI能够正确地显示和隐藏。*/
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if(bDisableGameplay==true) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch=GetBaseAimRotation().Pitch;
	if(AO_Pitch>90.f&&!IsLocallyControlled())
	{
		FVector2d InRange=(270.f,360.f);
		FVector2d OutRange=(-90.f,0.f);
		AO_Pitch=FMath::GetMappedRangeValueClamped(InRange,OutRange,AO_Pitch);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity=GetVelocity();
	Velocity.Z=0.0f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffest(float Deltime)
{
	if(Combat&&Combat->EquippedWeapon==nullptr) return;
	float Speed=CalculateSpeed();
	bool bIsInAir =GetCharacterMovement()->IsFalling();

	if(Speed==0.f&&!bIsInAir)            //站立时
		{
		bRotateRootBone=true;
		FRotator CurrentAimRotation=FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		FRotator DeltaAimRotation=UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
		AO_Yaw=DeltaAimRotation.Yaw;
		if(TurningInPlace==ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw=AO_Yaw;
		}
		bUseControllerRotationYaw=true;
		TurnInPlace(Deltime);
		}
	if(Speed>0.f||bIsInAir)
	{
		bRotateRootBone=false;
		StartingAimRotation=FRotator(0.f,GetBaseAimRotation().Yaw,0.f);
		AO_Yaw=0;
		bUseControllerRotationYaw=true;
		TurningInPlace=ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}

void ABlasterCharacter::SimProxiesTurn()
{
	if(Combat==nullptr||Combat->EquippedWeapon==nullptr) return;

	bRotateRootBone=false;
	float Speed=CalculateSpeed();
	if(Speed>0.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotationLastFram = ProxyRotation;
	ProxyRotation=GetActorRotation();
	ProxyYaw=UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation,ProxyRotationLastFram).Yaw;

	if(FMath::Abs(ProxyYaw)>TurnThreshold)
	{
		if(ProxyYaw>TurnThreshold)
		{
			TurningInPlace=ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw<TurnThreshold)
		{
			TurningInPlace=ETurningInPlace::ETIP_Right;
		}
		else
		{
			TurningInPlace=ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace=ETurningInPlace::ETIP_NotTurning; 
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw>90.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw<-90.f)
	{
		TurningInPlace=ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace!=ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw=FMath::FInterpTo(InterpAO_Yaw,0.f,DeltaTime,4.f);
		AO_Yaw=InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw)<15.f)
		{
			TurningInPlace=ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation=FRotator(0.f,GetBaseAimRotation().Yaw,0.0f);  //Yaw在-15到15区间时不会播放转向动画
		}
	}
}

void ABlasterCharacter::Jump()
{
	if(bDisableGameplay==true) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if(bDisableGameplay==true) return;
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if(bDisableGameplay==true) return;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled()) return;
	if((FollowCamera->GetComponentLocation()-GetActorLocation()).Size()<CameraRhreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee=true;
		}
		if(Combat&&Combat->SecondaryWeapon&&Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee=false;
		}
		if(Combat&&Combat->SecondaryWeapon&&Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
/*OnRep_OverlappingWeapon在服务器端调用时，客户端也会自动同步。当OverlappingWeapon属性被复制到了客户端的时候，在客户端内部会再次调用这个函数。
 *该函数的作用是处理在其他客户端上由服务器同步导致的重叠武器的变化。在函数中，首先判断是否有现在正在重叠的武器，如果有，就调用ShowPickupWidget方法将其提示UI显示出来；
 *然后再判断参数传入的上一个武器是否为null，如果不为null，那么就将其ShowPickupWidget方法调用关闭，将提示UI隐藏。*/
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
    if(Combat==nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combat==nullptr) return FVector();
	return Combat->HitTarget;
}

ECombtState ABlasterCharacter::GetCombatState() const
{
	if(Combat==nullptr) return ECombtState::ECS_MAX;
	return Combat->CombatState;
}

bool ABlasterCharacter::IsLocalReloading()
{
	if(Combat == nullptr) return false;
	return	Combat->bLocallyReloading;
}







