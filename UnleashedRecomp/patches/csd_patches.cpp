#include <user/config.h>
#include <api/SWA.h>
#include <app.h>
#include <ui/game_window.h>
#include <ui/sdl_listener.h>
#include <gpu/video.h>

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
        xpointer<xpointer<Cast>> pCasts;
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

static Mutex g_pathMutex;
static std::map<const void*, XXH64_hash_t> g_paths;

static XXH64_hash_t HashStr(const std::string_view& value)
{
    return XXH3_64bits(value.data(), value.size());
}

static void EmplacePath(const void* key, const std::string_view& value)
{
    std::lock_guard lock(g_pathMutex);
    g_paths.emplace(key, HashStr(value));
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

    EmplacePath(castNode->pCasts[castIndex].get(), path);

    if (castNode->RootCastIndex == castIndex)
        EmplacePath(castNode, path);

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

static constexpr float ORIGINAL_ASPECT_RATIO = 16.0f / 9.0f;

static float g_offsetX;
static float g_offsetY;

static void ComputeOffsets(float width, float height)
{
    float aspectRatio = width / height;
    if (aspectRatio >= ORIGINAL_ASPECT_RATIO)
    {
        g_offsetX = 0.5f * (aspectRatio * 720.0f - 1280.0f);
        g_offsetY = 0.0f;
    }
    else
    {
        g_offsetX = 0.0f;
        g_offsetY = 0.5f * (1280.0f / aspectRatio - 720.0f);
    }
}

static class SDLEventListenerForCSD : public SDLEventListener
{
public:
    void OnSDLEvent(SDL_Event* event) override
    {
        if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)
            ComputeOffsets(event->window.data1, event->window.data2);
    }
} g_sdlEventListenerForCSD;

// Chao::CSD::SetOffsets
PPC_FUNC_IMPL(__imp__sub_830C0A78);
PPC_FUNC(sub_830C0A78)
{
    __imp__sub_830C0A78(ctx, base);

    ComputeOffsets(GameWindow::s_width, GameWindow::s_height);
}

// SWA::CGameDocument::ComputeScreenPosition
PPC_FUNC_IMPL(__imp__sub_8250FC70);
PPC_FUNC(sub_8250FC70)
{
    __imp__sub_8250FC70(ctx, base);

    auto position = reinterpret_cast<be<float>*>(base + ctx.r3.u32);
    position[0] = position[0] - g_offsetX;
    position[1] = position[1] - g_offsetY;
}

void ComputeScreenPositionMidAsmHook(PPCRegister& f1, PPCRegister& f2)
{
    f1.f64 -= g_offsetX;
    f2.f64 -= g_offsetY;
}

enum
{
    ALIGN_CENTER = 0 << 0,

    ALIGN_TOP = 1 << 0,
    ALIGN_LEFT = 1 << 1,
    ALIGN_BOTTOM = 1 << 2,
    ALIGN_RIGHT = 1 << 3,

    ALIGN_TOP_LEFT = ALIGN_TOP | ALIGN_LEFT,
    ALIGN_TOP_RIGHT = ALIGN_TOP | ALIGN_RIGHT,
    ALIGN_BOTTOM_LEFT = ALIGN_BOTTOM | ALIGN_LEFT,
    ALIGN_BOTTOM_RIGHT = ALIGN_BOTTOM | ALIGN_RIGHT,

    STRETCH_HORIZONTAL = 1 << 4,
    STRETCH_VERTICAL = 1 << 5,

    STRETCH = STRETCH_HORIZONTAL | STRETCH_VERTICAL,
};

static const ankerl::unordered_dense::map<XXH64_hash_t, uint32_t> g_flags =
{
    // ui_boss_gauge
    { HashStr("ui_boss_gauge/gauge_bg"), ALIGN_TOP_RIGHT },
    { HashStr("ui_boss_gauge/gauge_2"), ALIGN_TOP_RIGHT },
    { HashStr("ui_boss_gauge/gauge_1"), ALIGN_TOP_RIGHT },
    { HashStr("ui_boss_gauge/gauge_breakpoint"), ALIGN_TOP_RIGHT },

    // ui_exstage
    { HashStr("ui_exstage/shield/L_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_exstage/shield/L_gauge_effect"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_exstage/shield/L_gauge_effect_2"), ALIGN_BOTTOM_LEFT },   
    { HashStr("ui_exstage/energy/R_gauge"), ALIGN_BOTTOM_RIGHT },
    { HashStr("ui_exstage/energy/R_gauge_effect"), ALIGN_BOTTOM_RIGHT },
    { HashStr("ui_exstage/energy/R_gauge_effect_2"), ALIGN_BOTTOM_RIGHT }, 
    { HashStr("ui_exstage/hit/hit_counter_bg"), ALIGN_RIGHT },
    { HashStr("ui_exstage/hit/hit_counter_num"), ALIGN_RIGHT },

    // ui_general
    { HashStr("ui_general/bg"), STRETCH },

    // ui_loading
    { HashStr("ui_loading/bg_1"), STRETCH },
    { HashStr("ui_loading/bg_2"), STRETCH },

    // ui_missionscreen
    { HashStr("ui_missionscreen/player_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/time_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/score_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/item_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/laptime_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/lap_count"), ALIGN_TOP_RIGHT },

    // ui_misson
    { HashStr("ui_misson/bg"), STRETCH },

    // ui_playscreen
    { HashStr("ui_playscreen/player_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen/time_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen/score_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen/exp_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen/so_speed_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen/so_ringenagy_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen/gauge_frame"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen/ring_count"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen/ring_get"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen/add/speed_count"), ALIGN_RIGHT },
    { HashStr("ui_playscreen/add/u_info"), ALIGN_BOTTOM_RIGHT },
    { HashStr("ui_playscreen/add/medal_get_s"), ALIGN_BOTTOM_RIGHT },
    { HashStr("ui_playscreen/add/medal_get_m"), ALIGN_BOTTOM_RIGHT },

    // ui_playscreen_ev
    { HashStr("ui_playscreen_ev/player_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen_ev/score_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen_ev/ring_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen_ev/ring_get"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen_ev/exp_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_playscreen_ev/add/u_info"), ALIGN_BOTTOM_RIGHT },
    { HashStr("ui_playscreen_ev/add/medal_get_s"), ALIGN_BOTTOM_RIGHT },
    { HashStr("ui_playscreen_ev/add/medal_get_m"), ALIGN_BOTTOM_RIGHT },  
    { HashStr("ui_playscreen_ev/gauge/unleash_bg"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/life_bg"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/unleash_body"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/unleash_bar_1"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/unleash_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/unleash_gauge_effect_2"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/unleash_gauge_effect"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/unleash_bar_2"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/life"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield_position"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_01"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_02"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_03"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_04"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_05"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_06"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_07"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_08"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_09"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_10"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_11"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_12"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_13"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_14"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_15"), ALIGN_BOTTOM_LEFT },

    // ui_playscreen_ev_hit
    { HashStr("ui_playscreen_ev_hit/hit_counter_bg"), ALIGN_RIGHT },
    { HashStr("ui_playscreen_ev_hit/hit_counter_num"), ALIGN_RIGHT },
    { HashStr("ui_playscreen_ev_hit/hit_counter_txt_1"), ALIGN_RIGHT },
    { HashStr("ui_playscreen_ev_hit/hit_counter_txt_2"), ALIGN_RIGHT },
    { HashStr("ui_playscreen_ev_hit/chance_attack"), ALIGN_RIGHT },

    // ui_playscreen_su
    { HashStr("ui_playscreen_su/su_sonic_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_su/gaia_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_playscreen_su/footer"), ALIGN_BOTTOM_RIGHT },

    // ui_prov_playscreen
    { HashStr("ui_prov_playscreen/so_speed_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_prov_playscreen/so_ringenagy_gauge"), ALIGN_BOTTOM_LEFT },
    { HashStr("ui_prov_playscreen/bg"), ALIGN_TOP_LEFT },
    { HashStr("ui_prov_playscreen/info_1"), ALIGN_TOP_LEFT },
    { HashStr("ui_prov_playscreen/info_2"), ALIGN_TOP_LEFT },
    { HashStr("ui_prov_playscreen/ring_get_effect"), ALIGN_BOTTOM_LEFT },

    // ui_saveicon
    { HashStr("ui_saveicon/icon"), ALIGN_BOTTOM_RIGHT },

    // ui_title
    { HashStr("ui_title/bg/headr"), ALIGN_TOP | STRETCH_HORIZONTAL },
    { HashStr("ui_title/bg/footer"), ALIGN_BOTTOM | STRETCH_HORIZONTAL },
    { HashStr("ui_title/bg/position"), ALIGN_BOTTOM },

    // ui_townscreen
    { HashStr("ui_townscreen/time"), ALIGN_TOP_RIGHT },
    { HashStr("ui_townscreen/time_effect"), ALIGN_TOP_RIGHT },
    { HashStr("ui_townscreen/info"), ALIGN_TOP_LEFT },
    { HashStr("ui_townscreen/cam"), ALIGN_TOP_RIGHT },

    // ui_worldmap
    { HashStr("ui_worldmap/contents/info/bg/cts_info_bg"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/bg/info_bg_1"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_1"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_2"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_3"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_4"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/footer/worldmap_footer_bg"), ALIGN_BOTTOM },
    { HashStr("ui_worldmap/footer/worldmap_footer_img_A"), ALIGN_BOTTOM },
    { HashStr("ui_worldmap/header/worldmap_header_bg"), ALIGN_TOP },
    { HashStr("ui_worldmap/header/worldmap_header_img"), ALIGN_TOP_LEFT },
};

static std::optional<uint32_t> FindFlags(uint32_t data)
{
    XXH64_hash_t path;
    {
        std::lock_guard lock(g_pathMutex);

        auto findResult = g_paths.find(g_memory.Translate(data));
        if (findResult == g_paths.end())
            return {};

        path = findResult->second;
    }

    auto findResult = g_flags.find(path);
    if (findResult != g_flags.end())
        return findResult->second;

    return {};
}

static std::optional<uint32_t> g_sceneFlags;

void RenderCsdSceneMidAsmHook(PPCRegister& r30)
{
    g_sceneFlags = FindFlags(r30.u32);
}

static std::optional<uint32_t> g_castNodeFlags;

void RenderCsdCastNodeMidAsmHook(PPCRegister& r10, PPCRegister& r27)
{
    g_castNodeFlags = FindFlags(r10.u32 + r27.u32);
}

static std::optional<uint32_t> g_castFlags;

void RenderCsdCastMidAsmHook(PPCRegister& r4)
{
    g_castFlags = FindFlags(r4.u32);
}

static void Draw(PPCContext& ctx, uint8_t* base, PPCFunc* original, uint32_t stride)
{
    uint32_t flags = 0;

    if (g_castFlags.has_value())
    {
        flags = g_castFlags.value();
    }
    else if (g_castNodeFlags.has_value())
    {
        flags = g_castNodeFlags.value();
    }
    else if (g_sceneFlags.has_value())
    {
        flags = g_sceneFlags.value();
    }

    auto backBuffer = Video::GetBackBuffer();

    uint32_t size = ctx.r5.u32 * stride;
    ctx.r1.u32 -= size;

    uint8_t* stack = base + ctx.r1.u32;
    memcpy(stack, base + ctx.r4.u32, size);

    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if ((flags & STRETCH_HORIZONTAL) != 0)
    {
        scaleX = backBuffer->width / 1280.0f;
    }
    else
    {
        if ((flags & ALIGN_RIGHT) != 0)
            offsetX = g_offsetX * 2.0f;
        else if ((flags & ALIGN_LEFT) == 0)
            offsetX = g_offsetX;
    }

    if ((flags & STRETCH_VERTICAL) != 0)
    {
        scaleY = backBuffer->height / 720.0f;
    }
    else
    {
        if ((flags & ALIGN_BOTTOM) != 0)
            offsetY = g_offsetY * 2.0f;
        else if ((flags & ALIGN_TOP) == 0)
            offsetY = g_offsetY;
    }

    for (size_t i = 0; i < ctx.r5.u32; i++)
    {
        auto position = reinterpret_cast<be<float>*>(stack + i * stride);

        float x = offsetX + position[0] * scaleX;
        float y = offsetY + position[1] * scaleY;

        position[0] = round(x / backBuffer->width * GameWindow::s_width) / GameWindow::s_width * backBuffer->width;
        position[1] = round(y / backBuffer->height * GameWindow::s_height) / GameWindow::s_height * backBuffer->height;
    }

    ctx.r4.u32 = ctx.r1.u32;
    original(ctx, base);
    ctx.r1.u32 += size;
}

// SWA::CCsdPlatformMirage::Draw
PPC_FUNC_IMPL(__imp__sub_825E2E70);
PPC_FUNC(sub_825E2E70)
{
    Draw(ctx, base, __imp__sub_825E2E70, 0x14);
}

// SWA::CCsdPlatformMirage::DrawNoTex
PPC_FUNC_IMPL(__imp__sub_825E2E88);
PPC_FUNC(sub_825E2E88)
{
    Draw(ctx, base, __imp__sub_825E2E88, 0xC);
}

// Hedgehog::MirageDebug::SetScissorRect
PPC_FUNC_IMPL(__imp__sub_82E16C70);
PPC_FUNC(sub_82E16C70)
{
    auto backBuffer = Video::GetBackBuffer();
    auto scissorRect = reinterpret_cast<GuestRect*>(base + ctx.r4.u32);

    scissorRect->left = scissorRect->left + g_offsetX;
    scissorRect->top = scissorRect->top + g_offsetY;
    scissorRect->right = scissorRect->right + g_offsetX;
    scissorRect->bottom = scissorRect->bottom + g_offsetY;

    __imp__sub_82E16C70(ctx, base);
}

// Hedgehog::MirageDebug::CPrimitive2D::Draw
PPC_FUNC_IMPL(__imp__sub_830D1EF0);
PPC_FUNC(sub_830D1EF0)
{
    __imp__sub_830D1EF0(ctx, base);

    auto backBuffer = Video::GetBackBuffer();
    auto position = reinterpret_cast<be<float>*>(base + ctx.r4.u32);

    for (size_t i = 0; i < 4; i++)
    {
        position[0] = position[0] * 1280.0f / backBuffer->width;
        position[1] = position[1] * 720.0f / backBuffer->height;
        position += 7;
    }
}
