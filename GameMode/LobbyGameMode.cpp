// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();                           //获取等待大厅的人数
	if(NumberOfPlayer==2)            //当大厅中的人数足够时自动进入地图
	{
		UWorld* World=GetWorld();
		if(World)
		{
			bUseSeamlessTravel=true;
			/*地图切换总结：https://www.dandelioncloud.cn/article/details/1533788088698695682*/
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));

			//可以添加一个空的Transition level作为贴图加载的缓冲关卡
		}
	}
}
