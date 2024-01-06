#include "GeometryCollectionTool.h"

#include "ContentBrowserModule.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "GeometryCollection/GeometryCollection.h"

#include "GeometryCollectionProxyData.h"

static const FName GeometryCollectionToolTabName("GeometryCollectionTool");

#define LOCTEXT_NAMESPACE "GeometryCollectionToolModule"

void FGeometryCollectionToolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	//FESExtractAnimRootMotionCurveToolStyle::Initialize();
	//FESExtractAnimRootMotionCurveToolStyle::ReloadTextures();

	//FESExtractAnimRootMotionCurveToolCommands::Register();

	// PluginCommands = MakeShareable(new FUICommandList);
	// PluginCommands->MapAction(
	// 	FESExtractAnimRootMotionCurveToolCommands::Get().ExtractRootCurve,
	// 	FExecuteAction::CreateRaw(this, &FESExtractAnimRootMotionCurveToolModule::OnExtractRootMotionCurveMenuClick),
	// 	FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGeometryCollectionToolModule::RegisterMenus));
}

void FGeometryCollectionToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	//FESExtractAnimRootMotionCurveToolStyle::Shutdown();

	//FESExtractAnimRootMotionCurveToolCommands::Unregister();

	/*IAnimationEditorModule* AnimationEditorModule = FModuleManager::Get().GetModulePtr<IAnimationEditorModule>("AnimationEditor");
	if (AnimationEditorModule)
	{
		typedef IAnimationEditorModule::FAnimationEditorToolbarExtender DelegateType;
		AnimationEditorModule->GetAllAnimationEditorToolbarExtenders().RemoveAll([=](const DelegateType& In) { return In.GetHandle() == AnimationEditorExtenderHandle; });
	}*/
}

void SaveGeometryCollectionToJson(FGeometryCollection* Geometry, bool bUseMeter=true)
{
	auto ConvertLocation = [&bUseMeter](const FVector& InLoc)->FVector {
		return bUseMeter? InLoc*0.01 : InLoc;
	};

	if (Geometry)
	{
		// Transform Group
		auto Transform = Geometry->FindAttribute<FTransform>(FTransformCollection::TransformAttribute, FTransformCollection::TransformGroup);
		auto Parent = Geometry->FindAttribute<int32>(FTransformCollection::ParentAttribute, FTransformCollection::TransformGroup);
		auto Children = Geometry->FindAttribute<TSet<int32>>(FTransformCollection::ChildrenAttribute, FTransformCollection::TransformGroup);
		auto Implicits = Geometry->FindAttribute<FGeometryDynamicCollection::FSharedImplicit>(FGeometryDynamicCollection::ImplicitsAttribute, FTransformCollection::TransformGroup);
		auto SimulationType = Geometry->FindAttribute<int32>(FGeometryCollection::SimulationTypeAttribute, FTransformCollection::TransformGroup);
		auto MassToLocal = Geometry->FindAttribute<FTransform>("MassToLocal", FTransformCollection::TransformGroup);

		// Vertices Group
		auto Vertex = Geometry->FindAttribute<FVector3f>("Vertex", FGeometryCollection::VerticesGroup);
		auto Normal = Geometry->FindAttribute<FVector3f>("Normal", FGeometryCollection::VerticesGroup);
		auto BoneMap = Geometry->FindAttribute<int32>("BoneMap", FGeometryCollection::VerticesGroup);

		// Faces Group
		auto Indices = Geometry->FindAttribute<FIntVector>("Indices", FGeometryCollection::FacesGroup);
		auto Visible = Geometry->FindAttribute<bool>("Visible", FGeometryCollection::FacesGroup);

		// Geometry Group
		auto TransformIndex = Geometry->FindAttribute<int32>("TransformIndex", FGeometryCollection::GeometryGroup);
		auto BoundingBox = Geometry->FindAttribute<FBox>("BoundingBox", FGeometryCollection::GeometryGroup);
		auto VertexStart = Geometry->FindAttribute<int32>("VertexStart", FGeometryCollection::GeometryGroup);
		auto VertexCount = Geometry->FindAttribute<int32>("VertexCount", FGeometryCollection::GeometryGroup);
		auto FaceStart = Geometry->FindAttribute<int32>("FaceStart", FGeometryCollection::GeometryGroup);
		auto FaceCount = Geometry->FindAttribute<int32>("FaceCount", FGeometryCollection::GeometryGroup);

		// 

		struct Internal
		{
			static TSharedRef<FJsonValue> CreateJsonValue(const FVector2D& InVec2D)
			{
				TArray<TSharedPtr<FJsonValue>> StructJsonArray;
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec2D.X));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec2D.Y));
				return MakeShared<FJsonValueArray>(StructJsonArray);
			}
			static TSharedRef<FJsonValue> CreateJsonValue(const FMatrix2x2& InMatrix)
			{
				float m00, m01, m10, m11;
				InMatrix.GetMatrix(m00, m01, m10, m11);

				TArray<TSharedPtr<FJsonValue>> StructJsonArray;
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(m00));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(m01));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(m10));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(m11));
				return MakeShared<FJsonValueArray>(StructJsonArray);
			}
			static TSharedRef<FJsonValue> CreateJsonValue(const FVector& InVec3D)
			{
				TArray<TSharedPtr<FJsonValue>> StructJsonArray;
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec3D.X));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec3D.Y));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec3D.Z));
				return MakeShared<FJsonValueArray>(StructJsonArray);
			}
			static TSharedRef<FJsonValue> CreateJsonValue(const FQuat& InQuat)
			{
				TArray<TSharedPtr<FJsonValue>> StructJsonArray;
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InQuat.X));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InQuat.Y));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InQuat.Z));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InQuat.W));
				return MakeShared<FJsonValueArray>(StructJsonArray);
			}
			static TSharedRef<FJsonValue> CreateJsonValue(const FIntVector& InVec3D)
			{
				TArray<TSharedPtr<FJsonValue>> StructJsonArray;
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec3D.X));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec3D.Y));
				StructJsonArray.Add(MakeShared<FJsonValueNumber>(InVec3D.Z));
				return MakeShared<FJsonValueArray>(StructJsonArray);
			}
		};

		IFileManager& FileManager = IFileManager::Get();
		const FString TimeStamp = FDateTime::Now().ToFormattedString(TEXT("%Y-%m-%d_%H-%M-%S"));
		FString SavedDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / "Geometry");
		if (!FileManager.DirectoryExists(*SavedDir))
		{
			FileManager.MakeDirectory(*SavedDir);
		}
		FString Path = SavedDir / TimeStamp + ".json";
		//FPaths::MakePathRelativeTo(Path, *FPaths::ProjectDir());

		TSharedRef<FJsonObject> GeometryObject = MakeShared<FJsonObject>();
		// Transform
		{
			TSharedRef<FJsonObject> JsonTransformObject = MakeShared<FJsonObject>();

			TArray< TSharedPtr< FJsonValue > > JsonPositionArray;
			TArray< TSharedPtr< FJsonValue > > JsonRotationArray;
			TArray< TSharedPtr< FJsonValue > > JsonScaleArray;
			for (const auto& O : *Transform)
			{
				JsonPositionArray.Add(Internal::CreateJsonValue(ConvertLocation(O.GetLocation())));
				JsonRotationArray.Add(Internal::CreateJsonValue(O.GetRotation()));
				JsonScaleArray.Add(Internal::CreateJsonValue(O.GetScale3D()));
			}
			JsonTransformObject->SetArrayField("Position", JsonPositionArray);
			JsonTransformObject->SetArrayField("Rotation", JsonRotationArray);
			JsonTransformObject->SetArrayField("Scale", JsonScaleArray);

			GeometryObject->SetObjectField("Transform", JsonTransformObject);
		}

		// Parent
		{
			TArray< TSharedPtr< FJsonValue > > JsonParentArray;
			for (const auto& O : *Parent)
			{
				JsonParentArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("Parent", JsonParentArray);
		}

		// Children
		{
			TArray< TSharedPtr< FJsonValue > > JsonChildrenArray;
			for (const auto& O : *Children)
			{
				TArray< TSharedPtr<FJsonValue> > JsonIndexArray;
				for (const auto& Child : O)
				{
					JsonIndexArray.Add(MakeShared<FJsonValueNumber>(Child));
				}
				JsonChildrenArray.Add(MakeShared<FJsonValueArray>(JsonIndexArray));
			}
			GeometryObject->SetArrayField("Children", JsonChildrenArray);
		}

		// Simulate type (Cluster or Rigid), FGeometryCollection::ESimulationTypes
		{
			TArray< TSharedPtr< FJsonValue > > JsonSimulationTypeArray;
			for (const auto& O : *SimulationType)
			{
				JsonSimulationTypeArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("SimulationType", JsonSimulationTypeArray);
		}

		// MassToLocal
		{
			TSharedRef<FJsonObject> JsonMassToLocalObject = MakeShared<FJsonObject>();

			TArray< TSharedPtr< FJsonValue > > JsonPositionArray;
			TArray< TSharedPtr< FJsonValue > > JsonRotationArray;
			TArray< TSharedPtr< FJsonValue > > JsonScaleArray;
			for (const auto& O : *MassToLocal)
			{
				JsonPositionArray.Add(Internal::CreateJsonValue(ConvertLocation(O.GetLocation())));
				JsonRotationArray.Add(Internal::CreateJsonValue(O.GetRotation()));
				JsonScaleArray.Add(Internal::CreateJsonValue(O.GetScale3D()));
			}
			JsonMassToLocalObject->SetArrayField("Position", JsonPositionArray);
			JsonMassToLocalObject->SetArrayField("Rotation", JsonRotationArray);
			JsonMassToLocalObject->SetArrayField("Scale", JsonScaleArray);

			GeometryObject->SetObjectField("MassToLocal", JsonMassToLocalObject);
		}

		// Vertex
		{
			TArray< TSharedPtr< FJsonValue > > JsonVertexArray;
			for (const auto& O : *Vertex)
			{
				JsonVertexArray.Add(Internal::CreateJsonValue(ConvertLocation(FVector(O))));
			}
			GeometryObject->SetArrayField("Vertex", JsonVertexArray);
		}

		// Indices
		{
			TArray< TSharedPtr< FJsonValue > > JsonIndicesArray;
			for (const auto& O : *Indices)
			{
				JsonIndicesArray.Add(Internal::CreateJsonValue(O));
			}
			GeometryObject->SetArrayField("Indices", JsonIndicesArray);
		}

		// Bone map
		{
			TArray< TSharedPtr< FJsonValue > > JsonBoneMapArray;
			for (const auto& O : *BoneMap)
			{
				JsonBoneMapArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("BoneMap", JsonBoneMapArray);
		}

		// Geometry Group -----------------------------------------------------------

		// Transform index
		{
			TArray< TSharedPtr< FJsonValue > > JsonTransformIndexArray;
			for (const auto& O : *TransformIndex)
			{
				JsonTransformIndexArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("TransfromIndex", JsonTransformIndexArray);
		}

		// Face start
		{
			TArray< TSharedPtr< FJsonValue > > JsonFaceStartArray;
			for (const auto& O : *FaceStart)
			{
				JsonFaceStartArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("FaceStart", JsonFaceStartArray);
		}

		// Face count
		{
			TArray< TSharedPtr< FJsonValue > > JsonFaceCountArray;
			for (const auto& O : *FaceCount)
			{
				JsonFaceCountArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("FaceCount", JsonFaceCountArray);
		}

		// Vertex start
		{
			TArray< TSharedPtr< FJsonValue > > JsonVertexStartArray;
			for (const auto& O : *VertexStart)
			{
				JsonVertexStartArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("VertexStart", JsonVertexStartArray);
		}

		// Vertex count
		{
			TArray< TSharedPtr< FJsonValue > > JsonVertexCountArray;
			for (const auto& O : *VertexCount)
			{
				JsonVertexCountArray.Add(MakeShared<FJsonValueNumber>(O));
			}
			GeometryObject->SetArrayField("VertexCount", JsonVertexCountArray);
		}

		// Visible
		{
			TArray< TSharedPtr< FJsonValue > > JsonVisibleArray;
			const auto& VisibleArray = *Visible;
			for (int32 i = 0; i < VisibleArray.Num(); ++i)
			{

				JsonVisibleArray.Add(MakeShared<FJsonValueBoolean>(VisibleArray[i]));
			}
			GeometryObject->SetArrayField("Visible", JsonVisibleArray);
		}

		// Implicits
		{
			TArray< TSharedPtr< FJsonValue > > JsonImplicitsArray;
			for (const auto& O : *Implicits)
			{
				TSharedPtr<FJsonObject> JsonImplicitEntry = MakeShareable(new FJsonObject);
				{
					TArray< TSharedPtr<FJsonValue> > JsonVertexArray;
					TArray< TSharedPtr<FJsonValue> > JsonFaceArray;

					if (!O || !O->IsConvex())
					{
						JsonImplicitEntry->SetArrayField(TEXT("Vertex"), JsonVertexArray);
						JsonImplicitEntry->SetArrayField(TEXT("Face"), JsonFaceArray);
					}
					else
					{
						auto Convex = O->AsAChecked<Chaos::FConvex>();
						// 1. Vertex
						{
							const auto& Vertices = Convex->GetVertices();
							for (int Index = 0; Index < Vertices.Num(); ++Index)
							{
								JsonVertexArray.Add(Internal::CreateJsonValue(ConvertLocation(FVector(Vertices[Index]))));
							}

							JsonImplicitEntry->SetArrayField(TEXT("Vertex"), JsonVertexArray);
						}

						// 2. Plane vertex (for debug draw)
						{
							for (int32 PlaneIndex = 0; PlaneIndex < Convex->GetFaces().Num(); ++PlaneIndex)
							{
								TArray< TSharedPtr<FJsonValue> > JsonIndexArray;

								const int32 PlaneVerticesNum = Convex->NumPlaneVertices(PlaneIndex);
								for (int32 PlaneVertexIndex = 0; PlaneVertexIndex < PlaneVerticesNum; ++PlaneVertexIndex)
								{
									const int32 VertexIndex = Convex->GetPlaneVertex(PlaneIndex, PlaneVertexIndex);
									JsonIndexArray.Add(MakeShared<FJsonValueNumber>(VertexIndex));
								}
								JsonFaceArray.Add(MakeShared<FJsonValueArray>(JsonIndexArray));
							}
							JsonImplicitEntry->SetArrayField(TEXT("Face"), JsonFaceArray);
						}
					}
				}

				JsonImplicitsArray.Add(MakeShared<FJsonValueObject>(JsonImplicitEntry));
			}
			GeometryObject->SetArrayField("Implicits", JsonImplicitsArray);
		}

		FString JsonString;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString, 0);
		FJsonSerializer::Serialize(GeometryObject, JsonWriter, true);

		FFileHelper::SaveStringToFile(JsonString, *Path);
	}
}

void FGeometryCollectionToolModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& MenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	MenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateLambda([this](const TArray<FAssetData>& SelectedAssets)
		{
			TSharedRef<FExtender> Extender(new FExtender());

			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda(
					[SelectedAssets, this](FMenuBuilder& MenuBuilder)
					{
						auto CanExecuteAction = [=]
						{
							if (SelectedAssets.Num() <= 0) return false;

							for (auto& SelectedAsset : SelectedAssets)
							{
								if (!SelectedAsset.IsValid()) return false;

								if (!(SelectedAsset.GetClass()->IsChildOf(UGeometryCollection::StaticClass()) ||
									SelectedAsset.GetClass()->IsChildOf(UGeometryCollection::StaticClass())))
									return false;
							}
							return true;
						};

						MenuBuilder.AddMenuEntry(
							LOCTEXT("Extract Geometry Collection Data", "GeometryCollection (cm)"),
							LOCTEXT("Extract Geometry Collection Data", "GeometryCollection (cm)"),
							FSlateIcon(),
							FUIAction(
								FExecuteAction::CreateLambda([SelectedAssets, this]()
									{
										for (auto& SelectedAsset : SelectedAssets)
										{
											const auto Collection = Cast<UGeometryCollection>(SelectedAsset.GetAsset());
											if (Collection != nullptr)
											{
												SaveGeometryCollectionToJson(Collection->GetGeometryCollection().Get(), false);
											}
										}
									}),
								FCanExecuteAction::CreateLambda(CanExecuteAction)
							)
						);
						MenuBuilder.AddMenuEntry(
							LOCTEXT("Extract Geometry Collection Data", "GeometryCollection (m)"),
							LOCTEXT("Extract Geometry Collection Data", "GeometryCollection (m)"),
							FSlateIcon(),
							FUIAction(
								FExecuteAction::CreateLambda([SelectedAssets, this]()
									{
										for (auto& SelectedAsset : SelectedAssets)
										{
											const auto Collection = Cast<UGeometryCollection>(SelectedAsset.GetAsset());
											if (Collection != nullptr)
											{
												SaveGeometryCollectionToJson(Collection->GetGeometryCollection().Get(), true);
											}
										}
									}),
								FCanExecuteAction::CreateLambda(CanExecuteAction)
							)
						);
					})
			);
			return Extender;
		}));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGeometryCollectionToolModule, GeometryCollectionToolModule)