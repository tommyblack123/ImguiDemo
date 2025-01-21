#include "pch.h"
#include "UeEngineTools.h"
#include"ImguiInit.h"


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

bool UeEngineTools::ProjectWorldLocationToScreen(Vector3 WorldLocation, OUT Vector2& ScreenLocation)
{
	LONG_PTR exebase = (LONG_PTR)GetModuleHandle(nullptr);

	using call1 = bool (*)(void* PlayerController, Vector3 WorldLocation, OUT Vector2& ScreenLocation, bool bPlayerViewportRelative);


	call1 _call1 = (call1)(exebase +WorldToScreenOffset);

	auto gworld = GetGWorld();



	return _call1(gworld->OwningGameInstance->GetPlayerController(), WorldLocation, ScreenLocation,false);


}

void UeEngineTools::DrawAllActors()
{
	auto gworld = GetGWorld();

	OutputDebugStringEx("[qqsghack]PlayerController:0x%llX\r\n", gworld->OwningGameInstance->GetPlayerController());

	//printf("PersistentLevel:0x%llX count:%d\r\n", gworld->PersistentLevel,gworld->PersistentLevel->Actors.Count);


	for (int i = 0; i < gworld->PersistentLevel->Actors.Count; i++)
	{
		UeEngineTools::AActor * pActor = gworld->PersistentLevel->Actors.data[i];

		if (!pActor || !pActor->RootComponent)
			continue;


		Vector3 WorldLocation  = pActor->RootComponent->ComponentToWorld.Translation;

		Vector2 ScreenLocation = {};

		auto clsname  = GetName((UObjectBase*)pActor);


		if (ProjectWorldLocationToScreen(WorldLocation, ScreenLocation) && ScreenLocation.x > 0 && ScreenLocation.y > 0)
		{

			OutputDebugStringEx("[qqsghack] Actor:0x%llX clsname:%s x:%.2f y:%.2f\r\n", pActor, clsname.data(),
				ScreenLocation.x, ScreenLocation.y);

			//¿ªÊ¼»æÖÆ
			ImGui::GetForegroundDrawList()->AddText({ ScreenLocation.x,ScreenLocation.y }, ImColor(255, 255, 0), clsname.data());
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
