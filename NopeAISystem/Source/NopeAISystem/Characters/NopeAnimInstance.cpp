// DRAGO Entertainment (c) 2020


#include "NopeAnimInstance.h"
#include "NopeAISystem/AI/NPAIDefines.h"

void UNopeAnimInstance::SaveAnimInstance(struct FAnimationSaveStruct& OutAnimationSaveStruct)
{
	UAnimMontage* AnimMontage = this->GetCurrentActiveMontage();
	FAnimMontageInstance* AnimMontageInstance = this->GetActiveMontageInstance();

	if (AnimMontage && AnimMontageInstance)
	{
		OutAnimationSaveStruct.AnimMontageSoft = TSoftObjectPtr<UAnimMontage>(AnimMontage);
		OutAnimationSaveStruct.PlayRate = AnimMontageInstance->GetPlayRate();
		OutAnimationSaveStruct.StartingPosition = AnimMontageInstance->GetPosition();
		OutAnimationSaveStruct.StartingSection = AnimMontageInstance->GetCurrentSection();
		OutAnimationSaveStruct.VisibilityBasedAnimTickOption = (uint8)GetSkelMeshComponent()->VisibilityBasedAnimTickOption;
	}
	OutAnimationSaveStruct.StartCharacterTransform = StartCharacterTransform;
	OutAnimationSaveStruct.EndCharacterTransform = EndCharacterTransform;
}

void UNopeAnimInstance::LoadAnimInstance(const struct FAnimationSaveStruct& OutAnimationSaveStruct)
{
	StartCharacterTransform = OutAnimationSaveStruct.StartCharacterTransform;
	StartCharacterTransform = OutAnimationSaveStruct.EndCharacterTransform;
	
	GetSkelMeshComponent()->VisibilityBasedAnimTickOption = (EVisibilityBasedAnimTickOption)OutAnimationSaveStruct.VisibilityBasedAnimTickOption;
	class UAnimMontage* AnimMontage = OutAnimationSaveStruct.AnimMontageSoft.LoadSynchronous();
	if (AnimMontage)
	{
		const float PlayRate = OutAnimationSaveStruct.PlayRate;
		const float StartingPosition = OutAnimationSaveStruct.StartingPosition;
		const FName StartingSection = OutAnimationSaveStruct.StartingSection;
		const float MontageLength = Montage_Play(AnimMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartingPosition);/*
		bool bPlayedSuccessfully = (MontageLength > 0.f);
		if (bPlayedSuccessfully)
		{
			Montage_JumpToSection(StartingSection, AnimMontage);
		}*/
	}
}

void UNopeAnimInstance::SetStartEndCharacterSnapTransform(const FTransform& InStart, const FTransform& InEnd)
{
	StartCharacterTransform = InStart;
	EndCharacterTransform = InEnd;
}
