#include "NPPathFollowingComponent.h"
#include <../../../../../../../Source/Runtime/NavigationSystem/Public/NavigationSystem.h>
#include "NopeAISystem/Navigation/NPNavLinkCustomComponent.h"

UNPPathFollowingComponent::UNPPathFollowingComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/) : Super(ObjectInitializer)
{

}

void UNPPathFollowingComponent::SetMoveSegment(int32 SegmentStartIndex)
{
	Super::SetMoveSegment(SegmentStartIndex);
	int32 EndSegmentIndex = SegmentStartIndex + 1;
	const FNavigationPath* PathInstance = Path.Get();
	if (PathInstance != nullptr && PathInstance->GetPathPoints().IsValidIndex(SegmentStartIndex) && PathInstance->GetPathPoints().IsValidIndex(EndSegmentIndex))
	{
		EndSegmentIndex = DetermineCurrentTargetPathPoint(SegmentStartIndex);

		const FNavPathPoint& PathPt1 = PathInstance->GetPathPoints()[EndSegmentIndex];

		// handle moving through custom nav links
		if (PathPt1.CustomNavLinkId != FNavLinkId::Invalid)
		{
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
			INavLinkCustomInterface* CustomNavLink = NavSys->GetCustomLink(PathPt1.CustomNavLinkId);

			UNPNavLinkCustomComponent* NPNavLinkCustomComponent = Cast<UNPNavLinkCustomComponent>(CustomNavLink);
			if (NPNavLinkCustomComponent)
			{
				CurrentAcceptanceRadius = NPNavLinkCustomComponent->AcceptableRadius;
			}
		}
		OnCurrentAcceptanceRadiusChanged.Broadcast(CurrentAcceptanceRadius);
	}
}

bool UNPPathFollowingComponent::IsPointReached(const FVector& GoalLocation, float GoalRadius, float GoalHalfHeight, const FVector& AgentLocation, float RadiusThreshold)
{
	return HasReachedInternal(GoalLocation, 0, 0, AgentLocation, RadiusThreshold, MinAgentRadiusPct);
}
