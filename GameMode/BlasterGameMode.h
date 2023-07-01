// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern  BLASTER_API const FName Cooldown;                    //一小局游戏结束
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(class ABlasterCharacter* EliemmedCharacter,class ABlasterPlayerController* VictimController,ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter,AController* ElimmedController);

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	
	UPROPERTY(EditAnywhere)
	float WarmupTime =10.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime =120.f;
	float LevelStartingTime=0.f;

	UPROPERTY(EditAnywhere)
	float CooldownTime = 10.f;

	bool bTeamMatch = false;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
};
