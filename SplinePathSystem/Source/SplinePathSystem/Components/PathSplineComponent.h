// Szymon XIII Wielki

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "PathSplineComponent.generated.h"


UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), HideCategories = (Activation, AssetUserData, Collision, Cooking, HLOD, Lighting, LOD, Mobile, Mobility, Navigation, Physics, RayTracing, Rendering, Tags, TextureStreaming))
class SPLINEPATHSYSTEM_API UPathSplineComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	UPathSplineComponent();

private:
	UPROPERTY(Instanced, Export)
	class UPathSplineMetadata* MySplineMetadata = nullptr;

public:

	virtual USplineMetadata* GetSplinePointsMetadata();
	virtual const USplineMetadata* GetSplinePointsMetadata() const;
	virtual void PostLoad() override;
	virtual void PostDuplicate(bool bDuplicateForPie) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditImport() override;

#endif
public:
	void FixupPoints();
};

