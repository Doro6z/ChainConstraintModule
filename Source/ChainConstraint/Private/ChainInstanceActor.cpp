#include "ChainInstanceActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AChainInstanceActor::AChainInstanceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AChainInstanceActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitializeFromProfile();
	}
}

void AChainInstanceActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Profile && bAutoRebuild)
	{
		InitializeFromProfile();
	}
}

void AChainInstanceActor::InitializeFromProfile()
{
	if (!Profile) return;

	RebuildChain();
}

void AChainInstanceActor::RebuildChain()
{
	ClearChain();
	BuildChain();
	BindAnchors();
}

void AChainInstanceActor::ClearChain()
{
	for (UStaticMeshComponent* Comp : LinkComponents)
	{
		if (Comp) Comp->DestroyComponent();
	}
	LinkComponents.Empty();

	for (UPhysicsConstraintComponent* Const : ConstraintComponents)
	{
		if (Const) Const->DestroyComponent();
	}
	ConstraintComponents.Empty();
}

void AChainInstanceActor::BuildChain()
{
	if (!Profile)
		return;

	CurrentSegmentCount = Profile->Visual.DefaultSegmentCount;

	// Safety
	CurrentSegmentCount = FMath::Max(2, CurrentSegmentCount);

	// Create N link components
	for (int32 i = 0; i < CurrentSegmentCount; ++i)
	{
		FName CompName = FName(*FString::Printf(TEXT("Link_%d"), i));
		UStaticMeshComponent* Link = NewObject<UStaticMeshComponent>(this, CompName);

		Link->SetupAttachment(RootComponent);
		Link->RegisterComponent();

		ApplyProfileToLink(Link);
		LinkComponents.Add(Link);
	}

	// Create constraints between consecutive links
	for (int32 i = 0; i < CurrentSegmentCount - 1; ++i)
	{
		FName CName = FName(*FString::Printf(TEXT("Constraint_%d"), i));

		UPhysicsConstraintComponent* Constraint = NewObject<UPhysicsConstraintComponent>(this, CName);
		Constraint->SetupAttachment(RootComponent);
		Constraint->RegisterComponent();

		Constraint->SetConstrainedComponents(
			LinkComponents[i],
			NAME_None,
			LinkComponents[i + 1],
			NAME_None
		);

		ApplyProfileToConstraint(Constraint);
		ConstraintComponents.Add(Constraint);
	}
}

void AChainInstanceActor::ApplyProfileToLink(UStaticMeshComponent* Link)
{
	if (!Link || !Profile) return;

	const FChainLinkPhysicsSettings& Phys = Profile->Physics;
	const FChainVisualSettings& Vis = Profile->Visual;

	Link->SetStaticMesh(Vis.LinkMesh);
	Link->SetRelativeTransform(Vis.LinkRelativeTransform);

	Link->SetSimulatePhysics(true);
	Link->SetMassOverrideInKg(NAME_None, Phys.LinkMass, true);
	Link->SetLinearDamping(Phys.LinearDamping);
	Link->SetAngularDamping(Phys.AngularDamping);

	if (Phys.CollisionProfileName != NAME_None)
	{
		Link->SetCollisionProfileName(Phys.CollisionProfileName);
	}
	else
	{
		Link->SetCollisionObjectType(Phys.CollisionChannel);
	}

	Link->SetNotifyRigidBodyCollision(true);
	Link->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Link->SetVisibility(true);
}

void AChainInstanceActor::ApplyProfileToConstraint(UPhysicsConstraintComponent* Constraint)
{
	if (!Constraint || !Profile) return;

	const FChainConstraintSettings& C = Profile->Constraint;

	// Angular
	Constraint->SetAngularSwing1Limit(C.bEnableSwing ? ACM_Limited : ACM_Free, C.MaxSwingAngle);
	Constraint->SetAngularSwing2Limit(C.bEnableSwing ? ACM_Limited : ACM_Free, C.MaxSwingAngle);
	Constraint->SetAngularTwistLimit(C.bEnableTwist ? ACM_Limited : ACM_Free, C.MaxTwistAngle);

	// Linear
	if (C.LinearLimit > 0)
	{
		Constraint->SetLinearXLimit(ACM_Limited, C.LinearLimit);
		Constraint->SetLinearYLimit(ACM_Limited, C.LinearLimit);
		Constraint->SetLinearZLimit(ACM_Limited, C.LinearLimit);
	}
	else
	{
		Constraint->SetLinearXLimit(ACM_Free, 0.f);
		Constraint->SetLinearYLimit(ACM_Free, 0.f);
		Constraint->SetLinearZLimit(ACM_Free, 0.f);
	}

	// Stiffness
	Constraint->SetLinearDriveParams(C.LinearStiffness, 0.f, 0.f);
	Constraint->SetAngularDriveParams(C.AngularStiffness, 0.f, 0.f);

	// Breaking
	Constraint->ConstraintInstance.ProfileInstance.LinearBreakThreshold = C.BreakForce;
	Constraint->ConstraintInstance.ProfileInstance.AngularBreakThreshold = C.BreakTorque;
}

void AChainInstanceActor::BindAnchors()
{
	if (LinkComponents.Num() == 0) return;

	// Anchor link 0
	{
		if (StartAnchor.bUseWorldLocation)
		{
			LinkComponents[0]->SetWorldLocation(StartAnchor.WorldLocation);
		}
		else if (StartAnchor.Component)
		{
			LinkComponents[0]->AttachToComponent(StartAnchor.Component,
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				StartAnchor.SocketName);
		}
	}

	// Anchor last link
	if (Profile->bSupportsLooseEnd == false && LinkComponents.IsValidIndex(LinkComponents.Num() - 1))
	{
		if (!EndAnchor.bUseWorldLocation && EndAnchor.Component)
		{
			LinkComponents.Last()->AttachToComponent(EndAnchor.Component,
				FAttachmentTransformRules::KeepWorldTransform,
				EndAnchor.SocketName);
		}
		else if (EndAnchor.bUseWorldLocation)
		{
			LinkComponents.Last()->SetWorldLocation(EndAnchor.WorldLocation);
		}
	}
}

void AChainInstanceActor::SetStartAnchor(const FChainAnchor& NewAnchor)
{
	StartAnchor = NewAnchor;
	if (bAutoRebuild)
	{
		RebuildChain();
	}
}

void AChainInstanceActor::SetEndAnchor(const FChainAnchor& NewAnchor)
{
	EndAnchor = NewAnchor;
	if (bAutoRebuild)
	{
		RebuildChain();
	}
}

void AChainInstanceActor::SetTargetLength(float NewLength)
{
	if (!Profile || !Profile->bAllowDynamicLengthChange) return;

	// Placeholder: we will handle dynamic constraint re-param later.
	UE_LOG(LogTemp, Warning, TEXT("Dynamic length change not implemented yet."));
}

void AChainInstanceActor::BreakLink(int32 LinkIndex)
{
	if (!ConstraintComponents.IsValidIndex(LinkIndex))
		return;

	UPhysicsConstraintComponent* C = ConstraintComponents[LinkIndex];
	if (C)
	{
		C->BreakConstraint();
	}
}
