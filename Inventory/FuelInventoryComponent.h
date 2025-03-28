// Blueprints Studio

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "FuelInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFuelComponentStatusSignature, const bool, IsActivated);

UCLASS(ClassGroup=Inventory)
class MOUNTAINTRAILS_API UFuelInventoryComponent : public UInventoryComponent
{
	GENERATED_BODY()

public:
	UFuelInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Cooking")
	FFuelComponentStatusSignature OnFuelComponentStatusChanged;

	bool IsFireActive();

	bool TrySetFireActive(bool InbIsFireActive);

	struct FNPTimeStruct UpdatedFireTime(const struct FNPTimeStruct UpdatedTime, const struct FNPTimeStruct TimeDifference);
	
	UFUNCTION(BlueprintPure, Category = "FuelInventoryComponent")
	float GetFuelPercent(FNPTimeStruct& LeftTime);

protected:
	UPROPERTY()
	bool bIsFireActive = false;

	UPROPERTY(VisibleAnywhere, Category = "FuelInventoryComponent")
	struct FNPTimeStruct LeftFireTime;

	UPROPERTY(VisibleAnywhere, Category = "FuelInventoryComponent")
	struct FNPTimeStruct MaxFireTime;
};
