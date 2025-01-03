#include <user/config.h>
#include <api/SWA.h>
#include <ui/game_window.h>

using SVertexData = SWA::Sequence::Utility::CPlayMovieWrapper::CRender::SVertexData;

// Update movie player aspect ratio.
PPC_FUNC_IMPL(__imp__sub_82AE30D8);
PPC_FUNC(sub_82AE30D8)
{
    auto pViewportWidth = (be<uint32_t>*)g_memory.Translate(ctx.r4.u32 + 0x14);
    auto pViewportHeight = (be<uint32_t>*)g_memory.Translate(ctx.r4.u32 + 0x18);

    auto pTopLeft = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C);
    auto pTopRight = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(SVertexData));
    auto pBottomRight = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(SVertexData) * 2);
    auto pBottomLeft = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(SVertexData) * 3);

    auto quadWidth = std::fabs(pTopRight->X - pTopLeft->X) * ((float)*pViewportWidth / 2);
    auto quadHeight = std::fabs(pTopLeft->Y - pBottomLeft->Y) * ((float)*pViewportHeight / 2);

    auto movieAspectRatio = quadWidth / quadHeight;
    auto windowAspectRatio = (float)GameWindow::s_width / (float)GameWindow::s_height;

    auto a = -1.00078f;
    auto b = 1.00139f;
    auto scaleU = 1.0f;
    auto scaleV = 1.0f;
    auto centreV = (pTopLeft->V + pBottomRight->V) / 2.0f;

    if (windowAspectRatio > movieAspectRatio)
    {
        scaleU = movieAspectRatio / windowAspectRatio;
    }
    else
    {
        scaleV = windowAspectRatio / movieAspectRatio;
    }

    pTopLeft->X = a;
    pTopLeft->Y = b;
    pTopLeft->U = (pTopLeft->U - centreV) / scaleU + centreV;
    pTopLeft->V = (pTopLeft->V - centreV) / scaleV + centreV;

    pTopRight->X = b;
    pTopRight->Y = b;
    pTopRight->U = (pTopRight->U - centreV) / scaleU + centreV;
    pTopRight->V = (pTopRight->V - centreV) / scaleV + centreV;

    pBottomLeft->X = a;
    pBottomLeft->Y = a;
    pBottomLeft->U = (pBottomLeft->U - centreV) / scaleU + centreV;
    pBottomLeft->V = (pBottomLeft->V - centreV) / scaleV + centreV;

    pBottomRight->X = b;
    pBottomRight->Y = a;
    pBottomRight->U = (pBottomRight->U - centreV) / scaleU + centreV;
    pBottomRight->V = (pBottomRight->V - centreV) / scaleV + centreV;

    __imp__sub_82AE30D8(ctx, base);
}

void RemoveMoviePlayerLetterboxMidAsmHook() {}

bool MotionBlurMidAsmHook()
{
    return Config::MotionBlur != EMotionBlur::Off;
}

// These are here for now to not recompile basically all of the project.
namespace Chao::CSD
{
    struct Cast
    {
        SWA_INSERT_PADDING(0x144);
    };

    struct CastLink
    {
        be<uint32_t> ChildCastIndex;
        be<uint32_t> SiblingCastIndex;
    };

    struct CastNode
    {
        be<uint32_t> CastCount;
        xpointer<Cast> pCasts;
        be<uint32_t> RootCastIndex;
        xpointer<CastLink> pCastLinks;
    };

    struct CastIndex
    {
        xpointer<const char> pCastName;
        be<uint32_t> CastNodeIndex;
        be<uint32_t> CastIndex;
    };

    struct Scene
    {
        SWA_INSERT_PADDING(0x24);
        be<uint32_t> CastNodeCount;
        xpointer<CastNode> pCastNodes;
        be<uint32_t> CastCount;
        xpointer<CastIndex> pCastIndices;
    };

    struct SceneIndex
    {
        xpointer<const char> pSceneName;
        be<uint32_t> SceneIndex;
    };

    struct SceneNodeIndex
    {
        xpointer<const char> pSceneNodeName;
        be<uint32_t> SceneNodeIndex;
    };

    struct SceneNode
    {
        be<uint32_t> SceneCount;
        xpointer<xpointer<Scene>> pScenes;
        xpointer<SceneIndex> pSceneIndices;
        be<uint32_t> SceneNodeCount;
        xpointer<SceneNode> pSceneNodes;
        xpointer<SceneNodeIndex> pSceneNodeIndices;
    };

    struct Project
    {
        xpointer<SceneNode> pRootNode;
    };
}

static std::map<const void*, XXH64_hash_t> g_paths;

static void EmplacePath(const void* key, const std::string_view& value)
{
    g_paths.emplace(key, XXH3_64bits(value.data(), value.size()));
}

static void TraverseCast(Chao::CSD::Scene* scene, uint32_t castNodeIndex, Chao::CSD::CastNode* castNode, uint32_t castIndex, const std::string& parentPath)
{
    if (castIndex == ~0)
        return;

    TraverseCast(scene, castNodeIndex, castNode, castNode->pCastLinks[castIndex].SiblingCastIndex, parentPath);

    std::string path = parentPath;

    for (size_t i = 0; i < scene->CastCount; i++)
    {
        auto& index = scene->pCastIndices[i];
        if (index.CastNodeIndex == castNodeIndex && index.CastIndex == castIndex)
        {
            path += index.pCastName;
            break;
        }
    }

    EmplacePath(&castNode->pCasts[castIndex], path);
    path += "/";

    TraverseCast(scene, castNodeIndex, castNode, castNode->pCastLinks[castIndex].ChildCastIndex, path);
}

static void TraverseScene(Chao::CSD::Scene* scene, std::string path)
{
    EmplacePath(scene, path);
    path += "/";

    for (size_t i = 0; i < scene->CastNodeCount; i++)
    {
        auto& castNode = scene->pCastNodes[i];
        TraverseCast(scene, i, &castNode, castNode.RootCastIndex, path);
    }
}

static void TraverseSceneNode(Chao::CSD::SceneNode* sceneNode, std::string path)
{
    EmplacePath(sceneNode, path);
    path += "/";

    for (size_t i = 0; i < sceneNode->SceneCount; i++)
    {
        auto& sceneIndex = sceneNode->pSceneIndices[i];
        TraverseScene(sceneNode->pScenes[sceneIndex.SceneIndex], path + sceneIndex.pSceneName.get());
    }

    for (size_t i = 0; i < sceneNode->SceneNodeCount; i++)
    {
        auto& sceneNodeIndex = sceneNode->pSceneNodeIndices[i];
        TraverseSceneNode(&sceneNode->pSceneNodes[sceneNodeIndex.SceneNodeIndex], path + sceneNodeIndex.pSceneNodeName.get());
    }
}

void MakeCsdProjectMidAsmHook(PPCRegister& r3, PPCRegister& r29)
{
    uint8_t* base = g_memory.base;
    auto csdProject = reinterpret_cast<Chao::CSD::CProject*>(base + PPC_LOAD_U32(PPC_LOAD_U32(r3.u32 + 16) + 4));
    auto name = reinterpret_cast<const char*>(base + PPC_LOAD_U32(r29.u32));
    TraverseSceneNode(csdProject->m_pResource->pRootNode, name);
}
