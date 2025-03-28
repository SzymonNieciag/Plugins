
#include "FuelInventoryComponent.h"
#include "MountainTrails/MTGameState.h"
#include "MountainTrails/MTDeveloperSettings.h"

UFuelInventoryComponent::UFuelInventoryComponent()
{

}

bool UFuelInventoryComponent::IsFireActive()
{
	return bIsFireActive;
}

bool UFuelInventoryComponent::TrySetFireActive(bool InbIsFireActive)
{
	if (bIsFireActive == InbIsFireActive)
	{
		return bIsFireActive;
	}
	if (Slots.Num() <= 0)
	{
		InbIsFireActive = false;
	}
	OnFuelComponentStatusChanged.Broadcast(InbIsFireActive);
	return bIsFireActive = InbIsFireActive;
}

struct FNPTimeStruct UFuelInventoryComponent::UpdatedFireTime(const struct FNPTimeStruct UpdatedTime, const struct FNPTimeStruct TimeDifference)
{
    FNPTimeStruct UpdatedFireTime = FNPTimeStruct(0, 0, 0);

    if (bIsFireActive)
    {
        LeftFireTime.UpdateTime(-TimeDifference.TotalTime);
        UpdatedFireTime = TimeDifference;

        if (LeftFireTime.TotalTime <= 0)
        {
            float TotalAddedFuel = 0;
            for (int32 i = Slots.Num() - 1; i >= 0; i--)
            {
                if (FSlotStruct* SlotStruct = Slots[i].GetMutablePtr<FSlotStruct>())
                {
                    FName RowName = SlotStruct->GetCleanedSoftClassName();
                    FFuelStruct* ItemStruct = GetDefault<UMTDeveloperSettings>()->FuelAmountsData.LoadSynchronous()->FindRow<FFuelStruct>(RowName, "", true);

                    if (ItemStruct)
                    {
                        for (int32 j = 0; j < SlotStruct->Quantity; j++)
                        {
                            LeftFireTime.UpdateTime(ItemStruct->FuelValue);
                            MaxFireTime.UpdateTimeStruct(ItemStruct->FuelValue);
                            RemoveItem(SlotStruct->InteractableActorClass, 1);
                            TotalAddedFuel += ItemStruct->FuelValue;

                            if (LeftFireTime.TotalTime > 0)
                            {
                                return UpdatedFireTime;
                            }
                        }
                    }
                }
            }
            LeftFireTime.UpdateTimeStruct(0);
            UpdatedFireTime.UpdateTimeStruct(TotalAddedFuel);
            TrySetFireActive(false);
            return UpdatedFireTime;
        }
    }
    return UpdatedFireTime;
}

float UFuelInventoryComponent::GetFuelPercent(FNPTimeStruct& LeftTime)
{
	LeftTime = LeftFireTime;
	if (LeftFireTime.TotalTime <= 0)
	{
		return 0;
	}
	return LeftFireTime.TotalTime/MaxFireTime.TotalTime;
}
