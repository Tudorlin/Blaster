// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/BlasterPlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}
void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if(TopScoringPlayers.Num()==0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore=ScoringPlayer->GetScore();
	}
	if(ScoringPlayer->GetScore()==TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);//去重添加
	}
	if(ScoringPlayer->GetScore()>TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore=ScoringPlayer->GetScore();
	}}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;
}

void ABlasterGameState::OnRep_RedTeamScore()
{
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
}
