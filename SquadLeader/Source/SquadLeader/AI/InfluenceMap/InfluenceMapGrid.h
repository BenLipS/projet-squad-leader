// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/Info.h"
#include "InfluenceMapGrid.generated.h"

USTRUCT()
struct SQUADLEADER_API FNeighboor {

	GENERATED_USTRUCT_BODY()

		FNeighboor() {
	}

	TArray<int> m_neighboor;

};

UENUM()
enum Type {
	Soldier UMETA(DisplayName = "Soldier"),
	ControlArea UMETA(DisplayName = "ControlArea"),
	Projectile UMETA(DisplayName = "Projectile"),
	Obstacle UMETA(DisplayName = "Obstacle"),
	None UMETA(DisplayName = "None"),
};

UENUM()
enum TileState {
	Free UMETA(DisplayName = "Free"),
	Block UMETA(DisplayName = "Block"),
};

USTRUCT()
struct SQUADLEADER_API FGridPackage {

	GENERATED_USTRUCT_BODY()
		FGridPackage() {}

	int team_value = 0;
	FVector m_location_on_map;
	TEnumAsByte<Type> m_type;
	uint32 ActorID;
};

USTRUCT()
struct SQUADLEADER_API FGridPackageObstacle : public FGridPackage {
	GENERATED_USTRUCT_BODY()
		FGridPackageObstacle() {}

	TArray<FVector> Locations;
};

USTRUCT()
struct SQUADLEADER_API FInfluenceTeamData {
	GENERATED_USTRUCT_BODY()

		FInfluenceTeamData() {}

	float InfluenceValue = 0.0f;

	//The list of the different type on the tile(Soldier, COntrolArea, Projectile, etc.)
	TArray<TEnumAsByte<Type>> Types;

	TArray<uint32> ActorsID;
};

/*
* This struct represent a Tile-Base for the grid
*
*/
USTRUCT()
struct SQUADLEADER_API FTileBase {

	GENERATED_USTRUCT_BODY()

		FTileBase() {
	}

	//the position of the tile in the world
	FVector Location;

	FNeighboor Neighboors;

	TileState State;

	TMap<uint8, FInfluenceTeamData> InfluenceTeam;
};

USTRUCT()
struct SQUADLEADER_API FActorData {
	GENERATED_USTRUCT_BODY()
public:
	FActorData() {}
public:
	uint32 ActorID;
	TArray<uint32> IndexInfluence;
public:
	void AddIndexs(const uint32 index) noexcept { IndexInfluence.Add(index); }
};

/**
 * This class will create a Grid for the influence Map
 * It will also contains the Infleunce Map
 */
UCLASS(Blueprintable)
class SQUADLEADER_API AInfluenceMapGrid : public AInfo
{
	GENERATED_BODY()

public:
	AInfluenceMapGrid();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void ReceivedMessage(FGridPackage _message);
	void ReceivedMessage(FGridPackageObstacle Message);

public:
	float GetValue(const FVector2D Location, const uint8 Team);
private:
	/*
	* will initialize the array with the tile needed
	*/
	void InitGrid() noexcept;

	/*
	* will check if the location is good
	* we do that with the help of the nav-mesh
	*/
	bool IsValid(FVector _location) const;

	/*
	* Draw the grid
	* only use this for debug
	*/
	void DrawGrid() const;

	/*
	* Update the grid with world information
	*/
	void UpdateGrid() noexcept;

	/*
	* Return the Index in the grid of a certain _location
	*/
	bool FindTileIndex(const FVector _location, uint32& Index) const noexcept;

	bool FindIndexModifyinTeam1(const FVector2D Location, uint32& Index);
	bool FindIndexModifyinTeam2(const FVector2D Location, uint32& Index);

	/*
	* Check if _location is on the tile who's in tile_location
	*/
	bool IsOnTile(FVector _location, FVector tile_location) const noexcept;

	bool IsOnTileUpdate(const FVector2D Location, const FVector TileLocation) const noexcept;

	/*
	* Find the neighboor for all tile
	*/
	void FindNeighboor() noexcept;

	/*
	* Find the neighboor for a specific tile
	*/
	void Neighboors(int index) noexcept;

	/*
	* Check if two tile are neighboor
	*/
	void IsNeighboor(int index_1, int index_2) noexcept;

	/*
	* Algorithm recursif
	* calculate the influence of player on the grid
	*/
	void CalculateSoldierInfluence(int index, int start_index, int source_index, int distance, uint32 actorID, uint32 Team, uint16& ActorDataIndex) noexcept;

	/*
	* Calculate the influence of a control area
	*/
	void CalculateControlAreaInfluence(int index, int start_index, int source_index, int distance, uint32 actorID, uint32 Team, uint16& ActorDataIndex) noexcept;

	void CalculateProjectileInfluence(int index, int start_index, int source_index, int distance, uint32 actorID, uint32 Team, uint16& ActorDataIndex) noexcept;

	/*
	* Calculate the time of execution of a function
	*/
	void TimeFunction();

	void UpdateTile(int index, float value, int team, Type type, uint32 actorID, uint16& ActorDataIndex) noexcept;

	void AddUpdateTileTeam1(const uint32 index);
	void AddUpdateTileTeam2(const uint32 index);

public:

	//Dimension of the grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
		int m_grid_width = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
		int m_grid_height = 100000;

	//Dimension of a tile-base
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
		int m_tile_width = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
		int m_tile_height = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
		float m_height = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
		FVector m_startLocation = FVector{ 0.f,0.f,0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
		TArray<float> m_heightList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool m_DrawAllGrid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool m_DrawCharacterInfluence = false;

private:

	/*
	* Array of the tile-base for the influence map
	*/
	UPROPERTY()
		TArray<FTileBase> Grid;

	//The number of tile in the end
	int nbr_tile = 0;

	/*
	* Will contains the index of the tile that need
	* to be update
	* with that we don't need tu update all the grid
	* we pop out an index when he's update value is down to 0
	*/
	UPROPERTY()
		TArray<uint32> m_index_update;

	UPROPERTY()
		TArray<uint32> m_index_team1;
	UPROPERTY()
		TArray<uint32> m_index_team2;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Influence Value")
		float ProjectileInfluenceValue = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Influence Value")
		int ProjectileAreaInfluence = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Influence Value")
		float ControlAreaInfluenceValue = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Influence Value")
		int ControlAreaAreaInfluence = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Influence Value")
		float CharacterInfluenceValue = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Influence Value")
		int CharacterAreaInfluence = 8;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update Tick")
		int TickUpdate = 5;

	UPROPERTY()
		int CurrentTick = 5;

protected:

	UPROPERTY()
		TArray<FActorData> ActorsData;

	bool ActorAlreadyExist(const uint32 ActorID, uint16& Index) const;

	void DeleteInfluence(const uint16 IndexActor, const uint8 Team) noexcept;

	void UpdateControlArea(const uint16 IndexControlArea, const uint8 Team, const uint32 ControlAreID, uint16 IndexActor) noexcept;
	void UpdateSoldier(const uint16 IndexSoldier, const uint8 Team, const uint32 SoldierID) noexcept;

	void SoldierInfluence(FGridPackage Message, uint32 IndexTile, uint16 IndexActor);
	void ControlAreaInfluence(FGridPackage Message, uint32 IndexTile, uint16 IndexActor);
	void ProjectileInfluence(FGridPackage Message, uint32 IndexTile, uint16 IndexActor);
	void ObstacleInfluence(FGridPackageObstacle Message, uint32 IndexTile, uint16 IndexActor);

public:
	void EraseObstacleInfluence(FGridPackageObstacle Message);
	void EraseSoldierInfluence(FGridPackage Message);

	double GetInfluenceAverage(const uint32 ActorID, const uint8 Team);


};
