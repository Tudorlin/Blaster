#include "LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Blaster/Blaster.h"
#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFramePackage();
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;                  //总长度是链表首尾的时间差，用作分母
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time)/Distance,0.f,1.f); //击中时间减去储存的最迟时间作分子

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for(auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;             //从头到位遍历整个链表，读取HitBox的Key获得需要的HitBox

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];          //获取指定HitBox的此时储存的信息
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;

		//插值
		
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	return InterpFramePackage;	
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr) return FServerSideRewindResult();
	
	FFramePackage CurrentFrame;                               //保存当前HitBox的信息，在ResetBoxes作参数
	CacheBoxPositions(HitCharacter,CurrentFrame);
	MoveBoxes(HitCharacter,Package);
	EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::NoCollision);
	
	UBoxComponent* HeadBox = HitCharacter->HitBoxCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);//暂时启用HitBox的碰撞
	
	FHitResult ConfirmHitResult;            //射线检测的结果
	const FVector TraceEnd = TraceStart+(HitLocation-TraceStart)*1.25;
	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox);
		if(ConfirmHitResult.bBlockingHit)      //命中说明爆头,不需要检测其他位置的HitBox
		{
			// if (ConfirmHitResult.Component.IsValid())
			// {
			// 	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
			// 	if (Box)
			// 	{
			// 		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
			// 	}
			// }
			ResetHitBoxes(HitCharacter,CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true,true};
		}
		else                   //没有爆头时检查其他HitBox
		{
			for(auto& HitBoxPair : HitCharacter->HitBoxCollisionBoxes)
			{
				if(HitBoxPair.Value!=nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,	
				TraceEnd,
				ECC_HitBox);
			if(ConfirmHitResult.bBlockingHit)
			{
				// if (ConfirmHitResult.Component.IsValid())
				// {
				// 	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				// 	if (Box)
				// 	{
				// 		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				// 	}
				// }
				ResetHitBoxes(HitCharacter,CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true,false};
			}
		}
	}
	ResetHitBoxes(HitCharacter,CurrentFrame);                              //未命中
	EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter,CurrentFrame);
	MoveBoxes(HitCharacter,Package);
	EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::NoCollision);

	UBoxComponent* HeadBox = HitCharacter->HitBoxCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);
	
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if(PathResult.HitResult.bBlockingHit)
	{
		if(PathResult.HitResult.Component.IsValid())
		{
			// UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			// if(Box)
			// {
			// 	DrawDebugBox(GetWorld(),Box->GetComponentLocation(),Box->GetScaledBoxExtent(),
			// 		FQuat(Box->GetComponentRotation()),FColor::Red,false,10.f);
			// }
			ResetHitBoxes(HitCharacter,CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter,ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true,true};
		}
	}
	else
	{
		for (auto& HitBoxPair : HitCharacter->HitBoxCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			// if (PathResult.HitResult.Component.IsValid())
			// {
			// 	UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			// 	if (Box)
			// 	{
			// 		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
			// 	}
			// }
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, false };
		}
	}
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for(auto& Frame : FramePackages)
	{
		if(Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}
	TArray<FFramePackage> CurrentFrames;
	for(auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for(auto& Frame : FramePackages)
	{
		UBoxComponent* HeadBox = Frame.Character->HitBoxCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);//暂时启用HitBox的碰撞
	}
	FShotgunServerSideRewindResult ShotgunResult;              //返回值
	UWorld * World = GetWorld();
	//爆头
	for(auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation-TraceStart)*1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
				);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if(BlasterCharacter)
			{
				// if (ConfirmHitResult.Component.IsValid())
				// {
				// 	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				// 	if (Box)
				// 	{
				// 		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				// 	}
				// }
				if(ShotgunResult.HeadShots.Contains(BlasterCharacter))
				{
					ShotgunResult.HeadShots[BlasterCharacter]++;  //统计霰弹击中的人数
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter,1);  //否则就只击中一个
				}
			}
		}
	}
	for(auto& Frame : FramePackages)
	{
		for(auto& HitBoxPair : Frame.Character->HitBoxCollisionBoxes)
		{
			if(HitBoxPair.Value!=nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);
			}
			UBoxComponent* HeadBox = Frame.Character->HitBoxCollisionBoxes[FName("head")];
			HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	//只击中身体
	for(auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart)*1.25f;
		if(World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if(BlasterCharacter)
			{
				// if (ConfirmHitResult.Component.IsValid())
				// {
				// 	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				// 	if (Box)
				// 	{
				// 		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				// 	}
				// }
				if(ShotgunResult.BodyShots.Contains(BlasterCharacter))
				{
					ShotgunResult.BodyShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter,1);
				}
			}
		}
	}
	for(auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character,Frame);
		EnableCharacterMeshCollision(Frame.Character,ECollisionEnabled::QueryAndPhysics);
	}
	return ShotgunResult;
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePack)  //传递HitBox的信息
{
	
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePack.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)            //移动HitBox到插值的位置
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)        //HitBox返回
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)                                                   //快速设置静态网格的碰撞
{
	if(HitCharacter&&HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& FramePackage, const FColor& Color)
{
	for (auto& BoxInfo : FramePackage.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideReWind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter,HitTime);
	return ConfirmHit(FrameToCheck,HitCharacter,TraceStart,HitLocation);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	float HitTime)  //需要服务器倒带时处理客户端发送的击中请求并造成伤害
{
	FServerSideRewindResult Confirm = ServerSideReWind(HitCharacter,TraceStart,HitLocation,HitTime);

	if(Character&&HitCharacter&&Character->GetEquippedWeapon()&&Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass());
	}
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter,HitTime);
	return ProjectileConfirmHit(FrameToCheck,HitCharacter,TraceStart,InitialVelocity,HitTime);
}

void ULagCompensationComponent::ProjectileScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
    FServerSideRewindResult ProjectileConfirm = ProjectileServerSideRewind(HitCharacter,TraceStart,InitialVelocity,HitTime);

	if(Character&&HitCharacter&&ProjectileConfirm.bHitConfirmed&&Character->GetEquippedWeapon())
	{
		const float Damage = ProjectileConfirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
			);
	}
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for(auto& HitCharacter : HitCharacters)
	{
		if(Character==nullptr||HitCharacter==nullptr||HitCharacter->GetEquippedWeapon()==nullptr) continue;
		float TotalDamage = 0.f;
		if(Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter]*HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if(Confirm.BodyShots[HitCharacter])
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter]*HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass());
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)    //查找需要检查命中的HitBox的时空信息
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;
	if(bReturn) return FFramePackage();

	FFramePackage FrameToCheck;                          
	bool bShouldInterpolate = true;               //是否插值    默认值false会导致命中时引擎崩溃,FrameToCheck为空值

	//被击玩家的HitBox信息,链表的尾结点储存时间的值是链表中最小的
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryFrame = History.GetTail()->GetValue().Time;
	const float NewestHistoryFrame = History.GetHead()->GetValue().Time;

	if(OldestHistoryFrame > HitTime )    //击中时间过于遥远，不需要服务器倒带
		{
			return FFramePackage(); 
		}

	if(OldestHistoryFrame == HitTime) //击中的时间刚好等于链表中尾结点值的时间
		{
			FrameToCheck = History.GetTail()->GetValue();
			bShouldInterpolate = false;
		}

	if(NewestHistoryFrame <= HitTime)//击中时间比最新储存进链表的值的时间还早或者刚好相等时，将最新时间的结点作为查找的结点
		{
			FrameToCheck = History.GetHead()->GetValue();
			bShouldInterpolate = false;
		}
		
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while (Older->GetValue().Time>HitTime)
	{
		if(Older->GetNextNode()==nullptr) break;
		Older = Older->GetNextNode();
		if(Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if(Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if(bShouldInterpolate)
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(),Younger->GetValue(),HitTime);
	}

	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompensationComponent::SaveFramePackage()            //储存角色的HitBox信息
{
	if(Character==nullptr||!Character->HasAuthority()) return;
	if(FrameHistory.Num()<=1)                  //储存的HitBox位置信息少于1时直接添加
		{
			FFramePackage ThisFrame;
			SaveFramePackage(ThisFrame);
			FrameHistory.AddHead(ThisFrame);
		}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)                    //只记录四秒内的HitBox信息
			{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
			}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		
		//ShowFramePackage(ThisFrame,FColor::MakeRandomColor());                      //debug用
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)       //将延迟补偿组件的所有者Character身上的HitBox信息储存到FramePackage中
{
	Character = Character==nullptr? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for(auto& BoxPair : Character->HitBoxCollisionBoxes)  //BoxPair对应HitBox,HitBoxCollisionBoxs中包含了按FName映射的Box信息
			{
				FBoxInformation BoxInformation;
				BoxInformation.Location = BoxPair.Value->GetComponentLocation();
				BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
				BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
				Package.HitBoxInfo.Add(BoxPair.Key,BoxInformation);
			}
	}
}