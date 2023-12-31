// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blaster/BlasterGameState/BlasterGameState.h"
#include "Blaster/BlasterPlayerState/BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
 {
	bDelayedStart=true;
 }

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime=GetWorld()->GetTimeSeconds();
	
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It)   //用迭代器遍历游戏中的Controller
	{
		ABlasterPlayerController *BlasterPlayerController = Cast<ABlasterPlayerController>(*It);

		if(BlasterPlayerController)
		{
			BlasterPlayerController->OnMatchStateSet(MatchState,bTeamMatch);
		}
	}
}


void ABlasterGameMode::Tick(float DeltaSeconds)
 {
 	Super::Tick(DeltaSeconds);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}

	else if(MatchState==MatchState::InProgress)
	{
		CountdownTime=WarmupTime+MatchTime-GetWorld()->GetTimeSeconds()+LevelStartingTime;
		if(CountdownTime<=0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState==MatchState::Cooldown)
	{
		CountdownTime=CooldownTime+WarmupTime+MatchTime-GetWorld()->GetTimeSeconds()+LevelStartingTime;
		if(CountdownTime<=0.f)
		{
			RestartGame();
		}
	}
 }

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,
                                        ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if(AttackerController==nullptr||AttackerController->PlayerState==nullptr) return;
	if(VictimController==nullptr||VictimController->PlayerState==nullptr) return;
	ABlasterPlayerState* AttackerPlayerState=AttackerController?Cast<ABlasterPlayerState>(AttackerController->PlayerState):nullptr;
	ABlasterPlayerState* VictimPlayerState=VictimController?Cast<ABlasterPlayerState>(VictimController->PlayerState):nullptr;
	
	ABlasterGameState* BlasterGameState =GetGameState<ABlasterGameState>();
	
	if(AttackerPlayerState && AttackerPlayerState!=VictimPlayerState&&BlasterGameState&&BlasterGameState)
	{
		TArray<ABlasterPlayerState*>PlayerCurrentlyInTheLead;    //BlasterPlayerState中保存了玩家的分数和被击败的次数
		for(auto LeadPlayer : BlasterGameState->TopScoringPlayers)    //TopScoringPlayer是BlasterPlayerState类型的数组,将该数组中的元素添加到刚创建的数组中
			{
			PlayerCurrentlyInTheLead.Add(LeadPlayer);
			}
		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);     //玩家得分之后检查是否为得分最高者
		if(BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if(Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}
		for(int32 i = 0;i<PlayerCurrentlyInTheLead.Num();i++)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(PlayerCurrentlyInTheLead[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayerCurrentlyInTheLead[i]->GetPawn());
				if(Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}

	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;It++)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayer&&AttackerPlayerState&&VictimPlayerState)
		{
			BlasterPlayer->BroadcastElim(AttackerPlayerState,VictimPlayerState);
		}
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if(ElimmedController)
	{ 
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,APlayerStart::StaticClass(),PlayerStarts);
		int32 Selection =FMath::RandRange(0,PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(ElimmedController,PlayerStarts[Selection]);
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if(PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if(PlayerLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}
