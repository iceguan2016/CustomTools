// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
//#include "ESExtractAnimRootMotionWindow.h"

class FToolBarBuilder;
class FMenuBuilder;

class FGeometryCollectionToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	//void OnExtractRootMotionCurveMenuTick(TWeakPtr<IAnimationEditor>  IAnimationEditor);

	//TSharedRef<FExtender> GetAnimationEditorToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<IAnimationEditor> InAnimationEditor);
	
	//void HandleAddAnimationEditorToolbarExtender(FToolBarBuilder& ParentToolbarBuilder, TWeakPtr<IAnimationEditor>  IAnimationEditor);

private:
	void RegisterMenus();

	//void ExtractRootCurve(UAnimSequence* AnimSequence);

	//void ExtractRootCurve(UAnimMontage* AnimMontage);

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	//TSharedPtr <SESExtractAnimRootMotionWindow> ExtractRootMotionSettingWindow = nullptr;

	FDelegateHandle AnimationEditorExtenderHandle;
};
