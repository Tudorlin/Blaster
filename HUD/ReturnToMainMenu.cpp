
#include "ReturnToMainMenu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;        //接受鼠标键盘的输入

	UWorld* World = GetWorld();
	if(World)
	{
		PlayerController = PlayerController = nullptr ? World->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if(ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this,&UReturnToMainMenu::ReturnButtonClicked);
	}
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if(MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this,&UReturnToMainMenu::OnDestroySession);
		}
	}
}

void UReturnToMainMenu::MenuDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if(World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	if(ReturnButton&&ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this,&UReturnToMainMenu::ReturnButtonClicked);
	}
	if(MultiplayerSessionsSubsystem&&MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this,&UReturnToMainMenu::OnDestroySession);
	}
}

bool UReturnToMainMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if(World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
	}
	else
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			PlayerController->ClientReturnToMainMenuWithTextReason(FText());
		}
	}
}

void UReturnToMainMenu::OnPlayerLeftGame()   //关闭玩家的游戏会话   真——————离开游戏
{
	UE_LOG(LogTemp, Warning, TEXT("OnPlayerLeftGame()"))
	if(MultiplayerSessionsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem valid"))
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

void UReturnToMainMenu::ReturnButtonClicked()                 //玩家点击返回菜单按钮最先调用的函数
{
	ReturnButton->SetIsEnabled(false);                //禁用按钮，

	UWorld* World = GetWorld();     //从世界获取玩家控制器1，从玩家控制器获取pawn在转为玩家调用ServerLeftGame（），该函数将调用BlasterGamemode类中的PlayerLeftGame函数,从而将玩家从玩家列表中移除
	if(World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if(FirstPlayerController)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if(BlasterCharacter)
			{
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this,&UReturnToMainMenu::OnPlayerLeftGame);   //绑定至Character的委托上
			}
			else     //BlasterCharacter为空则说明控制器没有pawn，正在等待响应，重新启用按钮
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}
