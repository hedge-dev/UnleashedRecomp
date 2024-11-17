#pragma once

#include "Hedgehog/Base/hhObject.h"
#include "Hedgehog/Base/System/hhAllocator.h"
#include "Hedgehog/Base/Thread/hhHolder.h"
#include "Hedgehog/Base/Thread/hhHolderBase.h"
#include "Hedgehog/Base/Thread/hhSynchronizedObject.h"
#include "Hedgehog/Base/Thread/hhSynchronizedPtr.h"
#include "Hedgehog/Base/Type/hhSharedString.h"
#include "Hedgehog/Database/System/hhDatabaseData.h"
#include "Hedgehog/Math/Vector2.h"
#include "Hedgehog/MirageCore/Renderable/hhRenderable.h"
#include "Hedgehog/Universe/Engine/hhMessageActor.h"
#include "Hedgehog/Universe/Engine/hhMessageProcess.h"
#include "Hedgehog/Universe/Engine/hhUpdateInfo.h"
#include "Hedgehog/Universe/Engine/hhUpdateUnit.h"
#include "Hedgehog/Universe/Thread/hhParallelJob.h"

#include "CSD/Manager/csdmBase.h"
#include "CSD/Manager/csdmNode.h"
#include "CSD/Manager/csdmNodeObserver.h"
#include "CSD/Manager/csdmObserverBase.h"
#include "CSD/Manager/csdmProject.h"
#include "CSD/Manager/csdmRCObject.h"
#include "CSD/Manager/csdmRCObjectImp.h"
#include "CSD/Manager/csdmRCPtr.h"
#include "CSD/Manager/csdmRCPtrAbs.h"
#include "CSD/Manager/csdmResourceBase.h"
#include "CSD/Manager/csdmScene.h"
#include "CSD/Manager/csdmSceneObserver.h"
#include "CSD/Manager/csdmSubjectBase.h"
#include "CSD/Platform/csdTexList.h"

#include "SWA/Camera/Camera.h"
#include "SWA/CSD/CsdDatabaseWrapper.h"
#include "SWA/CSD/CsdProject.h"
#include "SWA/CSD/CsdTexListMirage.h"
#include "SWA/CSD/GameObjectCSD.h"
#include "SWA/Player/Character/EvilSonic/Hud/EvilHudGuide.h"
#include "SWA/Player/Character/EvilSonic/EvilSonic.h"
#include "SWA/Player/Character/EvilSonic/EvilSonicContext.h"
#include "SWA/System/GameMode/Title/TitleMenu.h"
#include "SWA/System/GameMode/Title/TitleStateBase.h"
#include "SWA/System/ApplicationDocument.h"
#include "SWA/System/GameDocument.h"
#include "SWA/System/InputState.h"
#include "SWA/System/PadState.h"
