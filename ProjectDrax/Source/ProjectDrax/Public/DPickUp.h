// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DPickUp.generated.h"

UENUM(BlueprintType)
namespace ECategory
{
	enum CategoryType
	{
		EBMeds			UMETA(DisplayName = "Meds"),
		EAmmo			UMETA(DisplayName = "Ammo"),
		EThrowables		UMETA(DisplayName = "Throwables"),
		EAttachments    UMETA(DisplayName = "Attachments")
	};
}

USTRUCT(BlueprintType)
struct FPickUpData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Desc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<ECategory::CategoryType> Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Heal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Boost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* SplashArt;
};

UCLASS()
class PROJECTDRAX_API ADPickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADPickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FPickUpData PickUP;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UStaticMeshComponent* MeshComp;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision)
		class UBoxComponent* CollisionComp;*/

	UPROPERTY(EditAnywhere,Category="PickUP",BlueprintReadWrite)
		class UDataTable* PickUpDataTable;
public:	
	// Called every frame
	void DestroyPickUp();
};
