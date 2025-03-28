
#include "MTInteractableMarkerComponent.h"
#include "MountainTrails/Interfaces/Interaction.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/Character.h>

bool UMTInteractableMarkerComponent::CanInteract()
{
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (Character)
	{
		IInteraction* Interaction = Cast<IInteraction>(GetOwner());
		if (Interaction != nullptr)
		{
			if (Interaction->Execute_CanInteract(GetOwner(), Character))
				return true;
		}
	}
	return false;
}
