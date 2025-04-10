#pragma once

#include "PathSplineMetadataDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"
#include "SplinePathSystem/Components/PathSplineMetadata.h" 
#include "Components/SplineComponent.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "ComponentVisualizer.h"
#include "SplinePathSystem/SplinePathActor.h"
#include "PropertyCustomizationHelpers.h"
#include "IStructureDetailsView.h"

#define LOCTEXT_NAMESPACE "FPathSplineMetadataDetails"

FPathSplineMetadataDetails::FPathSplineMetadataDetails()
{
	// Dodaj domyślny element do lokalnej tablicy Connections
	FSplinePathConnection DefaultConnection;
	DefaultConnection.SplineIndex = -1; // Domyślny indeks
	DefaultConnection.ConnectedSplinePathActor = nullptr; // Domyślna referencja
}

// Nazwa klasy szczegółów
FName FPathSplineMetadataDetails::GetName() const
{
	return FName("PathSplineMetadataDetails");
}

// Wyświetlana nazwa klasy szczegółów w edytorze
FText FPathSplineMetadataDetails::GetDisplayName() const
{
	return LOCTEXT("PathSplineMetadataDetails", "Path Spline Metadata Details");
}

void FPathSplineMetadataDetails::Update(USplineComponent* InSplineComponent, const TSet<int32>& InSelectedKeys)
{
	SplineComp = InSplineComponent;
	SelectedKeys = InSelectedKeys;

	if (SplineComp)
	{
		UPathSplineMetadata* PathMetadata = GetPathMetadata();
		if (PathMetadata)
		{
			for (int32 SelectedKey : InSelectedKeys)
			{
				if (SelectedKey >= 0 && SelectedKey < PathMetadata->PointData.Num())
				{
					FSplinePathPointData& PointData = PathMetadata->PointData[SelectedKey];
					SplinePathConnections = PointData.SplinePathConnections;
					FSplinePathPointData* Data = reinterpret_cast<FSplinePathPointData*>(StructOnScope->GetStructMemory());
					if (Data)
					{
						Data->SplinePathConnections = SplinePathConnections;
					}
				}
			}
		}
	}
}

void FPathSplineMetadataDetails::GenerateChildContent(IDetailGroup& DetailGroup)
{
	// Tworzymy FStructOnScope z UStruct oraz wskaźnikiem na dane
	StructOnScope = MakeShared<FStructOnScope>(FSplinePathPointData::StaticStruct());

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bAllowSearch = false;
	Args.bAllowFavoriteSystem = false;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedRef<IStructureDetailsView> StructureDetailsView = PropertyModule.CreateStructureDetailView(Args, FStructureDetailsViewArgs(), StructOnScope);

	// Reagowanie na zmiany w strukturze
	StructureDetailsView->GetOnFinishedChangingPropertiesDelegate().AddSP(this, &FPathSplineMetadataDetails::OnPropertyValueChanged);

	// Dodajemy widok struktury do grupy detali
	DetailGroup.AddWidgetRow()
		[
			StructureDetailsView->GetWidget().ToSharedRef()
		];
}

class UPathSplineMetadata* FPathSplineMetadataDetails::GetPathMetadata() const
{
	if (SplineComp)
	{
		return Cast<UPathSplineMetadata>(SplineComp->GetSplinePointsMetadata());
	}
	return nullptr;
}

UObject* FPathSplineMetadataDetails::GetConnectedSplinePathActor(int32 Index) const
{
	UPathSplineMetadata* PathMetadata = GetPathMetadata();

	for (int32 SelectedKey : SelectedKeys)
	{
		if (PathMetadata && PathMetadata->PointData.IsValidIndex(Index))
		{
			return PathMetadata->PointData[SelectedKey].SplinePathConnections[Index].ConnectedSplinePathActor;
		}
	}
	return nullptr; // Domyślna wartość
}

int32 FPathSplineMetadataDetails::GetSplineIndex(int32 Index) const
{
	UPathSplineMetadata* PathMetadata = GetPathMetadata();
	for (int32 SelectedKey : SelectedKeys)
	{
		if (PathMetadata && PathMetadata->PointData.IsValidIndex(Index))
		{
			return PathMetadata->PointData[SelectedKey].SplinePathConnections[Index].SplineIndex;
		}
	}
	return 0; // Domyślna wartość
}

#undef LOCTEXT_NAMESPACE

void FPathSplineMetadataDetails::OnPropertyValueChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	// Jeżeli masz dostęp do danych w StructOnScope, możesz je zaktualizować
	FSplinePathPointData* Data = reinterpret_cast<FSplinePathPointData*>(StructOnScope->GetStructMemory());
	if (Data)
	{
		UPathSplineMetadata* PathMetadata = GetPathMetadata();
		if (PathMetadata)
		{
			for (const int32& SelectedKey : SelectedKeys)
			{
				if (PathMetadata && PathMetadata->PointData.IsValidIndex(SelectedKey))
				{
					PathMetadata->PointData[SelectedKey].SplinePathConnections = Data->SplinePathConnections;
					SplinePathConnections = Data->SplinePathConnections;
				}
			}
			if (PathMetadata->GetSplineComponent())
			{
				PathMetadata->GetSplineComponent()->PostEditChange();
			}
		}
	}
}

