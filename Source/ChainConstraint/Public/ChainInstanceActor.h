#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChainProfile.h"
#include "ChainInstanceActor.generated.h"

class UStaticMeshComponent;
class UPhysicsConstraintComponent;

/**
 * Chain anchor definition: can be a world location or a component/socket.
 */
USTRUCT(BlueprintType)
struct FChainAnchor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	TObjectPtr<USceneComponent> Component = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	FVector WorldLocation = FVector::ZeroVector;

	/** If true, use WorldLocation instead of Component/Socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	bool bUseWorldLocation = true;

	FVector ResolveLocation() const
	{
		if (!Component)
		{
			return WorldLocation;
		}

		if (SocketName != NAME_None)
		{
			return Component->GetSocketLocation(SocketName);
		}

		return Component->GetComponentLocation();
	}
};

/**
 * AChainInstanceActor:
 * - Consumes UChainProfile
 * - Generates runtime links + constraints
 * - Manages anchors and dynamic behavior
 * - Server-authoritative physics
 */
UCLASS()
class YOURMODULE_API AChainInstanceActor : public AActor
{
	GENERATED_BODY()

public:
	AChainInstanceActor();

	/** Data Asset describing the chain (mesh, physics, constraints, LOD, network). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	TObjectPtr<UChainProfile> Profile;

	/** Start / End anchors (socket, component, or world location). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain|Anchors")
	FChainAnchor StartAnchor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain|Anchors")
	FChainAnchor EndAnchor;

	/** If true, rebuilds chain automatically when anchors are modified. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	bool bAutoRebuild = false;

	/** Current effective segment count (after LOD). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chain")
	int32 CurrentSegmentCount;

	/** Dynamic arrays holding mesh links and constraints. */
	UPROPERTY(VisibleAnywhere, Category = "Chain|Runtime")
	TArray<TObjectPtr<UStaticMeshComponent>> LinkComponents;

	UPROPERTY(VisibleAnywhere, Category = "Chain|Runtime")
	TArray<TObjectPtr<UPhysicsConstraintComponent>> ConstraintComponents;

protected:

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Build chain using the assigned profile. */
	UFUNCTION(BlueprintCallable, Category = "Chain")
	void InitializeFromProfile();

	/** Destroy previous chain and rebuild a new one. */
	UFUNCTION(BlueprintCallable, Category = "Chain")
	void RebuildChain();

	/** Core generation function: creates links + constraints. */
	void BuildChain();

	/** Apply profile settings (mesh, mass, collision, damping…). */
	void ApplyProfileToLink(UStaticMeshComponent* Link);

	/** Apply profile constraint settings to a joint. */
	void ApplyProfileToConstraint(UPhysicsConstraintComponent* Constraint);

	/** Anchor link 0 to StartAnchor, link N to EndAnchor (if any). */
	void BindAnchors();

	/** Removes existing links and constraints. */
	void ClearChain();

public:

	/** Anchor manipulation API */
	UFUNCTION(BlueprintCallable, Category = "Chain|Anchors")
	void SetStartAnchor(const FChainAnchor& NewAnchor);

	UFUNCTION(BlueprintCallable, Category = "Chain|Anchors")
	void SetEndAnchor(const FChainAnchor& NewAnchor);

	/** Rope-like dynamic length changes (grappling hook). */
	UFUNCTION(BlueprintCallable, Category = "Chain|Dynamics")
	void SetTargetLength(float NewLength);

	/** Break an individual link constraint (destructible chain). */
	UFUNCTION(BlueprintCallable, Category = "Chain|Dynamics")
	void BreakLink(int32 LinkIndex);
};
