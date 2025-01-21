#pragma once
#include <windows.h>

#include <string>
#include <vector>



//死寂
const static int64_t GNameOffset = 0x4A99140;//喊话base

const static int64_t GWorldOffset = 0x4C1D800;//喊话base

const static int64_t WorldToScreenOffset = 0x2BE5730;//喊话base


namespace UeEngineTools
{
#define  NAME_SIZE 1024
	static constexpr UINT FNameMaxBlockBits = 13;

	static constexpr UINT FNameBlockOffsetBits = 16;

	static constexpr UINT FNameBlockOffsets = 1 << FNameBlockOffsetBits;

	static constexpr UINT FNameMaxBlocks = 1 << FNameMaxBlockBits;


	struct FNameEntryHeader
	{
		UINT16 bIsWide : 1;
		static constexpr UINT32 ProbeHashBits = 5;
		UINT16 LowercaseProbeHash : ProbeHashBits;
		UINT16 Len : 10;
	};

	struct FNameEntry
	{

		FNameEntryHeader Header;
		union
		{
			char	AnsiName[NAME_SIZE];
			WCHAR	WideName[NAME_SIZE];
		};
	};


	struct FNameEntryId
	{

		UINT Value;
	};

	struct FName
	{

		FNameEntryId	ComparisonIndex;
	};


	class FNameEntryAllocator
	{
	public:
		void* Lock;
		int CurrentBlock = 0;
		int CurrentByteCursor = 0;
		UCHAR* Blocks[FNameMaxBlocks] = {}; //+0x10
	};

	class FNamePool  //GName对象
	{
	public:
		FNameEntryAllocator Entries;


	};


	class UObjectBase
	{
	public:
		void* vtable; 

		int	ObjectFlags; // + 8
		int	InternalIndex; //+c

		void* ClassPrivate;  //+0x10

		/** Name of this object */
		FName							NamePrivate;  //+0x18

	};

	//ue5
	void IdToBlackAndOffset(UINT id, OUT UINT& block, OUT UINT& offset);

	FNameEntryId GetDisplayIndexInternal(UObjectBase* objadr);


	FNamePool* GetGName();

	std::string GetName(UObjectBase* obj);

	std::string GetName(int nID);




//dump UWorld

	template<typename T>
	class TArray
	{
	public:
		T* data;
		int Count;
		int Max;
	};

	struct Vector2
	{
		float x;
		float y;
	};

	struct Vector3 : public Vector2
	{
		float z;
	};

	struct Vector4 : public Vector3
	{
		float w;
	};


	struct FTransform
	{
		Vector4    Rotation;                                                   // 0x0000   (0x0010)  
		Vector4    Translation;                                                // 0x0010   (0x000C)  
		Vector4    Scale3D;                                                    // 0x0020   (0x000C)  

	};



	class USceneComponent
	{
	public:
		UCHAR unknowdata[0x1C0];

		FTransform ComponentToWorld;
	};


	class AActor
	{
	public:
		UCHAR unknowdata[0x130];
		class USceneComponent* RootComponent;
	};


	class ULevel
	{
	public:

		UCHAR unknowdata[0x98];
		TArray<AActor*> Actors;

	};

	class APlayerController
	{
	public:

	};

	class UPlayer
	{
	public:
		unsigned char                                      UnknownData00_8[0x30];     
		APlayerController* PlayerController;                                           // 0x0030   (0x0008)  

	};


	class ULocalPlayer : public UPlayer
	{
	public:

	};


	class UGameInstance
	{
	public:
		APlayerController* GetPlayerController();

		unsigned char             UnknownData00_8[0x38];                                   
		TArray<ULocalPlayer*>        LocalPlayers;   // 0x0038   (0x0010)  

	};



	class UWorld
	{
	public:
		unsigned char                                      UnknownData[0x30];
		class ULevel* PersistentLevel;                                            // 0x0030   (0x0008)  当前关卡
		unsigned char                                      UnknownData2[0x148];
		UGameInstance* OwningGameInstance;  // 0x180

	};


	UWorld* GetGWorld();


	bool ProjectWorldLocationToScreen(Vector3 WorldLocation,OUT Vector2& ScreenLocation);

	void DrawAllActors();
};

