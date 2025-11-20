#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h" // ECollisionChannel
#include "UObject/ObjectMacros.h"
#include "ChainProfile.generated.h"

/**
 * High-level classification of the chain behavior.
 * Used for presets and documentation, not hard constraints.
 */
UENUM(BlueprintType)
enum class EChainType : uint8
{
	Rope        UMETA(DisplayName = "Rope / Cable"),
	MetalChain  UMETA(DisplayName = "Metal Chain"),
	Grapple     UMETA(DisplayName = "Grappling Hook"),
	Restraint   UMETA(DisplayName = "Restraint / Shackles"),
	Custom      UMETA(DisplayName = "Custom")
};

/**
 * Network replication strategy for a chain instance.
 * FullRep       : all link transforms are replicated (simpler, more bandwidth).
 * KeyLinksRep   : only a subset of links (root/mid/end) is replicated (cheaper, needs client-side reconstruction).
 * None          : no physical replication (cosmetic or local-only).
 */
UENUM(BlueprintType)
enum class EChainNetworkMode : uint8
{
	FullRep     UMETA(DisplayName = "Full Replication"),
	KeyLinksRep UMETA(DisplayName = "Key Links Only"),
	None        UMETA(DisplayName = "No Network Replication")
};

/**
 * Visual and geometric settings for individual links composing the chain.
 */
USTRUCT(BlueprintType)
struct FChainVisualSettings
{
	GENERATED_BODY()

	/** Static mesh used for each chain link. Oriented along X by default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMesh> LinkMesh = nullptr;

	/** Optional relative transform applied to each link mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FTransform LinkRelativeTransform = FTransform::Identity;

	/** Default total segment count for this profile (before LOD overrides). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", meta = (ClampMin = "2", UIMin = "2"))
	int32 DefaultSegmentCount = 8;

	/** Default total chain length in centimeters (before runtime overrides). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", meta = (ClampMin = "1.0", UIMin = "10.0"))
	float DefaultLength = 500.0f;

	/** If true, all segments share the same length. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	bool bUniformSegmentLength = true;
};

/**
 * Physical properties for individual chain links.
 */
USTRUCT(BlueprintType)
struct FChainLinkPhysicsSettings
{
	GENERATED_BODY()

	/** Mass of each link in kilograms. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0.001", UIMin = "0.01"))
	float LinkMass = 1.0f;

	/** Linear damping applied to each link rigid body. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0.0"))
	float LinearDamping = 0.1f;

	/** Angular damping applied to each link rigid body. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics", meta = (ClampMin = "0.0"))
	float AngularDamping = 0.1f;

	/** Collision channel used by the chain links. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_PhysicsBody;

	/** Optional collision profile name for the links. If set, overrides CollisionChannel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	FName CollisionProfileName = NAME_None;

	/** If true, links can collide with each other (more expensive, more realistic). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	bool bEnableSelfCollision = false;
};

/**
 * Constraint (joint) settings between two adjacent chain links.
 * These parameters are mapped to Chaos joint / constraint settings.
 */
USTRUCT(BlueprintType)
struct FChainConstraintSettings
{
	GENERATED_BODY()

	/** Enables angular swing limits around the constraint. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Angular")
	bool bEnableSwing = true;

	/** Maximum swing angle in degrees around the primary axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Angular", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxSwingAngle = 45.0f;

	/** Enables twist limits around the link axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Angular")
	bool bEnableTwist = false;

	/** Maximum twist angle in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Angular", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxTwistAngle = 20.0f;

	/** Linear distance limit between two links, in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Linear", meta = (ClampMin = "0.0"))
	float LinearLimit = 0.0f;

	/** Stiffness for the linear constraint (position). Higher values => stiffer chain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Linear", meta = (ClampMin = "0.0"))
	float LinearStiffness = 50000.0f;

	/** Stiffness for the angular constraint (rotational). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Angular", meta = (ClampMin = "0.0"))
	float AngularStiffness = 50000.0f;

	/** Force threshold at which the constraint breaks (0 = unbreakable). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Break", meta = (ClampMin = "0.0"))
	float BreakForce = 0.0f;

	/** Torque threshold at which the constraint breaks (0 = unbreakable). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraint|Break", meta = (ClampMin = "0.0"))
	float BreakTorque = 0.0f;
};

/**
 * LOD (Level Of Detail) settings for a chain profile.
 * Used to reduce cost of simulation and collisions based on distance.
 */
USTRUCT(BlueprintType)
struct FChainLODLevel
{
	GENERATED_BODY()

	/** Minimum camera distance for this LOD to be considered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (ClampMin = "0.0"))
	float MinDistance = 0.0f;

	/** Maximum camera distance for this LOD to be considered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (ClampMin = "0.0"))
	float MaxDistance = 10000.0f;

	/** Optional override for segment count at this LOD. <= 0 = use profile default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (ClampMin = "0"))
	int32 SegmentCountOverride = 0;

	/** If false, physics simulation can be disabled for this LOD. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
	bool bSimulatePhysics = true;

	/** If false, collisions can be disabled for this LOD. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
	bool bEnableCollisions = true;

	/** Optional tick rate factor for simulation (1.0 = every frame, 0.5 = every other frame, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD", meta = (ClampMin = "0.01"))
	float SimulationRateFactor = 1.0f;
};

/**
 * Network-related hints for chain instances using this profile.
 * The actual implementation lives in the runtime actor, but the intent is defined here.
 */
USTRUCT(BlueprintType)
struct FChainNetworkSettings
{
	GENERATED_BODY()

	/** Network replication mode for chains using this profile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network")
	EChainNetworkMode NetworkMode = EChainNetworkMode::KeyLinksRep;

	/** If true, the root of the chain will use standard movement replication. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network")
	bool bReplicateRootTransform = true;

	/**
	 * When using KeyLinksRep, this defines how many key links (besides root)
	 * should be replicated (e.g. mid, end). 0 = auto.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network", meta = (ClampMin = "0", ClampMax = "8"))
	int32 ReplicatedKeyLinksCount = 2;
};

/**
 * Data Asset describing a reusable chain configuration:
 * visual, physical, constraint, LOD and network behavior.
 */
UCLASS(BlueprintType)
class YOURMODULE_API UChainProfile : public UDataAsset
{
	GENERATED_BODY()

public:

	UChainProfile();

	/** High-level chain type for documentation and presets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
	EChainType ChainType = EChainType::Rope;

	/** Visual and geometric settings for the chain links. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
	FChainVisualSettings Visual;

	/** Physical properties applied to each chain link. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
	FChainLinkPhysicsSettings Physics;

	/** Constraint settings applied between adjacent chain links. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain")
	FChainConstraintSettings Constraint;

	/** LOD levels for distance-based performance control. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain|LOD")
	TArray<FChainLODLevel> LODLevels;

	/** Network replication hints for instances using this profile. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain|Network")
	FChainNetworkSettings NetworkSettings;

	/** If true, the last link can remain unattached and behave as a loose end. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain|Behavior")
	bool bSupportsLooseEnd = true;

	/**
	 * If true, the chain length can be modified at runtime (e.g. grappling hook or retractable chain).
	 * The runtime actor is responsible for enforcing this behavior.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain|Behavior")
	bool bAllowDynamicLengthChange = true;

	/**
	 * If true, the chain rest pose is defined in world space (e.g. hangs under gravity).
	 * If false, it can follow an initial authored pose when the anchors move.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chain|Behavior")
	bool bUseWorldSpaceRestPose = true;

public:

	/** Returns the base segment count defined by the profile (ignoring LOD). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chain|Profile")
	int32 GetBaseSegmentCount() const;

	/** Returns the base chain length defined by the profile (ignoring LOD). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chain|Profile")
	float GetBaseLength() const;

	/**
	 * Resolves an effective segment count for a given camera distance.
	 * Uses LOD overrides if any LOD level matches the distance.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chain|Profile")
	int32 GetSegmentCountAtDistance(float Distance) const;

	/**
	 * Returns the LOD index used for the given distance, or INDEX_NONE if no LOD matches.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chain|Profile")
	int32 GetLODIndexForDistance(float Distance) const;
};
