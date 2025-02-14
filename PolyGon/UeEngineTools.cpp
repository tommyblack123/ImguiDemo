#include "pch.h"
#include "UeEngineTools.h"
#include <intrin.h>

void UeEngineTools::IdToBlackAndOffset(UINT id, OUT UINT &block, OUT UINT &offset)
{

	block = id >> FNameBlockOffsetBits;
	offset = id  &(FNameBlockOffsets - 1);

}

UeEngineTools::FNameEntryId UeEngineTools::GetDisplayIndexInternal(UObjectBase* obj)
{
	return obj->NamePrivate.ComparisonIndex; // obj+ 0x18
}

UeEngineTools::FNamePool* UeEngineTools::GetGName()
{
	
	LONG_PTR exebase = (LONG_PTR)GetModuleHandle(nullptr);


	return (UeEngineTools::FNamePool*)(exebase + GNameOffset);
}

std::string UeEngineTools::GetName(UObjectBase* obj)
{
	UeEngineTools::FNameEntryId _id = GetDisplayIndexInternal(obj);

	return GetName(_id.Value);
}

UeEngineTools::UWorld* UeEngineTools::GetGWorld()
{
	LONG_PTR exebase = (LONG_PTR)GetModuleHandle(nullptr);

	return *(UeEngineTools::UWorld**)(exebase + GWorldOffset);

}

void UeEngineTools::GetBoneIndex(UeEngineTools::ACharacter* pActor,BoneIdx *Idx)
{
	for (int i = 0; i < pActor->Mesh->SkeletalMesh->Names.Count; i++)
	{
		auto BoneName = GetName(pActor->Mesh->SkeletalMesh->Names.data[i].NameID);


		if (BoneName == "Head") { Idx->head = i; }
		else if (BoneName == "hand_l") { Idx->hand_l = i; }
		else if (BoneName == "hand_r") { Idx->hand_r = i; }
		else if (BoneName == "ball_l") { Idx->ball_l = i; }
		else if (BoneName == "ball_r") { Idx->ball_r = i; }
		else if (BoneName == "foot_l") { Idx->foot_l = i; }
		else if (BoneName == "calf_l") { Idx->calf_l = i; }
		else if (BoneName == "calf_r") { Idx->calf_r = i; }
		else if (BoneName == "foot_r") { Idx->foot_r = i; }
		else if (BoneName == "pelvis") { Idx->pelvis = i; }
		else if (BoneName == "neck_01") { Idx->neck_01 = i; }
		else if (BoneName == "thigh_l") { Idx->thigh_l = i; }
		else if (BoneName == "thigh_r") { Idx->thigh_r = i; }
		else if (BoneName == "spine_03") { Idx->spine_03 = i; }
		else if (BoneName == "spine_02") { Idx->spine_02 = i; }
		else if (BoneName == "spine_01") { Idx->spine_01 = i; }
		else if (BoneName == "lowerarm_l") { Idx->lowerarm_l = i; }
		else if (BoneName == "lowerarm_r") { Idx->lowerarm_r = i; }
		else if (BoneName == "clavicle_l") { Idx->clavicle_l = i; }
		else if (BoneName == "clavicle_r") { Idx->clavicle_r = i; }
		else if (BoneName == "upperarm_l") { Idx->upperarm_l = i; }
		else if (BoneName == "upperarm_r") { Idx->upperarm_r = i; };

	}
}

UeEngineTools::Vector3 UeEngineTools::GetBoneMatrix(USkeletalMeshComponent* Mesh, int BoneIndex)
{

	using call1 = UeEngineTools::FMatrix* (*)(void* Mesh,  OUT FMatrix* outParam,int BoneIndex);


	LONG_PTR exebase = (LONG_PTR)GetModuleHandle(nullptr);


	call1 _call1 = (call1)(exebase + GetBoneMatrixOffset);


	UeEngineTools::FMatrix FMatrix;

	_call1(Mesh, &FMatrix, BoneIndex);

	Vector3 vec3;

	vec3.X = FMatrix._41;
	vec3.Y = FMatrix._42;
	vec3.Z = FMatrix._43;


	return vec3;

}

bool UeEngineTools::ProjectWorldLocationToScreen(Vector3 WorldLocation, OUT Vector2& ScreenLocation)
{
	LONG_PTR exebase = (LONG_PTR)GetModuleHandle(nullptr);

	using call1 = bool (*)(void* PlayerController, Vector3 WorldLocation, OUT Vector2& ScreenLocation, bool bPlayerViewportRelative);


	call1 _call1 = (call1)(exebase +WorldToScreenOffset);

	auto gworld = GetGWorld();

	return _call1(gworld->OwningGameInstance->GetPlayerController(), WorldLocation, ScreenLocation,false);
}


void UeEngineTools::DrawPartBone(int Start,int End, ACharacter*pActor, UeEngineTools::BoneIdx* Idx, int r, int g, int b, DrawBoneLineCallBack _callback)
{
	int* boneidx = (int*)Idx;

	Vector2 Pt{ 0 };
	Vector2 oPt{ 0 };

	for (int i = Start; i < Start + End; i++)
	{
		Vector3 pos = GetBoneMatrix(pActor->Mesh, boneidx[i]);
		if (ProjectWorldLocationToScreen(pos, Pt) && oPt.X > 0 && oPt.Y > 0)
		{
			_callback(oPt.X, oPt.Y,Pt.X,Pt.Y,r,g,b);
		}
		oPt = Pt;
	}
}


void UeEngineTools::DrawActorAllBone(ACharacter* pActor, UeEngineTools::BoneIdx* Idx, int r,int g,int b,DrawBoneLineCallBack _callback)
{
	DrawPartBone(0, 6, pActor,Idx,r,g,b,_callback);
	DrawPartBone(6, 7, pActor,Idx, r, g, b, _callback);
	DrawPartBone(14, 7, pActor,Idx, r, g, b, _callback);
}


void UeEngineTools::DrawAllActorsBone(DrawBoneLineCallBack _callback)
{
	auto gworld = GetGWorld();

	for (int i = 0; i < gworld->PersistentLevel->Actors.Count; i++)
	{
		UeEngineTools::ACharacter* pActor = gworld->PersistentLevel->Actors.data[i];

		if (!pActor || !pActor->RootComponent)
			continue;


		Vector3 WorldLocation = pActor->RootComponent->ComponentToWorld.Translation;

		Vector2 ScreenLocation = {};

		auto clsname = GetName((UObjectBase*)pActor);

		if (clsname.find("Zombie_BP") == std::string::npos)
			continue;


		BoneIdx idx;
		GetBoneIndex(pActor, &idx);

		DrawActorAllBone(pActor, &idx,255,255,0,_callback);

	}

}


void UeEngineTools::DrawAllActors(DrawAllActorCallBack _callback)
{
	auto gworld = GetGWorld();

	for (int i = 0; i < gworld->PersistentLevel->Actors.Count; i++)
	{
		UeEngineTools::ACharacter* pActor = gworld->PersistentLevel->Actors.data[i];

		if (!pActor || !pActor->RootComponent)
			continue;


		Vector3 WorldLocation  = pActor->RootComponent->ComponentToWorld.Translation;

		Vector2 ScreenLocation = {};

		auto clsname  = GetName((UObjectBase*)pActor);


		if (clsname.find("Zombie_BP") == std::string::npos)
			continue;


		BoneIdx idx;
		GetBoneIndex(pActor, &idx);




		//绘制所有骨骼名
		int Count = sizeof(BoneIdx) / sizeof(int);
		for (int j = 0; j < Count; j++)
		{
			int* boneidx = (int*)&idx;

			Vector3 pos = GetBoneMatrix(pActor->Mesh, boneidx[j]);
			auto _BoneName = GetName(pActor->Mesh->SkeletalMesh->Names.data[boneidx[j]].NameID);

			if (ProjectWorldLocationToScreen(pos, ScreenLocation) && ScreenLocation.X > 0 && ScreenLocation.Y > 0)
			{

				//OutputDebugStringEx("[qqsghack]%s:%d\r\n", __FUNCTION__, __LINE__);
				char buf[MAX_PATH] = { 0 };
				sprintf(buf, "%s", _BoneName.data());
				_callback(buf, ScreenLocation.X, ScreenLocation.Y);
			}


		}
		continue;

		
		//绘制骨骼名
		for (int j = 0; j < pActor->Mesh->SkeletalMesh->Names.Count; j++)
		{
			auto _BoneName=   GetName(pActor->Mesh->SkeletalMesh->Names.data[j].NameID);
			Vector3 pos = GetBoneMatrix(pActor->Mesh, j);

			if (ProjectWorldLocationToScreen(pos, ScreenLocation) && ScreenLocation.X > 0 && ScreenLocation.Y > 0)
			{

				//OutputDebugStringEx("[qqsghack]%s:%d\r\n", __FUNCTION__, __LINE__);
				char buf[MAX_PATH] = { 0 };
				sprintf(buf, "%s", _BoneName.data());
				_callback(buf, ScreenLocation.X, ScreenLocation.Y);
			}

		}

		continue;

		for (int j = 0; j< pActor->Mesh->BoneTransform.Count; j++)
		{
			//OutputDebugStringEx("[qqsghack]%s:%d\r\n", __FUNCTION__, __LINE__);


			Vector3 pos =  GetBoneMatrix(pActor->Mesh, j);


			//UeEngineTools::FMatrix maxtrix1;
			//UeEngineTools::FMatrix maxtrix2;
			//UeEngineTools::FMatrix maxtrix3;
			//if (!pActor->Mesh->BoneTransform.data[j].ToMatrixWithScale(maxtrix1) || !pActor->Mesh->ComponentToWorld.ToMatrixWithScale(maxtrix2))
			//{
			//	continue;

			//}

			//maxtrix3 = maxtrix1 * maxtrix2;
			//
			//
			//pos.X = maxtrix3._41;
			//pos.Y = maxtrix3._42;
			//pos.Z = maxtrix3._43;

			//OutputDebugStringEx("%s:%d\r\n", __FUNCTION__, __LINE__);

			if (ProjectWorldLocationToScreen(pos, ScreenLocation) && ScreenLocation.X > 0 && ScreenLocation.Y > 0)
			{
				
				//OutputDebugStringEx("[qqsghack]%s:%d\r\n", __FUNCTION__, __LINE__);
				char buf[MAX_PATH] = { 0 };
				sprintf(buf, "%p", pActor);
				_callback(buf, ScreenLocation.X, ScreenLocation.Y);
			}
			//OutputDebugStringEx("[qqsghack]%s:%d\r\n", __FUNCTION__, __LINE__);
		}

	}


}

bool UeEngineTools::LineTraceSingle(Vector3 Start, Vector3 End, TArray<AActor*>& ActorsToIgnore)
{
	LONG_PTR exebase = (LONG_PTR)GetModuleHandle(nullptr);


	typedef bool (*LineTraceSingleCallBack)(UeEngineTools::UWorld* WorldContextObject,
		Vector3 Start, Vector3 End, char TraceChannel, bool bTraceComplex, TArray<AActor*>& ActorsToIgnore,
		int DrawDebugType,
		FHitResult& OutHit, bool bIgnoreSelf,
		FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime);

	FHitResult OutHit;

	LineTraceSingleCallBack _callback = (LineTraceSingleCallBack)(exebase + LineTraceSingleOffset);


	return _callback(GetGWorld(), Start, End, 0, true, ActorsToIgnore, 0, OutHit, true, { 0,0,0,0 }, { 0,0,0,0 }, 0.f);
}

void UeEngineTools::DrawTraceSingle(DrawBoneLineCallBack _callback)
{
	auto gworld = GetGWorld();

	for (int i = 0; i < gworld->PersistentLevel->Actors.Count; i++)
	{
		UeEngineTools::ACharacter* pActor = gworld->PersistentLevel->Actors.data[i];

		if (!pActor || !pActor->RootComponent)
			continue;


		Vector3 WorldLocation = pActor->RootComponent->ComponentToWorld.Translation;

		Vector2 ScreenLocation = {};

		auto clsname = GetName((UObjectBase*)pActor);


		if (clsname.find("Zombie_BP") == std::string::npos)
			continue;


		BoneIdx idx;
		GetBoneIndex(pActor, &idx);

		APawn *Player =  (APawn*)gworld->OwningGameInstance->GetPlayer();
		APlayerController* pAPlayerController  = (APlayerController*)Player->Controller;

		Vector3 StartPos = pAPlayerController->PlayerCameraManager->CameraCachePrivate.POVPos;

		//OutputDebugStringEx("[qqsghack] the StartPos:%.2f %.2f %.2f\r\n", StartPos.X, StartPos.Y, StartPos.Z);

		Vector3 EndPos = GetBoneMatrix(pActor->Mesh, idx.head);
		TArray<AActor*> ActorsToIgnore;

		ActorsToIgnore.push(Player, pActor);
		bool bret = LineTraceSingle(StartPos, EndPos, ActorsToIgnore);
		ActorsToIgnore.Pop();

		if (bret)
		{
			DrawActorAllBone(pActor, &idx, 255, 255, 0, _callback);
		}
		else {

			DrawActorAllBone(pActor, &idx, 255, 0, 0, _callback);
		}
	}
}

std::string UeEngineTools::GetName(int nID)
{
	UeEngineTools::FNamePool* gname = GetGName();

	UINT block = 0;
	UINT offset = 0;

	IdToBlackAndOffset(nID, block, offset);

	auto _entey = reinterpret_cast<FNameEntry*>(gname->Entries.Blocks[block] + 2 * offset);

	return std::string(_entey->AnsiName, _entey->Header.Len);
}

UeEngineTools::APlayerController* UeEngineTools::UGameInstance::GetPlayerController()
{

	return this->LocalPlayers.data[0]->PlayerController;

}

UeEngineTools::AActor* UeEngineTools::UGameInstance::GetPlayer()
{

	return this->LocalPlayers.data[0]->PlayerController->Pawn;

}


void* UeEngineTools::HookPostRender(void *FakeFunc)
{

	auto gworld = GetGWorld();

	LONG_PTR  PlayerController =  (LONG_PTR)gworld->OwningGameInstance->GetPlayerController();


	LONG_PTR vtbale = (PlayerController + 0);
	


	void* realfunc = *(void**)(vtbale + 0x778);

	OutputDebugStringEx("[wow1] the PostRender:0x%llX\r\n", realfunc);

	return realfunc;
}



bool UeEngineTools::FTransform::ToMatrixWithScale(OUT UeEngineTools::FMatrix &outmatrix)
{

	//OutputDebugStringEx("[qqsghack] this:%p", this);


	if ((LONG_PTR)this < 0x10000)
		return false;


	FMatrix m;

	m._41 = Translation.X;
	m._42 = Translation.Y;
	m._43 = Translation.Z;

	float x2 = Rotation.X + Rotation.X;
	float y2 = Rotation.Y + Rotation.Y;
	float z2 = Rotation.Z + Rotation.Z;

	float xx2 = Rotation.X * x2;
	float yy2 = Rotation.Y * y2;
	float zz2 = Rotation.Z * z2;
	m._11 = (1.0f - (yy2 + zz2)) * Scale3D.X;
	m._22 = (1.0f - (xx2 + zz2)) * Scale3D.Y;
	m._33 = (1.0f - (xx2 + yy2)) * Scale3D.Z;


	float yz2 = Rotation.Y * z2;
	float wx2 = Rotation.W * x2;
	m._32 = (yz2 - wx2) * Scale3D.Z;
	m._23 = (yz2 + wx2) * Scale3D.Y;


	float xy2 = Rotation.X * y2;
	float wz2 = Rotation.W * z2;
	m._21 = (xy2 - wz2) * Scale3D.Y;
	m._12 = (xy2 + wz2) * Scale3D.X;


	float xz2 = Rotation.X * z2;
	float wy2 = Rotation.W * y2;
	m._31 = (xz2 + wy2) * Scale3D.Z;
	m._13 = (xz2 - wy2) * Scale3D.X;

	m._14 = 0.0f;
	m._24 = 0.0f;
	m._34 = 0.0f;
	m._44 = 1.0f;


	outmatrix = m;

	return true;


}

UeEngineTools::FMatrix UeEngineTools::FMatrix::operator*(const FMatrix& other)
{
	FMatrix NewMatrix;
	NewMatrix._11 = this->_11 * other._11 + this->_12 * other._21 + this->_13 * other._31 + this->_14 * other._41;
	NewMatrix._12 = this->_11 * other._12 + this->_12 * other._22 + this->_13 * other._32 + this->_14 * other._42;
	NewMatrix._13 = this->_11 * other._13 + this->_12 * other._23 + this->_13 * other._33 + this->_14 * other._43;
	NewMatrix._14 = this->_11 * other._14 + this->_12 * other._24 + this->_13 * other._34 + this->_14 * other._44;
	NewMatrix._21 = this->_21 * other._11 + this->_22 * other._21 + this->_23 * other._31 + this->_24 * other._41;
	NewMatrix._22 = this->_21 * other._12 + this->_22 * other._22 + this->_23 * other._32 + this->_24 * other._42;
	NewMatrix._23 = this->_21 * other._13 + this->_22 * other._23 + this->_23 * other._33 + this->_24 * other._43;
	NewMatrix._24 = this->_21 * other._14 + this->_22 * other._24 + this->_23 * other._34 + this->_24 * other._44;
	NewMatrix._31 = this->_31 * other._11 + this->_32 * other._21 + this->_33 * other._31 + this->_34 * other._41;
	NewMatrix._32 = this->_31 * other._12 + this->_32 * other._22 + this->_33 * other._32 + this->_34 * other._42;
	NewMatrix._33 = this->_31 * other._13 + this->_32 * other._23 + this->_33 * other._33 + this->_34 * other._43;
	NewMatrix._34 = this->_31 * other._14 + this->_32 * other._24 + this->_33 * other._34 + this->_34 * other._44;
	NewMatrix._41 = this->_41 * other._11 + this->_42 * other._21 + this->_43 * other._31 + this->_44 * other._41;
	NewMatrix._42 = this->_41 * other._12 + this->_42 * other._22 + this->_43 * other._32 + this->_44 * other._42;
	NewMatrix._43 = this->_41 * other._13 + this->_42 * other._23 + this->_43 * other._33 + this->_44 * other._43;
	NewMatrix._44 = this->_41 * other._14 + this->_42 * other._24 + this->_43 * other._34 + this->_44 * other._44;

	return NewMatrix;
}
