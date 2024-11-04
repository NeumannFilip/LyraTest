// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Engine/DataAsset.h"
#include "Settings/LyraSettingsShared.h"
#include "UObject/UObjectGlobals.h"

#include "LyraAimSensitivityData.generated.h"

class UObject;

/** Defines a set of gamepad sensitivity to a float value. */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Lyra Aim Sensitivity Data", ShortTooltip = "Data asset used to define a map of Gamepad Sensitivty to a float value."))
class LYRAGAME_API ULyraAimSensitivityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULyraAimSensitivityData(const FObjectInitializer& ObjectInitializer);
	
	const float SensitivtyEnumToFloat(const ELyraGamepadSensitivity InSensitivity) const;
	
protected:
	/** Map of SensitivityMap settings to their corresponding float */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ELyraGamepadSensitivity, float> SensitivityMap;
};