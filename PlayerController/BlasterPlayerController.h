// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate , bool , bPingTooHigh);     //文档关键词:委托
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health,float MaxHealth);
	void SetHUDShield(float Shield,float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(float Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDGrenades(int32 Grenades);
	void SetMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void HideTeamScores();
	void InitializeTeamScores();
	void SetBlueTeamScores(int32 BlueTeamScores);
	void SetRedTeamScores(int32 RedTeamScores);
	
	
	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State,bool bTeamMatch = false);
	void HandleMatchHasStarted(bool bTeamMatch = false);
	void HandleCooldown();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker , APlayerState* Victim);
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	void PollInit();

	virtual void SetupInputComponent() override;

	//服务器与客户端的同步时间
	//请求当前服务器时间，传递客户端发送请求的时间
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//告知客户端当前的服务器时间
	UFUNCTION(Client,Reliable)
	void ClientReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);

	float ClientServerData =0.f;

	UPROPERTY(EditAnywhere,Category=Time)
	float TimeSyncFrecquency =5.f;
	float TimeSyncRunningTime=0.f;

	void CheckTimeSync(float DeltaSecond);

	UFUNCTION(Server,Reliable)
	void CheckServerMatchState();                //检查服务器当前的状态

	UFUNCTION(Client,Reliable)
	void ClientJoinMidgame(FName StateMatch,float Warmup,float Match,float Cooldown,float StartingTime);                         //检查玩家中途加入时的状态

	void PlayHighPingWarning();
	void StopPlayHighPingWaring();
	void CheckPing(float DeltaTime);

	void ShowReturnToMainMenu();

	UFUNCTION(Client,Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker ,APlayerState* Victim);

	UPROPERTY(ReplicatedUsing=OnRep_ShowTeamScore)
	bool bShowTeamScore = false;

	UFUNCTION()
	void OnRep_ShowTeamScore();

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	//菜单
	UPROPERTY(EditAnywhere,Category=HUD)
	TSubclassOf<class UUserWidget> ReturnToMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;    //菜单组件

	bool bReturnToMainMenuOpen = false;         //判断菜单是否打开

	float LevelStartingTime = 0.f;
	float MatchTime =0.f;
	float WarmupTime=0.f;                             //游戏开始前的热身时间
	float CooldownTime=0.f;                             //新对局开启时的准备时间
	uint32 CountdownInt=0;                      //倒计时显示

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	bool bInitializeShield = false;
	float HUDShield;
	float HUDMaxShield;
	bool bInitializeScore = false;
	float HUDScore;
	bool bInitializeDefeats = false;
	int32 HUDDefeats;
	bool bInitializeGrenades = false;
	int32 HUDGrenades;

	float HUDCarriedAmmo;                 //初始武器子弹
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float PingAnimationRunnimgTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server,Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50,f;
};
