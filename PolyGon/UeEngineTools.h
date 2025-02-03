#pragma once
#include <windows.h>

#include <string>
#include <vector>


typedef void(*DrawAllActorCallBack)(std::string Content,float x,float y);

//绘制骨骼线条
typedef void(*DrawBoneLineCallBack)( float x, float y, float x1, float y1,int r,int g ,int b);








//polygon
const static int64_t GNameOffset = 0x807A100;

const static int64_t GWorldOffset = 0x8279F20;

const static int64_t WorldToScreenOffset = 0x49E8E00;

//先在sdk里搜索 TransformFromBoneSpace,定位到真实call之后
/*
* 
	 if ( BoneIndex != -1 )
	  {
		GetBoneMatrix(pThis, &fMatrix, BoneIndex);
*/

const static int64_t GetBoneMatrixOffset = 0x48CD990;


//搜索LineTraceSingle(  定位尾部真实call ,倒数第二个
const static int64_t LineTraceSingleOffset = 0x4AD8FE0;





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

	template <typename T>
	class TArray
	{
	public:

		void Initialize()
		{
			data = new T[2];
			Count = 0;
			Max = 0;
		}

		void push(T Actor, T Actor1)
		{
			data = new T[2];
			data[0] = Actor;
			data[1] = Actor1;
			Count = 2;
			Max = 2;
		};

		void Pop()
		{
			delete[] data;
		};

		T* data;
		int Count;
		int Max;
	};
	struct Vector2
	{
		float X = 0;
		float Y = 0;
	};

	struct Vector3 : public Vector2
	{
		float Z = 0;
	};

	struct Vector4 : public Vector3
	{
		float W = 0;
	};

	struct FMatrix
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;

		FMatrix operator*(const FMatrix& other);
	};


	struct FTransform
	{
		Vector4    Rotation;                                                   // 0x0000   (0x0010)  
		Vector4    Translation;                                                // 0x0010   (0x000C)  
		Vector4    Scale3D;                                                    // 0x0020   (0x000C)  

		bool ToMatrixWithScale(OUT UeEngineTools::FMatrix& outmatrix);
	};


	struct BoneName
	{
		int NameID;
		int A;
		int B;
	};

	enum BoneFNames {
		Root = 0,
		pelvis = 1,
		spine_01 = 2,
		spine_02 = 3,
		spine_03 = 4,
		neck_01 = 5,
		Head = 6,
		cam_bone = 7,
		clavicle_l = 8,
		upperarm_l = 9,
		lowerarm_l = 10,
		hand_l = 11,
		thumb_01_l = 12,
		thumb_02_l = 13,
		thumb_03_l = 14,
		index_01_l = 15,
		index_02_l = 16,
		index_03_l = 17,
		middle_01_l = 18,
		middle_02_l = 19,
		middle_03_l = 20,
		ring_01_l = 21,
		ring_02_l = 22,
		ring_03_l = 23,
		pinky_01_l = 24,
		pinky_02_l = 25,
		pinky_03_l = 26,
		clavicle_r = 27,
		upperarm_r = 28,
		lowerarm_r = 29,
		hand_r = 30,
		thumb_01_r = 31,
		thumb_02_r = 32,
		thumb_03_r = 33,
		index_01_r = 34,
		index_02_r = 35,
		index_03_r = 36,
		middle_01_r = 37,
		middle_02_r = 38,
		middle_03_r = 39,
		ring_01_r = 40,
		ring_02_r = 41,
		ring_03_r = 42,
		pinky_01_r = 43,
		pinky_02_r = 44,
		pinky_03_r = 45,
		thigh_l = 46,
		calf_l = 47,
		foot_l = 48,
		ball_l = 49,
		thigh_r = 50,
		calf_r = 51,
		foot_r = 52,
		ball_r = 53,
		ik_gun = 54,
		ik_target_r = 55,
		ik_target_l = 56,
		Highest = 57,
		Max = 58
	};



	struct BoneIdx
	{
		BoneIdx()
		{
			head = BoneFNames::Head;
			neck_01 = BoneFNames::neck_01;
			spine_03 = BoneFNames::spine_03;
			spine_02 = BoneFNames::spine_02;
			spine_01 = BoneFNames::spine_01;
			pelvis = BoneFNames::pelvis;

			hand_l = BoneFNames::hand_l;
			lowerarm_l = BoneFNames::lowerarm_l;
			upperarm_l = BoneFNames::upperarm_l;
			clavicle_l = BoneFNames::clavicle_l;
			clavicle_r = BoneFNames::clavicle_r;
			upperarm_r = BoneFNames::upperarm_r;
			lowerarm_r = BoneFNames::lowerarm_r;
			hand_r = BoneFNames::hand_r;

			ball_l = BoneFNames::ball_l;
			foot_l = BoneFNames::foot_l;
			calf_l = BoneFNames::calf_l;
			thigh_l = BoneFNames::thigh_l;
			thigh_r = BoneFNames::thigh_r;
			calf_r = BoneFNames::calf_r;
			foot_r = BoneFNames::foot_r;
			ball_r = BoneFNames::ball_r;
		}

		int head, neck_01, spine_03, spine_02, spine_01, pelvis;
		int hand_l, lowerarm_l, upperarm_l, clavicle_l, clavicle_r, upperarm_r, lowerarm_r, hand_r;
		int ball_l, foot_l, calf_l, thigh_l, thigh_r, calf_r, foot_r, ball_r;
	};




	class USceneComponent
	{
	public:
		UCHAR pa_00[0x1C0];

		FTransform ComponentToWorld;
	};


	class USkeletalMesh
	{
	public:
		char pa_00[0x1b0];

		TArray<BoneName> Names;
	};

	class USkinnedMeshComponent : public USceneComponent
	{
	public:
		char pa_1F0[0x290];
		USkeletalMesh* SkeletalMesh; //骨骼名称 480
		char pa_488[0x28];
		TArray<FTransform> BoneTransform; //+ 4b0
	};


	class USkeletalMeshComponent : public USkinnedMeshComponent
	{

	};

	struct FCameraCacheEntry
	{
		float                                              timestamp;                                                  // 0x0000   (0x0004)  
		unsigned char                                      UnknownData00_6[0xC];                                       // 0x0004   (0x000C)  MISSED
		Vector3											   POVPos;                                                        // 0x0010   (0x05F0)  
	};

	class AActor
	{
	public:
		UCHAR pa_00[0x130];
		USceneComponent* RootComponent;
	};



	class APlayerCameraManager : public AActor
	{
	public:
		char pa_138[0x19A8];
		FCameraCacheEntry      CameraCachePrivate;   // 0x1AE0   (0x0600)

	};

	class APawn;
	class AController : public AActor
	{
	public:
		UCHAR pa_138[0x118];
		APawn* Pawn; //0x250 指向本地玩家
	};

	class APlayerController : public AController
	{
	public:
		char pa_258[0x60];
		 APlayerCameraManager* PlayerCameraManager;                                        // 0x02B8   (0x0008)
	};



	class APawn : public AActor
	{
	public:
		UCHAR pa_138[0x120];
		AController* Controller; //+0x258
		UCHAR pa_260[0x20];
	};


	class ACharacter : public APawn
	{
	public:
		USkeletalMeshComponent* Mesh; //实际是在ACharacter下面  +0x280
	};


	class ULevel
	{
	public:

		UCHAR pa_00[0x98];
		TArray<ACharacter*> Actors;

	};

	class UPlayer
	{
	public:
		unsigned char      UnknownData00_8[0x30];     
		APlayerController* PlayerController;  // 0x0030   (0x0008)  

	};


	class ULocalPlayer : public UPlayer
	{
	public:

	};


	class UGameInstance
	{
	public:
		APlayerController* GetPlayerController();
		UeEngineTools::AActor* GetPlayer();

		unsigned char             pa_00[0x38];
		TArray<ULocalPlayer*>        LocalPlayers;   // 0x0038   (0x0010)  

	};



	class UWorld
	{
	public:
		unsigned char                                      pa_00[0x30];
		class ULevel* PersistentLevel;                                            // 0x0030   (0x0008)  当前关卡
		unsigned char                                      pa_38[0x180];
		UGameInstance* OwningGameInstance;  // 修复!

	};


	UWorld* GetGWorld();


	void GetBoneIndex(UeEngineTools::ACharacter* pActor, BoneIdx* boneidx);

	Vector3 GetBoneMatrix(USkeletalMeshComponent* Mesh,int BoneIndex);

	//世界坐标转屏幕
	bool ProjectWorldLocationToScreen(Vector3 WorldLocation,OUT Vector2& ScreenLocation);

	//绘制部分骨骼
	void DrawPartBone(int Start, int End, ACharacter* pActor, UeEngineTools::BoneIdx* Idx, int r, int g, int b, DrawBoneLineCallBack _callback);
	//绘制单个Actor骨骼
	void DrawActorAllBone(ACharacter* pActor, UeEngineTools::BoneIdx* Idx, int r, int g, int b,DrawBoneLineCallBack _callback);
	//绘制所有Actor骨骼
	void DrawAllActorsBone(DrawBoneLineCallBack _callback);
	void DrawAllActors(DrawAllActorCallBack _callback);

	struct FHitResult
	{
		char pa_00[0x00E8];
	};

	struct FLinearColor
	{
		float R, G, B, A;
	};
	bool LineTraceSingle(Vector3 Start, Vector3 End, TArray<AActor*>& ActorsToIgnore);

	void DrawTraceSingle(DrawBoneLineCallBack _callback);
};

