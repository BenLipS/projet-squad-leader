#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MainMenu/PlayerData/PlayerData.h"
#include "MainMenu/PlayerParam/PlayerParam.h"
#include "MainMenu/GameParam/GameParam.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "SquadLeaderGameInstance.generated.h"


/**
 * 
 */
UCLASS()
class SQUADLEADER_API USquadLeaderGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USquadLeaderGameInstance();
	virtual void Shutdown() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UserDataLocation")
		FString BaseServerDataAdress;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UserDataLocation")
		FString UserDataFilename;
	
protected:
	void OnStart();

private:
	FHttpModule* Http;

	PlayerData UserData;
	FString LocalIPAdress;

	FString AuthToken;

	bool OnlineStatus;
	
public:
	UFUNCTION(BlueprintCallable)
		void LaunchGame();
	UFUNCTION(BlueprintCallable)
		void SetGameParamToDefault();
	UFUNCTION(BlueprintCallable)
		void SetGameParamToRandom();
	UFUNCTION(BlueprintCallable)
		void SaveGameParam(TMap<FString, int> IntData, TMap<FString, FString> StringData);
	
	UFUNCTION(BlueprintCallable)
		void JoinGame(FString IPAdress);
	UFUNCTION(BlueprintCallable)
		void ProfileInfo();
	UFUNCTION(BlueprintCallable)
		void GetGameAvailable();

	UFUNCTION(BlueprintCallable)
		bool const GetNetworkStatus() { return OnlineStatus; }

	bool UpdateNetworkStatus(const int MatchResult, float GameDuration, int XP, int NbKillAI, int NbKillPlayer, int NbDeathByAI, int NbDeathByPlayer);

private:
	void NoConnexionComportment();

	void HttpCallPing();
	void HttpCallCreateUser();
	void HttpCallConnectUser();
	void HttpCallSendSyncData();
	void HttpCallReceiveSyncData();

	void HttpCallCreateNewGame();
	void HttpCallSetUpNewGame();
	void HttpCallAllowFriendForGame();
	void HttpCallDeleteGame();

	void HttpCallGetGame();

	void HttpCallChangeConnectedStatus(int status);

	void HttpCallUpdatePlayerAfterGame();
	
private:
	void OnResponseReceivedPing(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseReceivedCreateUser(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseReceivedConnectUser(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseReceivedReceiveSync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseCreateNewGame(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseDeleteGame(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseGetGame(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseDoNothing(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf <AGameParam> GameParam;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf <APlayerParam> PlayerParam;

private:
	FString GameID = "";  // only define if a game is create
};
