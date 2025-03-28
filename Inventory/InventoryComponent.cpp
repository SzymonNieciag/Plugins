
#include "InventoryComponent.h"
#include "MountainTrails/Libraries/MTDefines.h"
#include "MountainTrails/Interactables/InteractableActor.h"
#include "Kismet/GameplayStatics.h"
#include <MountainTrails/MountainTrailsGameMode.h>
#include <Kismet/KismetMathLibrary.h>
#include "MountainTrails/Libraries/EditableSystemDefines.h"
#include "MountainTrails/MTDeveloperSettings.h"
#include "MountainTrails/Libraries/MTLibrary.h"
#include "MountainTrails/MTGameInstance.h"
#include "StructUtils/InstancedStruct.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName Name = PropertyChangedEvent.Property->GetFName();
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UInventoryComponent, Slots) || Name == "Quantity" || Name == "InteractableActorClass")
	{
		CurrentInventoryCapacity = 0;
		for (FInstancedStruct& StructSlot : Slots)
		{
			if (FSlotStruct* InSlotStruct = StructSlot.GetMutablePtr<FSlotStruct>())
			{
				int TotalAddedItems;
				FindItemAndUpdateCapacity(InSlotStruct->Quantity, TotalAddedItems, InSlotStruct->InteractableActorClass, true);
			}
		}
	}
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

FInventoryOperationResult UInventoryComponent::AddItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int32 Quantity, bool IgnoreLimit /*= false*/)
{
	int TotalAddedItems = 0;

	if (!InteractableActorClass.IsValid() || Quantity == 0)
	{
		return FInventoryOperationResult(0, INDEX_NONE);
	}

	FItemStruct* ItemStruct = FindItemAndUpdateCapacity(Quantity, TotalAddedItems, InteractableActorClass, IgnoreLimit);

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		FInstancedStruct& StructSlot = Slots[i];

		if (FSlotStruct* SlotStruct = StructSlot.GetMutablePtr<FSlotStruct>())
		{
			if (IsCompatibilieSlotExist(StructSlot, InteractableActorClass))
			{
				SlotStruct->Quantity += TotalAddedItems;
				OnInventoryUpdated.Broadcast(this);
				OnItemAddedToInventory.Broadcast(this, StructSlot, Quantity);
				return FInventoryOperationResult(TotalAddedItems, i);
			}
		}
	}

	FInstancedStruct NewSlot = CreateSlotStruct(InteractableActorClass, TotalAddedItems, ItemStruct);
	int32 NewIndex = Slots.Add(NewSlot);

	OnInventoryUpdated.Broadcast(this);
	OnItemAddedToInventory.Broadcast(this, NewSlot, Quantity);

	return FInventoryOperationResult(TotalAddedItems, NewIndex);
}

FItemStruct* UInventoryComponent::FindItemAndUpdateCapacity(int32& Quantity, int& TotalAddedItems, TSoftClassPtr<class AInteractableActor>& InteractableActorClass, bool IgnoreLimit)
{
	FItemStruct* ItemStruct = nullptr;
	Quantity = FMath::Abs(Quantity);
	UMTGameInstance* MTGameInstance = UMTGameInstance::GetMTGameInstance(this);
	if (!MTGameInstance)
	{
		const UMTDeveloperSettings* MTDeveloperSettings = GetDefault<UMTDeveloperSettings>();
		TSubclassOf<UMTGameInstance> MTGameInstanceClass = MTDeveloperSettings->DefaultGameInstance.LoadSynchronous();
		if (MTGameInstanceClass)
		{
			MTGameInstance = Cast<UMTGameInstance>(MTGameInstanceClass->GetDefaultObject(true));
		}
	}
	// Source - calling data table
	static const FString ContextString(TEXT("Items Data"));

	FName RowName = UMTLibrary::GetCleanedSoftClassName(InteractableActorClass.ToString());
	ItemStruct = MTGameInstance->ItemsData->FindRow<FItemStruct>(RowName, ContextString, false);
	int ItemCapacity = 0;
	if (ItemStruct)
	{
		ItemCapacity = ItemStruct->ItemCapacity;
	}
	else
	{
		return ItemStruct;
	}
	float NextCapacity = CurrentInventoryCapacity + ItemCapacity * Quantity;

	if (IgnoreLimit || NextCapacity <= MaxInventoryCapacity || ItemCapacity == 0)
	{
		TotalAddedItems = Quantity;
	}
	else
	{
		int FreeSpace = MaxInventoryCapacity - CurrentInventoryCapacity;
		TotalAddedItems = FreeSpace / ItemCapacity;
	}

	CurrentInventoryCapacity += TotalAddedItems * ItemCapacity;
	return ItemStruct;
}

FInventoryOperationResult UInventoryComponent::RemoveItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int32 Quantity)
{
	int RemovedItemAmount = 0;
	Quantity = FMath::Abs(Quantity);

	if (!InteractableActorClass.IsValid() || Quantity == 0)
	{
		return FInventoryOperationResult(0, INDEX_NONE);
	}

	UMTGameInstance* MTGameInstance = UMTGameInstance::GetMTGameInstance(this);
	if (!MTGameInstance)
	{
		return FInventoryOperationResult(0, INDEX_NONE);
	}

	static const FString ContextString(TEXT("Items Data"));
	FName RowName = UMTLibrary::GetCleanedSoftClassName(InteractableActorClass.ToString());
	FItemStruct* ItemStruct = MTGameInstance->ItemsData->FindRow<FItemStruct>(RowName, ContextString, true);
	if (!ItemStruct)
	{
		return FInventoryOperationResult(0, INDEX_NONE);
	}

	for (int32 i = Slots.Num() - 1; i >= 0; i--)
	{
		if (FSlotStruct* SlotStruct = Slots[i].GetMutablePtr<FSlotStruct>())
		{
			if (SlotStruct->InteractableActorClass == InteractableActorClass)
			{
				if (Quantity < SlotStruct->Quantity)
				{
					SlotStruct->Quantity -= Quantity;
					RemovedItemAmount = Quantity;
					CurrentInventoryCapacity -= ItemStruct->ItemCapacity * RemovedItemAmount;
					OnInventoryUpdated.Broadcast(this);
					return FInventoryOperationResult(RemovedItemAmount, i);
				}
				else
				{
					RemovedItemAmount = SlotStruct->Quantity;
					SlotStruct->Quantity = 0;
					if (bRemoveSlotIfEmpty)
					{
						CurrentInventoryCapacity -= ItemStruct->ItemCapacity * RemovedItemAmount;
						Slots.RemoveAt(i);
						OnInventoryUpdated.Broadcast(this);
					}
					return FInventoryOperationResult(RemovedItemAmount, i);
				}
			}
		}
	}
	return FInventoryOperationResult(0, INDEX_NONE);
}

bool UInventoryComponent::HasItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int32 Quantity /*= 1*/)
{
	int LeftItems;
	int AmountOfItems;
	return HasItem(InteractableActorClass, LeftItems, AmountOfItems, Quantity);
}

bool UInventoryComponent::HasItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int& LeftMissingItems, int& AmountOfItems, int32 Quantity /*= 1*/)
{
	for (const FInstancedStruct& StructSlot : Slots)
	{
		if (const FSlotStruct* SlotStruct = StructSlot.GetPtr<FSlotStruct>())
		{
			if (SlotStruct->InteractableActorClass == InteractableActorClass)
			{
				AmountOfItems = SlotStruct->Quantity;
				if (SlotStruct->Quantity >= Quantity)
				{
					return true;
				}
				else
				{
					LeftMissingItems = Quantity - SlotStruct->Quantity;
					return false;
				}
			}
		}
	}
	LeftMissingItems = -1;
	return false;
}

TArray<FSlotStruct> UInventoryComponent::GetItemsOfType(EItemType TypeOfItem)
{
	TArray<FSlotStruct> OutSlots;
	for (const FInstancedStruct& StructSlot : Slots)
	{
		if (const FSlotStruct* SlotStruct = StructSlot.GetPtr<FSlotStruct>())
		{
			if (SlotStruct->TypeOfItem == TypeOfItem)
			{
				OutSlots.Add(*SlotStruct);
			}
		}
	}
	return OutSlots;
}

TArray<FSlotStruct> UInventoryComponent::GetItemsContainedInDataTable(class UDataTable* InDataTable)
{
	TArray<FSlotStruct> OutSlots;
	if (ensure(InDataTable))
	{
		TArray<FName> OutRowNames;
		OutRowNames = InDataTable->GetRowNames();
		for (const FName& RowName : OutRowNames)
		{
			for (const FInstancedStruct& StructSlot : Slots)
			{
				if (const FSlotStruct* Item = StructSlot.GetPtr<FSlotStruct>())
				{
					if (Item->GetCleanedSoftClassName() == RowName)
					{
						OutSlots.Add(*Item);
					}
				}
			}
		}
	}
	return OutSlots;
}

void UInventoryComponent::CleanInventory()
{
	Slots.Empty();
	CurrentInventoryCapacity = 0;
}

bool UInventoryComponent::IsEmpty()
{
	if (CurrentInventoryCapacity == 0)
		return true;
	return false;
}

void UInventoryComponent::AddMoney(float NewMoney)
{
	TotalMoney += NewMoney;
	OnInventoryMoneyUpdated.Broadcast(TotalMoney);
}

void UInventoryComponent::DesolveItem(AInteractableActor* InteractableActor, bool DesolveOption, float DesolveMaterials /*= 1*/)
{
	if (!InteractableActor)
	{
		return;
	}
	if (DesolveOption)
	{
		if (const FInteractableActorData* InteractableActorData = GetInteractableActorData(InteractableActor))
		{
			for (const FActorAmount& ActorAmount : InteractableActorData->RequirementsActors)
			{
				int RecovermaterialsAmount = UKismetMathLibrary::Round(ActorAmount.Quantity * DesolveMaterials);
				AddItem(ActorAmount.InteractableActorClass, RecovermaterialsAmount);
			}
		}
	}
	else
	{
		AddItem(this->GetClass(), 1);
	}
	InteractableActor->Destroy();
}

bool UInventoryComponent::HasMaterialsToCreateItem(class AInteractableActor* InteractableActor, TArray<FActorAmount>& OutRequirementsActors, TArray<FActorAmount>& LeftNeededRequirementsActors)
{
	bool CanBuild = true;
	const FInteractableActorData* InteractableActorData = GetInteractableActorData(InteractableActor);
	if (InteractableActorData)
	{
		for (FActorAmount ActorAmount : InteractableActorData->RequirementsActors)
		{
			int LeftItems = 0;
			if (!HasItem(ActorAmount.InteractableActorClass, LeftItems, ActorAmount.Quantity))
			{
				CanBuild = false;
				LeftNeededRequirementsActors.Add(FActorAmount(ActorAmount.InteractableActorClass, LeftItems));
			}
		}
		if (!CanBuild)
		{
			OutRequirementsActors = InteractableActorData->RequirementsActors;
			return false;
		}
	}
	return true;
}

bool UInventoryComponent::SpendMaterialsForItem(class AInteractableActor* InteractableActor)
{
	if (const FInteractableActorData* InteractableActorData = GetInteractableActorData(InteractableActor))
	{
		for (const FActorAmount& ActorAmount : InteractableActorData->RequirementsActors)
		{
			RemoveItem(ActorAmount.InteractableActorClass, ActorAmount.Quantity);
		}
		return true;
	}

	return false;
}

int32 UInventoryComponent::GetAmountOfItemBySoftClass(TSoftClassPtr<class AInteractableActor> SoftClass)
{
	int32 Amount = 0;
	for (const FInstancedStruct& StructSlot : Slots)
	{
		if (const FSlotStruct* Item = StructSlot.GetPtr<FSlotStruct>())
		{
			if (Item->InteractableActorClass == SoftClass)
			{
				Amount += Item->Quantity;
			}
		}
	}
	return Amount;
}

FInstancedStruct UInventoryComponent::CreateSlotStruct(TSoftClassPtr<AInteractableActor> InteractableActorClass, int32 Quantity, FItemStruct* ItemStruct)
{
	FSlotStruct NewSlot(InteractableActorClass, Quantity);
	if (ItemStruct)
	{
		NewSlot.TypeOfItem = ItemStruct->TypeOfItem;
	}
	return FInstancedStruct::Make(NewSlot);
}

bool UInventoryComponent::IsCompatibilieSlotExist(const FInstancedStruct& InstancedStruct, const TSoftClassPtr<AInteractableActor>& ItemClass) const
{
	if (const FSlotStruct* SlotStruct = InstancedStruct.GetPtr<FSlotStruct>())
	{
		return SlotStruct->InteractableActorClass == ItemClass;
	}
	return false;
}

const FInteractableActorData* UInventoryComponent::GetInteractableActorData(class AInteractableActor* InteractableActor)
{
	if (UMTGameInstance* MTGameInstance = UMTGameInstance::GetMTGameInstance(this))
	{
		if (ensure(MTGameInstance->EditableActorsDataBase))
		{
			if (InteractableActor)
			{
				static const FString ContextString(TEXT("InteractableActorStruct Data"));
				TSoftClassPtr<class AInteractableActor> InteractableActorClass = TSoftClassPtr<class AInteractableActor>(InteractableActor->GetClass());
				FName RowName = UMTLibrary::GetCleanedSoftClassName(InteractableActorClass.ToString());
				const FInteractableActorData* InteractableActorData = MTGameInstance->EditableActorsDataBase->FindRow<FInteractableActorData>(RowName, ContextString, true);
				if (InteractableActorData)
				{
					return InteractableActorData;
				}
			}
		}
	}
	return nullptr;
}

