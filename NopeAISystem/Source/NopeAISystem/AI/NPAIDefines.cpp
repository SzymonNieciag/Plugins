#include "NPAIDefines.h"
#include <Kismet/KismetMathLibrary.h>

ENUM_RANGE_BY_COUNT(ENPAIActionState, ENPAIActionState::MAX)

FNPAITaskProgress::FNPAITaskProgress()
{
	for (ENPAIActionState Val : TEnumRange<ENPAIActionState>())
	{
		RequirmentState.Add(Val);
	}
}

void FNPAITaskProgress::ResetTasks()
{
	if (bIsInitialized)
	{
		LeftTaskTime = InitLeftTaskTime;
		CurProgress = InitCurProgress;
		LeftRepeatCount = InitLeftRepeatCount;
		bIsInitialized = false;
		bIsFinished = false;
	}
}

void FNPAITaskProgress::InitTasks()
{
	if (!bIsInitialized)
	{
		InitLeftTaskTime = LeftTaskTime;
		if (MaxLeftTaskTime != 0.0f)
		{
			LeftTaskTime = FMath::RandRange(LeftTaskTime, MaxLeftTaskTime);
		}
		InitCurProgress = CurProgress;
		InitLeftRepeatCount = LeftRepeatCount;
		bIsInitialized = true;
	}
}

bool FNPAITaskProgress::CanUpdateTime(ENPAIActionState AIActionState)
{
	int Index = RequirmentState.Find(AIActionState);
	if (Index != -1)
	{
		return true;
	}
	return false;
}

void FNPTimeStruct::UpdateTimeStruct(float InTotalTime)
{
	int IntTotalTime = FMath::RoundToInt(InTotalTime);
	Minutes = IntTotalTime % 60;
	Hours = (IntTotalTime / 60) % 24;
	Days = (IntTotalTime / 1440);
	TotalTime = IntTotalTime;
}

FNPTimeStruct& FNPTimeStruct::operator=(const FNPTimeStruct& InOther)
{
	Days = InOther.Days;
	Hours = InOther.Hours;
	Minutes = InOther.Minutes;
	TotalTime = InOther.TotalTime;
	// return the existing object so we can chain this operator
	return *this;
}

FNPTimeStruct FNPTimeStruct::RandomTimeInRange(const FNPTimeStruct& MinRandomSpawnTime, const FNPTimeStruct& MaxRandomSpawnTime)
{
	int Minute = UKismetMathLibrary::RandomIntegerInRange(MinRandomSpawnTime.Minutes, MaxRandomSpawnTime.Minutes);
	int Hour = UKismetMathLibrary::RandomIntegerInRange(MinRandomSpawnTime.Hours, MaxRandomSpawnTime.Hours);
	int Day = UKismetMathLibrary::RandomIntegerInRange(MinRandomSpawnTime.Days, MaxRandomSpawnTime.Days);
	return FNPTimeStruct(Day, Hour, Minute);
}

FNPAITaskStruct::FNPAITaskStruct()
{
	TasksProgress.Add(FNPAITaskProgress());
	BeginValidTime.Days = 0;
	BeginValidTime.Hours = 0;
	BeginValidTime.Minutes = 0;

	EndValidTime.Days = 0;
	EndValidTime.Hours = 23;
	EndValidTime.Minutes = 59;
}
