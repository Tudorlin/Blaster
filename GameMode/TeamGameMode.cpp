// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamGameMode.h"

#include "Blaster/BlasterGameState/BlasterGameState.h"
#include "Blaster/BlasterPlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

ATeamGameMode::ATeamGameMode()
{
	
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)  //执行登录后的初始化工作。
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BGameState  = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if(BPState&&BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if(BGameState->BlueTeam.Num()>=BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.Add(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if(BGameState&&BPState)
	{
		if(BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if(BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}
}

float ATeamGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if(AttackerPState ==nullptr||VictimPState==nullptr) return BaseDamage;
	if(AttackerPState == VictimPState)
	{
		return BaseDamage;
	}
	if(AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamGameMode::HandleMatchHasStarted()  //执行游戏开始前的初始化工作。
{
	Super::HandleMatchHasStarted();

	ABlasterGameState *BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BGameState)
	{
		for(auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if(BPState&&BPState->GetTeam()==ETeam::ET_NoTeam)
			{
				if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
