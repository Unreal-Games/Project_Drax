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

	UPROPERTY(EditAnywhere)
		FString Name;

	UPROPERTY(EditAnywhere)
		FString Desc;

	UPROPERTY(EditAnywhere)
	class USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere)
		int32 Amount;

	UPROPERTY(EditAnywhere)
		float Weight;

	UPROPERTY(EditAnywhere)
		TEnumAsByte<ECategory::CategoryType> Category;

	UPROPERTY(EditAnywhere)
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

	UPROPERTY(EditAnywhere,Category="PickUP")
		class UDataTable* PickUpDataTable;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
