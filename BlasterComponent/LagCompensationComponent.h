// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)           //HitBox信息
struct FBoxInformation
{
	GENERATED_BODY()
	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;   //储存击中时间

	UPROPERTY()
	TMap<FName,FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlasterCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT()
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<ABlasterCharacter*,uint32> HeadShots;

	UPROPERTY()
	TMap<ABlasterCharacter*,uint32> BodyShots; 
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& FramePackage,const FColor& Color);

	//HitScan
	FServerSideRewindResult ServerSideReWind(
		class ABlasterCharacter *HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	//Projectile
	FServerSideRewindResult ProjectileServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
		);
	
	UFUNCTION(Server,Reliable)
	void ProjectileScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
		);

	//Shotgun
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime);

	UFUNCTION(Server,Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
		);
	
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame,const FFramePackage& YoungerFrame,float HitTime);
	void CacheBoxPositions(ABlasterCharacter* HitCharacter,FFramePackage& OutFramePack);
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ABlasterCharacter* HitCharacter,const FFramePackage& Package);
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
    void SaveFramePackage();
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter,float HitTime);

	//HitScan
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	//Projectile
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
	//Shotgun

	FShotgunServerSideRewindResult ShotgunConfirmHit(
			const TArray<FFramePackage>& FramePackages,
			const FVector_NetQuantize& TraceStart,
			const TArray<FVector_NetQuantize>& HitLocations
		);

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;    //双链表储存HitBox信息

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:	
};
