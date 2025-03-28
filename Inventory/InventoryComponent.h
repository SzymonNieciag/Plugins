// Blueprints Studio

#pragma once

#include "CoreMinimal.h"
#include <Components/ActorComponent.h>
#include "MountainTrails/Libraries/MTDefines.h"
#include "InstancedStruct.h"
#include "InventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryUpdate, const class UInventoryComponent*, Inventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMoneyUpdate, const int, Money);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAddedToInventory, class UInventoryComponent*, Inventory, const FInstancedStruct, Slot, const int32, Quantity);


UCLASS(ClassGroup=Inventory)
class MOUNTAINTRAILS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Inventory")
	FInventoryUpdate OnInventoryUpdated;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Inventory")
	FInventoryMoneyUpdate OnInventoryMoneyUpdated;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Inventory")
	FOnItemAddedToInventory OnItemAddedToInventory;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

protected:
	void BeginPlay() override;

	bool bRemoveSlotIfEmpty = true;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float TotalMoney = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (BaseStruct = "FSlotStruct"))
	TArray<FInstancedStruct> Slots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 CurrentInventoryCapacity = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 MaxInventoryCapacity = 100;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult AddItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int32 Quantity, bool IgnoreLimit = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryOperationResult RemoveItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int32 Quantity);

	bool HasItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int32 Quantity = 1);
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSoftClassPtr<class AInteractableActor> InteractableActorClass, int& LeftMissingItems, int& AmountOfItems, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FSlotStruct> GetItemsOfType(EItemType TypeOfItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FSlotStruct> GetItemsContainedInDataTable(class UDataTable* InDataTable);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CleanInventory();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsEmpty();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddMoney(float MoneyAmount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DesolveItem(class AInteractableActor* InteractableActor, bool DesolveOption, float DesolveMaterials = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasMaterialsToCreateItem(class AInteractableActor* InteractableActor, TArray<FActorAmount>& OutRequirementsActors, TArray<FActorAmount>& LeftNeededRequirementsActors);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool SpendMaterialsForItem(class AInteractableActor* InteractableActor);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetAmountOfItemBySoftClass(TSoftClassPtr<class AInteractableActor> SoftClass);

protected:
	virtual FInstancedStruct CreateSlotStruct(TSoftClassPtr<AInteractableActor> InteractableActorClass, int32 Quantity, FItemStruct* ItemStruct);

	virtual bool IsCompatibilieSlotExist(const FInstancedStruct& InstancedStruct, const TSoftClassPtr<AInteractableActor>& ItemClass) const;

private:
	const struct FInteractableActorData* GetInteractableActorData(class AInteractableActor* InteractableActor);

	FItemStruct* FindItemAndUpdateCapacity(int32& Quantity, int& TotalAddedItems, TSoftClassPtr<class AInteractableActor>& InteractableActorClass, bool IgnoreLimit);
};

