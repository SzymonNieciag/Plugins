// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <../Plugins/Runtime/SmartObjects/Source/SmartObjectsModule/Public/SmartObjectComponent.h>
#include "NPSmartObjectComponent.generated.h"

struct FSmartPointStruct; 

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), HideCategories = (Activation, AssetUserData, Collision, Cooking, HLOD, Lighting, LOD, Mobile, Mobility, Navigation, Physics, RayTracing, Rendering, Tags, TextureStreaming))
class NOPEAISYSTEM_API UNPSmartObjectComponent : public USmartObjectComponent
{
	GENERATED_BODY()

public:
	UNPSmartObjectComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay();
	virtual void PostLoad();

	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph);

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

protected:
	/** Native callback when this component is moved */
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;

public:
#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable)
	FSmartPointStruct StartReserveActor(class ANPAICharacterBase* AICharacterBase, bool TryReserveFirstSpot = true);

	UFUNCTION(BlueprintCallable)
	FSmartPointStruct StopReserveActor(class ANPAICharacterBase* AICharacterBase);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Task", meta = (ClampMin = "50.0", ClampMax = "300.0", UIMin = "50.0", UIMax = "300.0"))
	float AcceptableRadious = 50.0f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|SmartPoint")
	TArray<FSmartPointStruct> SmartPointsStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|SmartPoint")
	bool bReservePoint = true;

public:
	UFUNCTION(BlueprintPure, Category = "AI|Queue")
	FSmartPointStruct FindCharacter(class ANPAICharacterBase* AICharacterBase);

	UFUNCTION(BlueprintPure, Category = "AI|Queue")
	bool CanReserveActor(ACharacter* User);

private:
	void UpdateSmartPoints();
	void UpdateSmartPointTransform(FSmartPointStruct& SmartPointToUpdate, int Index);
};