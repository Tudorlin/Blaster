// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "PhysicsSettingsEnums.h"
#include "../../../Plugins/Developer/RiderLink/Source/RD/thirdparty/spdlog/include/spdlog/fmt/bundled/format.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/BlasterGameState/BlasterGameState.h"
#include "Blaster/BlasterPlayerState/BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/ReturnToMainMenu.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Net/UnrealNetwork.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	BlasterHUD=Cast<ABlasterHUD>(GetHUD());
	CheckServerMatchState();
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
}

void ABlasterPlayerController::PollInit()
{
	if(CharacterOverlay==nullptr)
	{
		if(BlasterHUD&&BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay=BlasterHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				if(bInitializeHealth)    SetHUDHealth(HUDHealth,HUDMaxHealth);
				if(bInitializeShield)    SetHUDShield(HUDShield,HUDMaxShield);
				if(bInitializeScore)     SetHUDScore(HUDScore);
				if(bInitializeDefeats)   SetHUDDefeats(HUDDefeats);
				if(bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if(bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if(BlasterCharacter&&BlasterCharacter->GetCombat())
				{
					if(bInitializeGrenades)    SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent == nullptr) return;

	InputComponent->BindAction("Quit",IE_Pressed,this,&ABlasterPlayerController::ShowReturnToMainMenu);
}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	if(HasAuthority())  return;
	HighPingRunningTime += DeltaTime;
	if(HighPingRunningTime>CheckPingFrequency)        //高延迟时每隔一段时间显示一次
	{
		PlayerState = PlayerState==nullptr? GetPlayerState<APlayerState>() : PlayerState;
		if(PlayerState)
		{
			// UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPing() * 4 : %d"), PlayerState->GetPing() * 4);
			if(PlayerState->GetPing()*4>HighPingThreshold)
			{
				PlayHighPingWarning();
				PingAnimationRunnimgTime = 0.f;
				ServerRequestServerTime(true);
			}
			else
			{
				ServerRequestServerTime(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if(bHighPingAnimationPlaying)
	{
		PingAnimationRunnimgTime+=DeltaTime;
		if(PingAnimationRunnimgTime>HighPingDuration)
		{
			StopPlayHighPingWaring();         //播放固定时间后停止
		}
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController,MatchState);
	DOREPLIFETIME(ABlasterPlayerController,bShowTeamScore);
}

void ABlasterPlayerController::InitializeTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->BlueTeamScore&&
				BlasterHUD->CharacterOverlay->RedTeamScore&&
					BlasterHUD->CharacterOverlay->ScoreSpacerText;
	if(bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->BlueTeamScore&&
				BlasterHUD->CharacterOverlay->RedTeamScore&&
					BlasterHUD->CharacterOverlay->ScoreSpacerText;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void ABlasterPlayerController::SetBlueTeamScores(int32 BlueTeamScores)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->BlueTeamScore;

	if(bHUDValid)
	{
		FString TextToScore = FString::Printf(TEXT("%d"),BlueTeamScores);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(TextToScore));
	}
}

void ABlasterPlayerController::SetRedTeamScores(int32 RedTeamScores)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->RedTeamScore;

	if(bHUDValid)
	{
		FString TextToScore = FString::Printf(TEXT("%d"),RedTeamScores);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(TextToScore));
	}
}

void ABlasterPlayerController::OnRep_ShowTeamScore()
{
	
}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	if(ReturnToMenuWidget==nullptr) return;      //蓝图中没有添加控件类的时候返回
	if(ReturnToMainMenu == nullptr)              //为源码中的变量赋值
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this,ReturnToMenuWidget);
	}
	if(ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;   //每次打开菜单改变布尔值
		if(bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuDown();
		}
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State,bool bTeamMatch)   //设置各个游戏状态下的实现
{
	MatchState=State;

	if(MatchState==MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState==MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if(MatchState==MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState==MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamMatch)
{
	if(HasAuthority()) bShowTeamScore = bTeamMatch;
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		if(BlasterHUD->CharacterOverlay==nullptr) BlasterHUD->AddCharacterOverlay();
		if(BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if(!HasAuthority()) return;
		if(bTeamMatch)
		{
			InitializeTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid =BlasterHUD->Announcement&&
			BlasterHUD->Announcement->AnnouncementText&&
				BlasterHUD->Announcement->InfoText;

		if(bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Game Started In :");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			ABlasterGameState *BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState=GetPlayerState<ABlasterPlayerState>();
			if(BlasterGameState)
			{
				FString InfoText;
				if(BlasterGameState->TopScoringPlayers.Num()==0)
				{
					InfoText=FString("There is no winner");
				}
				else if(BlasterGameState->TopScoringPlayers.Num()==1&&BlasterGameState->TopScoringPlayers[0]==BlasterPlayerState)
				{
					InfoText=FString("You are the winner!");
				}
				else if(BlasterGameState->TopScoringPlayers.Num()==1)
				{
					InfoText=FString::Printf(TEXT("Winner: \n%s"),*BlasterGameState->TopScoringPlayers[0]->GetPlayerName());
				}
				else if(BlasterGameState->TopScoringPlayers.Num()>1)
				{
					InfoText=FString("Players tied for the win:\n");
					for(auto TiedPlayer : BlasterGameState->TopScoringPlayers)
					{
						InfoText.Append(FString::Printf(TEXT("%s\n"),*TiedPlayer->GetPlayerName()));
					}
				}
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoText));
			}
		}
	}
	ABlasterCharacter* BlasterCharacter=Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter&&BlasterCharacter->GetCombat())
	{
		BlasterCharacter->bDisableGameplay=true;
		BlasterCharacter->GetCombat()->FireButtonPressed(false);
	}
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

	bool bHUDValid=BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->HealthBar&&
				BlasterHUD->CharacterOverlay->HealthText;
	if(bHUDValid)
	{
		const float HealthPercet=Health/MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercet);
		FString HealthText=FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health),FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth=true;
		HUDHealth=Health;
		HUDMaxHealth=MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

	bool bHUDValid=BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->ShieldBar&&
				BlasterHUD->CharacterOverlay->ShieldText;
	if(bHUDValid)
	{
		const float ShieldPercet=Shield/MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercet);
		FString ShieldText=FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Shield),FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield=true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

	bool bHUDValid=BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
				BlasterHUD->CharacterOverlay->ScoreAmount;
	if(bHUDValid)
	{
		FString ScoreText=FString::Printf(TEXT("%d"),FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore=true;
		HUDScore=Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(float Defeats)
{
	BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

	bool bHUDValid=BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
				BlasterHUD->CharacterOverlay->DefeatsAmount;
	if(bHUDValid)
	{
		FString DefeatsText=FString::Printf(TEXT("%d"),FMath::FloorToInt(Defeats));
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats=true;
		HUDDefeats=Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->AmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;	
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeGrenades;
		HUDGrenades=Grenades = true;
	}
}

void ABlasterPlayerController::SetMatchCountdown(float CountdownTime)
{
	BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

	bool bHUDValid=BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
				BlasterHUD->CharacterOverlay->MatchCountdownText;
	if(bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes=FMath::FloorToInt(CountdownTime/60.f);      //向下取整设置分钟
		int32 Seconds=CountdownTime-Minutes*60;           //总时间-分钟*60为秒的数量
		
		FString CountdownText=FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD=BlasterHUD==nullptr?Cast<ABlasterHUD>(GetHUD()):BlasterHUD;

	bool bHUDValid=BlasterHUD&&
		BlasterHUD->Announcement&&
				BlasterHUD->Announcement->WarmupTime;
	if(bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes=FMath::FloorToInt(CountdownTime/60.f);      //向下取整设置分钟
		int32 Seconds=CountdownTime-Minutes*60;           //总时间-分钟*60为秒的数量
		
		FString CountdownText=FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker,Victim);
}

void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();

	if(Attacker&&Victim&&Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			if(Attacker==Self&&Victim!=Self)
			{
				BlasterHUD->AddElimAnnouncement("You",Victim->GetPlayerName());
				return;
			}
			if(Victim == Self&&Attacker!=Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(),"You");
				return;
			}
			if(Attacker == Victim&&Attacker==Self)
			{
				BlasterHUD->AddElimAnnouncement("You","yourself");
				return;
			}
			if(Attacker == Victim&&Attacker!=Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(),"themself");
				return;
			}
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(),Victim->GetPlayerName());
		}
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft =0.f;
	if(MatchState==MatchState::WaitingToStart) TimeLeft=WarmupTime-GetServerTime()+LevelStartingTime;
	else if(MatchState==MatchState::InProgress) TimeLeft=WarmupTime+MatchTime-GetServerTime()+LevelStartingTime;
	else if(MatchState==MatchState::Cooldown) TimeLeft=CooldownTime+WarmupTime+MatchTime-GetServerTime()+LevelStartingTime;
	uint32 SecondLeft=FMath::CeilToInt(TimeLeft);
	if(CountdownInt!=SecondLeft)
	{
		if(MatchState==MatchState::WaitingToStart||MatchState==MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if(MatchState==MatchState::InProgress)
		{
			SetMatchCountdown(TimeLeft);
		}
	}
	CountdownInt=SecondLeft;
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest) 
{
	float ServerTimeofReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest,ServerTimeofReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime =GetWorld()->GetTimeSeconds() - TimeOfClientRequest;  //从客户端发出请求服务器处理完请求后发送回客户端的总时间
	SingleTripTime = 0.5f*RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest+SingleTripTime;  //服务端当前的时间
	ClientServerData = CurrentServerTime-GetWorld()->GetTimeSeconds();
}


float ABlasterPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds()+ClientServerData;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::CheckTimeSync(float DeltaSecond)
{
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::PlayHighPingWarning()
{
	BlasterHUD =BlasterHUD == nullptr? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->HighPingImage&&
				BlasterHUD->CharacterOverlay->HighPingAnimation;

	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation,0.f,5);
	}
}

void ABlasterPlayerController::StopPlayHighPingWaring()
{
	BlasterHUD = BlasterHUD == nullptr? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD&&
		BlasterHUD->CharacterOverlay&&
			BlasterHUD->CharacterOverlay->HighPingImage&&
				BlasterHUD->CharacterOverlay->HighPingAnimation;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ABlasterPlayerController::CheckServerMatchState_Implementation()
{
	ABlasterGameMode* GameMode=Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime=GameMode->WarmupTime;
		MatchTime=GameMode->MatchTime;
		CooldownTime=GameMode->CooldownTime;
		LevelStartingTime=GameMode->LevelStartingTime;
		MatchState=GameMode->GetMatchState();
		ClientJoinMidgame(MatchState,WarmupTime,MatchTime,CooldownTime,LevelStartingTime);
	}
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch,float Warmup,float Match,float Cooldown,float StartingTime)
{
	WarmupTime=Warmup;
	MatchTime=Match;
	CooldownTime=Cooldown;
	LevelStartingTime=StartingTime;
	MatchState=StateOfMatch;
	OnMatchStateSet(MatchState);

	if(BlasterHUD&&MatchState==MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABlasterCharacter* BlasterCharacter=Cast<ABlasterCharacter>(InPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHeath(),BlasterCharacter->GetMaxHealth());
	}
}
