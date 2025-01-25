#include <user/config.h>
#include <api/SWA.h>
#include <app.h>
#include <ui/game_window.h>
#include <gpu/video.h>

#include "aspect_ratio_patches.h"
#include "camera_patches.h"

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

// Chao::CSD::CMemoryAlloc::Free
PPC_FUNC_IMPL(__imp__sub_825E2E60);
PPC_FUNC(sub_825E2E60)
{
    if (ctx.r4.u32 != NULL && PPC_LOAD_U32(ctx.r4.u32) == 0x4E594946 && PPC_LOAD_U32(ctx.r4.u32 + 0x20) == 0x6E43504A) // NYIF, nCPJ
    {
        uint32_t fileSize = PPC_LOAD_U32(ctx.r4.u32 + 0x14);

        std::lock_guard lock(g_pathMutex);
        const uint8_t* key = base + ctx.r4.u32;

        auto lower = g_paths.lower_bound(key);
        auto upper = g_paths.lower_bound(key + fileSize);

        g_paths.erase(lower, upper);
    }

    __imp__sub_825E2E60(ctx, base);
}

static float ComputeScale(float aspectRatio)
{
    return ((aspectRatio * 720.0f) / 1280.0f) / sqrt((aspectRatio * 720.0f) / 1280.0f);
}

void AspectRatioPatches::ComputeOffsets()
{
    float width = Video::s_viewportWidth;
    float height = Video::s_viewportHeight;

    g_aspectRatio = width / height;
    g_aspectRatioGameplayScale = 1.0f;

    if (g_aspectRatio >= NARROW_ASPECT_RATIO)
    {
        g_aspectRatioOffsetX = (width - height * WIDE_ASPECT_RATIO) / 2.0f;
        g_aspectRatioOffsetY = 0.0f;
        g_aspectRatioScale = height / 720.0f;

        // keep same scale above Steam Deck aspect ratio
        if (g_aspectRatio < WIDE_ASPECT_RATIO)
        {
            // interpolate to original 4:3 scale
            float steamDeckScale = g_aspectRatio / WIDE_ASPECT_RATIO;
            float narrowScale = ComputeScale(NARROW_ASPECT_RATIO);

            float lerpFactor = std::clamp((g_aspectRatio - NARROW_ASPECT_RATIO) / (STEAM_DECK_ASPECT_RATIO - NARROW_ASPECT_RATIO), 0.0f, 1.0f);
            g_aspectRatioGameplayScale = narrowScale + (steamDeckScale - narrowScale) * lerpFactor;
        }
    }
    else
    {
        // 4:3 crop
        g_aspectRatioOffsetX = (width - width * NARROW_ASPECT_RATIO) / 2.0f;
        g_aspectRatioOffsetY = (height - width / NARROW_ASPECT_RATIO) / 2.0f;
        g_aspectRatioScale = width / 960.0f;
        g_aspectRatioGameplayScale = ComputeScale(NARROW_ASPECT_RATIO);
    } 

    g_aspectRatioNarrowScale = std::clamp((g_aspectRatio - NARROW_ASPECT_RATIO) / (WIDE_ASPECT_RATIO - NARROW_ASPECT_RATIO), 0.0f, 1.0f);
}

static void GetViewport(void* application, be<uint32_t>* width, be<uint32_t>* height)
{
    *width = 1280;
    *height = 720;
}

GUEST_FUNCTION_HOOK(sub_82E169B8, GetViewport);

// SWA::CGameDocument::ComputeScreenPosition
PPC_FUNC_IMPL(__imp__sub_8250FC70);
PPC_FUNC(sub_8250FC70)
{
    __imp__sub_8250FC70(ctx, base);

    auto position = reinterpret_cast<be<float>*>(base + ctx.r3.u32);

    position[0] = (position[0] / 1280.0f * Video::s_viewportWidth - g_aspectRatioOffsetX) / g_aspectRatioScale;
    position[1] = (position[1] / 720.0f * Video::s_viewportHeight - g_aspectRatioOffsetY) / g_aspectRatioScale;
}

void ComputeScreenPositionMidAsmHook(PPCRegister& f1, PPCRegister& f2)
{
    f1.f64 = (f1.f64 / 1280.0 * Video::s_viewportWidth - g_aspectRatioOffsetX) / g_aspectRatioScale;
    f2.f64 = (f2.f64 / 720.0 * Video::s_viewportHeight - g_aspectRatioOffsetY) / g_aspectRatioScale;
}

void WorldMapInfoMidAsmHook(PPCRegister& r4)
{
    // Prevent the game from snapping "cts_parts_sun_moon"
    // to "cts_guide_icon" automatically, we will do this ourselves.
    r4.u32 = 0x8200A621;
}

// SWA::CTitleStateWorldMap::Update
PPC_FUNC_IMPL(__imp__sub_8258B558);
PPC_FUNC(sub_8258B558)
{
    auto r3 = ctx.r3;
    __imp__sub_8258B558(ctx, base);

    uint32_t worldMapSimpleInfo = PPC_LOAD_U32(r3.u32 + 0x70);
    if (worldMapSimpleInfo != NULL)
    {
        auto setPosition = [&](uint32_t rcPtr, float offsetX = 0.0f, float offsetY = 0.0f)
            {
                uint32_t scene = PPC_LOAD_U32(rcPtr + 0x4);
                if (scene != NULL)
                {
                    scene = PPC_LOAD_U32(scene + 0x4);
                    if (scene != NULL)
                    {
                        ctx.r3.u32 = scene;
                        ctx.f1.f64 = offsetX + g_aspectRatioNarrowScale * 140.0f;
                        ctx.f2.f64 = offsetY;

                        if (Config::UIScaleMode == EUIScaleMode::Edge && g_aspectRatioNarrowScale >= 1.0f)
                            ctx.f1.f64 += g_aspectRatioOffsetX / g_aspectRatioScale;

                        sub_830BB3D0(ctx, base);
                    }
                }
            };

        setPosition(worldMapSimpleInfo + 0x2C, 299.0f, -178.0f);
        setPosition(worldMapSimpleInfo + 0x34);
        setPosition(worldMapSimpleInfo + 0x4C);

        for (uint32_t it = PPC_LOAD_U32(worldMapSimpleInfo + 0x20); it != PPC_LOAD_U32(worldMapSimpleInfo + 0x24); it += 8)
            setPosition(it);

        uint32_t menuTextBox = PPC_LOAD_U32(worldMapSimpleInfo + 0x5C);
        if (menuTextBox != NULL)
        {
            uint32_t textBox = PPC_LOAD_U32(menuTextBox + 0x4);
            if (textBox != NULL)
            {
                float value = 708.0f + g_aspectRatioNarrowScale * 140.0f;
                if (Config::UIScaleMode == EUIScaleMode::Edge && g_aspectRatioNarrowScale >= 1.0f)
                    value += g_aspectRatioOffsetX / g_aspectRatioScale;

                PPC_STORE_U32(textBox + 0x38, reinterpret_cast<uint32_t&>(value));
            }
        }
    }
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

    SCALE = 1 << 6,

    WORLD_MAP = 1 << 7,

    EXTEND_LEFT = 1 << 8,
    EXTEND_RIGHT = 1 << 9,

    STORE_LEFT_CORNER = 1 << 10,
    STORE_RIGHT_CORNER = 1 << 11,

    SKIP = 1 << 12,

    OFFSET_SCALE_LEFT = 1 << 13,
    OFFSET_SCALE_RIGHT = 1 << 14,
};

struct CsdModifier
{
    uint32_t flags{};
    float cornerMax{};
    uint32_t cornerIndex{};
};

static const ankerl::unordered_dense::map<XXH64_hash_t, CsdModifier> g_modifiers =
{
    // ui_balloon
    { HashStr("ui_balloon/window/bg"), { STRETCH } },
    { HashStr("ui_balloon/window/footer"), { ALIGN_BOTTOM } },

    // ui_boss_gauge
    { HashStr("ui_boss_gauge/gauge_bg"), { ALIGN_TOP_RIGHT | SCALE } },
    { HashStr("ui_boss_gauge/gauge_2"), { ALIGN_TOP_RIGHT | SCALE } },
    { HashStr("ui_boss_gauge/gauge_1"), { ALIGN_TOP_RIGHT | SCALE } },
    { HashStr("ui_boss_gauge/gauge_breakpoint"), { ALIGN_TOP_RIGHT | SCALE } },

    // ui_exstage
    { HashStr("ui_exstage/shield/L_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_exstage/shield/L_gauge_effect"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_exstage/shield/L_gauge_effect_2"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_exstage/energy/R_gauge"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_exstage/energy/R_gauge_effect"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_exstage/energy/R_gauge_effect_2"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_exstage/hit/hit_counter_bg"), { ALIGN_RIGHT | SCALE } },
    { HashStr("ui_exstage/hit/hit_counter_num"), { ALIGN_RIGHT | SCALE } },

    // ui_gate
    { HashStr("ui_gate/footer/status_footer"), { ALIGN_BOTTOM } },
    { HashStr("ui_gate/header/status_title"), { ALIGN_TOP | OFFSET_SCALE_LEFT, 652.0f } },
    { HashStr("ui_gate/header/status_title/title_bg/center"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_gate/header/status_title/title_bg/center/h_light"), { ALIGN_TOP | EXTEND_LEFT} },
    { HashStr("ui_gate/header/status_title/title_bg/right"), { ALIGN_TOP | STORE_RIGHT_CORNER } },
    { HashStr("ui_gate/window/window_bg"), { STRETCH } },

    // ui_general
    { HashStr("ui_general/bg"), { STRETCH } },
    { HashStr("ui_general/footer"), { ALIGN_BOTTOM } },

    // ui_itemresult
    { HashStr("ui_itemresult/footer/result_footer"), { ALIGN_BOTTOM } },
    { HashStr("ui_itemresult/main/iresult_title"), { ALIGN_TOP } },
    { HashStr("ui_itemresult/main/iresult_title"), { ALIGN_TOP | OFFSET_SCALE_LEFT, 688.0f } },
    { HashStr("ui_itemresult/main/iresult_title/title_bg/center"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_itemresult/main/iresult_title/title_bg/center/h_light"), { ALIGN_TOP | EXTEND_LEFT} },
    { HashStr("ui_itemresult/main/iresult_title/title_bg/right"), { ALIGN_TOP | STORE_RIGHT_CORNER } },

    // ui_loading
    { HashStr("ui_loading/bg_1"), { STRETCH } },
    { HashStr("ui_loading/bg_2"), { STRETCH } },

    // ui_mediaroom
    { HashStr("ui_mediaroom/header/bg/img_1"), { EXTEND_LEFT } },
    { HashStr("ui_mediaroom/header/bg/img_10"), { EXTEND_RIGHT } },  
    { HashStr("ui_mediaroom/header/frame/img_1"), { EXTEND_LEFT } },
    { HashStr("ui_mediaroom/header/frame/img_5"), { EXTEND_RIGHT } },

    // ui_missionscreen
    { HashStr("ui_missionscreen/player_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_missionscreen/time_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_missionscreen/score_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_missionscreen/item_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_missionscreen/laptime_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_missionscreen/lap_count"), { ALIGN_TOP_RIGHT | SCALE } },

    // ui_misson
    { HashStr("ui_misson/bg"), { STRETCH } },
    { HashStr("ui_misson/footer/footer_B"), { ALIGN_BOTTOM } },
    { HashStr("ui_misson/header/misson_title_B"), { ALIGN_TOP | OFFSET_SCALE_LEFT, 638.0f } },
    { HashStr("ui_misson/header/misson_title_B/title_bg/center"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_misson/header/misson_title_B/title_bg/center/h_light"), { ALIGN_TOP | EXTEND_LEFT} },
    { HashStr("ui_misson/header/misson_title_B/title_bg/right"), { ALIGN_TOP | STORE_RIGHT_CORNER } },
    { HashStr("ui_misson/window/bg_B2/position/bg"), { STRETCH } },

    // ui_pause
    { HashStr("ui_pause/bg"), { STRETCH } },
    { HashStr("ui_pause/footer/footer_A"), { ALIGN_BOTTOM } },
    { HashStr("ui_pause/footer/footer_B"), { ALIGN_BOTTOM } },
    { HashStr("ui_pause/header/status_title"), { ALIGN_TOP | OFFSET_SCALE_LEFT, 585.0f } },
    { HashStr("ui_pause/header/status_title/title_bg/center"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_pause/header/status_title/title_bg/center/h_light"), { ALIGN_TOP | EXTEND_LEFT} },
    { HashStr("ui_pause/header/status_title/title_bg/right"), { ALIGN_TOP | STORE_RIGHT_CORNER } },

    // ui_playscreen
    { HashStr("ui_playscreen/player_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen/time_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen/score_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen/exp_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen/so_speed_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen/so_ringenagy_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen/gauge_frame"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen/ring_count"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen/ring_get"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen/add/speed_count"), { ALIGN_RIGHT | SCALE } },
    { HashStr("ui_playscreen/add/u_info"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_playscreen/add/medal_get_s"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_playscreen/add/medal_get_m"), { ALIGN_BOTTOM_RIGHT | SCALE } },

    // ui_playscreen_ev
    { HashStr("ui_playscreen_ev/player_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/score_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/ring_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/ring_get"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/exp_count"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/add/u_info"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_playscreen_ev/add/medal_get_s"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_playscreen_ev/add/medal_get_m"), { ALIGN_BOTTOM_RIGHT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/unleash_bg"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/life_bg"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/unleash_body"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/unleash_bar_1"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/unleash_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/unleash_gauge_effect_2"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/unleash_gauge_effect"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/unleash_bar_2"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/life"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield_position"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_01"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_02"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_03"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_04"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_05"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_06"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_07"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_08"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_09"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_10"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_11"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_12"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_13"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_14"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_ev/gauge/shield/shield_15"), { ALIGN_BOTTOM_LEFT | SCALE } },

    // ui_playscreen_ev_hit
    { HashStr("ui_playscreen_ev_hit/hit_counter_bg"), { ALIGN_RIGHT | SCALE } },
    { HashStr("ui_playscreen_ev_hit/hit_counter_num"), { ALIGN_RIGHT | SCALE } },
    { HashStr("ui_playscreen_ev_hit/hit_counter_txt_1"), { ALIGN_RIGHT | SCALE } },
    { HashStr("ui_playscreen_ev_hit/hit_counter_txt_2"), { ALIGN_RIGHT | SCALE } },
    { HashStr("ui_playscreen_ev_hit/chance_attack"), { ALIGN_RIGHT | SCALE } },

    // ui_playscreen_su
    { HashStr("ui_playscreen_su/su_sonic_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_su/gaia_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_playscreen_su/footer"), { ALIGN_BOTTOM_RIGHT | SCALE } },

    // ui_prov_playscreen
    { HashStr("ui_prov_playscreen/so_speed_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_prov_playscreen/so_ringenagy_gauge"), { ALIGN_BOTTOM_LEFT | SCALE } },
    { HashStr("ui_prov_playscreen/bg"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_prov_playscreen/info_1"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_prov_playscreen/info_2"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_prov_playscreen/ring_get_effect"), { ALIGN_BOTTOM_LEFT | SCALE } },

    // ui_result
    { HashStr("ui_result/footer/result_footer"), { ALIGN_BOTTOM } },
    { HashStr("ui_result/main/result_title"), { ALIGN_TOP | OFFSET_SCALE_LEFT, 688.0f } },
    { HashStr("ui_result/main/result_title/title_bg/center"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_result/main/result_title/title_bg/center/h_light"), { ALIGN_TOP | EXTEND_LEFT} },
    { HashStr("ui_result/main/result_title/title_bg/right"), { ALIGN_TOP | STORE_RIGHT_CORNER } },
    { HashStr("ui_result/main/result_num_1"), { OFFSET_SCALE_RIGHT, 669.0f } },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1/left"), { STORE_LEFT_CORNER } },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1/right"), { SKIP } },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1/right/h_light"), { SKIP } },  
    { HashStr("ui_result/main/result_num_2"), { OFFSET_SCALE_RIGHT, 669.0f } },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1/left"), { STORE_LEFT_CORNER } },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1/right"), { SKIP } },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1/right/h_light"), { SKIP } },   
    { HashStr("ui_result/main/result_num_3"), { OFFSET_SCALE_RIGHT, 669.0f } },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1/left"), { STORE_LEFT_CORNER } },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1/right"), { SKIP } },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1/right/h_light"), { SKIP } },  
    { HashStr("ui_result/main/result_num_4"), { OFFSET_SCALE_RIGHT, 669.0f } },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1/left"), { STORE_LEFT_CORNER } },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1/right"), { SKIP } },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1/right/h_light"), { SKIP } },   
    { HashStr("ui_result/main/result_num_5"), { OFFSET_SCALE_RIGHT, 669.0f } },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1/left"), { STORE_LEFT_CORNER } },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1/right"), { SKIP } },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1/right/h_light"), { SKIP } }, 
    { HashStr("ui_result/main/result_num_6"), { OFFSET_SCALE_LEFT, 1094.0f } },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center"), { EXTEND_LEFT } },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center/h_light"), { EXTEND_LEFT } },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center/right"), { STORE_RIGHT_CORNER } },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center/left"), { SKIP } },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center/left/h_light"), { SKIP } },
    { HashStr("ui_result/newRecode/result_newR/position/newR_brilliance1"), { OFFSET_SCALE_RIGHT | STORE_LEFT_CORNER, 458.4f } },
    { HashStr("ui_result/newRecode/result_newR/position/newR_brilliance2"), { OFFSET_SCALE_RIGHT | STORE_LEFT_CORNER, 458.4f } },
    { HashStr("ui_result/newRecode/result_newR/position/newR_brilliance3"), { OFFSET_SCALE_RIGHT | STORE_LEFT_CORNER, 458.4f } },

    // ui_result_ex
    { HashStr("ui_result_ex/footer/result_footer"), { ALIGN_BOTTOM } },
    { HashStr("ui_result_ex/main/result_title"), { ALIGN_TOP | OFFSET_SCALE_LEFT, 688.0f } },
    { HashStr("ui_result_ex/main/result_title/title_bg/center"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_result_ex/main/result_title/title_bg/center/h_light"), { ALIGN_TOP | EXTEND_LEFT} },
    { HashStr("ui_result_ex/main/result_title/title_bg/right"), { ALIGN_TOP | STORE_RIGHT_CORNER } },
    { HashStr("ui_result_ex/main/number/result_num_1"), { OFFSET_SCALE_LEFT | OFFSET_SCALE_RIGHT, 669.0f } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1"), { OFFSET_SCALE_RIGHT, 669.0f, 0 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1/left"), { STORE_LEFT_CORNER, 0, 0 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1/right"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1/right/h_light"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_2"), { OFFSET_SCALE_RIGHT, 669.0f, 1 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_2/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_2/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_2/center_1/left"), { STORE_LEFT_CORNER, 0, 1 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_2/center_1/right"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_2/center_1/right/h_light"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_3"), { OFFSET_SCALE_RIGHT, 669.0f, 2 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_3/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_3/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_3/center_1/left"), { STORE_LEFT_CORNER, 0, 2 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_3/center_1/right"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_3/center_1/right/h_light"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_4"), { OFFSET_SCALE_RIGHT, 669.0f, 3 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_4/center_1"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_4/center_1/h_light"), { EXTEND_RIGHT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_4/center_1/left"), { STORE_LEFT_CORNER, 0, 3 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_4/center_1/right"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_4/center_1/right/h_light"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_6"), { OFFSET_SCALE_LEFT, 1094.0f, 4 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_6/center"), { EXTEND_LEFT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_6/center/right"), { STORE_RIGHT_CORNER, 0, 4 } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_6/center/h_light"), { EXTEND_LEFT } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_6/center/left"), { SKIP } },
    { HashStr("ui_result_ex/main/number/result_num_1/position_6/center/left/h_light"), { SKIP } },
    { HashStr("ui_result_ex/newRecode/result_newR/position/newR_brilliance1"), { OFFSET_SCALE_RIGHT | STORE_LEFT_CORNER, 458.4f } },
    { HashStr("ui_result_ex/newRecode/result_newR/position/newR_brilliance2"), { OFFSET_SCALE_RIGHT | STORE_LEFT_CORNER, 458.4f } },
    { HashStr("ui_result_ex/newRecode/result_newR/position/newR_brilliance3"), { OFFSET_SCALE_RIGHT | STORE_LEFT_CORNER, 458.4f } },

    // ui_shop
    { HashStr("ui_shop/footer/shop_footer"), { ALIGN_BOTTOM } },
    { HashStr("ui_shop/header/ring"), { ALIGN_TOP } },
    { HashStr("ui_shop/header/shop_nametag"), { ALIGN_TOP } },

    // ui_start
    { HashStr("ui_start/Clear/position/bg/bg_1"), { STRETCH } },
    { HashStr("ui_start/Clear/position/bg/bg_2"), { STRETCH } },
    { HashStr("ui_start/Start/img/bg/bg_1"), { STRETCH } },
    { HashStr("ui_start/Start/img/bg/bg_2"), { STRETCH } },

    // ui_status
    { HashStr("ui_status/footer/status_footer"), { ALIGN_BOTTOM } },
    { HashStr("ui_status/header/status_title"), { ALIGN_TOP | OFFSET_SCALE_LEFT, 617.0f } },
    { HashStr("ui_status/header/status_title/title_bg/center"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_status/header/status_title/title_bg/center/h_light"), { ALIGN_TOP | EXTEND_LEFT } },
    { HashStr("ui_status/header/status_title/title_bg/right"), { ALIGN_TOP | STORE_RIGHT_CORNER } },
    { HashStr("ui_status/logo/logo/bg_position/c_1"), { STRETCH_HORIZONTAL } },
    { HashStr("ui_status/logo/logo/bg_position/c_2"), { STRETCH_HORIZONTAL } },
    { HashStr("ui_status/main/arrow_effect/a_efc_1"), { OFFSET_SCALE_LEFT, 866.0f } },
    { HashStr("ui_status/main/arrow_effect/a_efc_1/position/img_01"), { STORE_RIGHT_CORNER } },
    { HashStr("ui_status/main/progless/bg/prgs_bg_1"), { OFFSET_SCALE_LEFT, 714.0f } },
    { HashStr("ui_status/main/progless/bg/prgs_bg_1/position/center/right"), { STORE_RIGHT_CORNER } },
    { HashStr("ui_status/main/progless/prgs/prgs_bar_1"), { OFFSET_SCALE_LEFT, 586.0f } },
    { HashStr("ui_status/main/progless/prgs/prgs_bar_1/position/bg/center/right"), { STORE_RIGHT_CORNER } },
    { HashStr("ui_status/main/tag/bg/tag_bg_1"), { OFFSET_SCALE_LEFT, 413.0f } },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center"), { EXTEND_LEFT } },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center/h_light"), { EXTEND_LEFT } },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center/right"), { STORE_RIGHT_CORNER } },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center/left"), { SKIP } },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center/left/h_light"), { SKIP } },
    { HashStr("ui_status/main/tag/txt/tag_txt_1"), { OFFSET_SCALE_LEFT, 352.0f } },
    { HashStr("ui_status/main/tag/txt/tag_txt_1/position/img"), { STORE_RIGHT_CORNER } },
    { HashStr("ui_status/window/bg"), { STRETCH } },

    // ui_title
    { HashStr("ui_title/bg/bg"), { STRETCH } },
    { HashStr("ui_title/bg/headr"), { ALIGN_TOP | STRETCH_HORIZONTAL } },
    { HashStr("ui_title/bg/footer"), { ALIGN_BOTTOM | STRETCH_HORIZONTAL } },
    { HashStr("ui_title/bg/position"), { ALIGN_BOTTOM } },

    // ui_townscreen
    { HashStr("ui_townscreen/time"), { ALIGN_TOP_RIGHT | SCALE } },
    { HashStr("ui_townscreen/time_effect"), { ALIGN_TOP_RIGHT | SCALE } },
    { HashStr("ui_townscreen/info"), { ALIGN_TOP_LEFT | SCALE } },
    { HashStr("ui_townscreen/cam"), { ALIGN_TOP_RIGHT | SCALE } },
    { HashStr("ui_townscreen/footer"), { ALIGN_BOTTOM } },

    // ui_worldmap
    { HashStr("ui_worldmap/contents/choices/cts_choices_bg"), { STRETCH } },
    { HashStr("ui_worldmap/contents/info/bg/cts_info_bg"), { ALIGN_TOP_LEFT | WORLD_MAP } },
    { HashStr("ui_worldmap/contents/info/bg/info_bg_1"), { ALIGN_TOP_LEFT | WORLD_MAP } },
    { HashStr("ui_worldmap/contents/info/img/info_img_1"), { ALIGN_TOP_LEFT | WORLD_MAP } },
    { HashStr("ui_worldmap/contents/info/img/info_img_2"), { ALIGN_TOP_LEFT | WORLD_MAP } },
    { HashStr("ui_worldmap/contents/info/img/info_img_3"), { ALIGN_TOP_LEFT | WORLD_MAP } },
    { HashStr("ui_worldmap/contents/info/img/info_img_4"), { ALIGN_TOP_LEFT | WORLD_MAP } },
    { HashStr("ui_worldmap/footer/worldmap_footer_bg"), { ALIGN_BOTTOM } },
    { HashStr("ui_worldmap/footer/worldmap_footer_img_A"), { ALIGN_BOTTOM } },
    { HashStr("ui_worldmap/header/worldmap_header_bg"), { ALIGN_TOP } },
    { HashStr("ui_worldmap/header/worldmap_header_img"), { ALIGN_TOP_LEFT | WORLD_MAP } },

    // ui_worldmap_help
    { HashStr("ui_worldmap_help/balloon/help_window/position/msg_bg_l"), { EXTEND_LEFT } },
    { HashStr("ui_worldmap_help/balloon/help_window/position/msg_bg_r"), { EXTEND_RIGHT } },
};

static std::optional<CsdModifier> FindModifier(uint32_t data)
{
    XXH64_hash_t path;
    {
        std::lock_guard lock(g_pathMutex);

        auto findResult = g_paths.find(g_memory.Translate(data));
        if (findResult == g_paths.end())
            return {};

        path = findResult->second;
    }

    auto findResult = g_modifiers.find(path);
    if (findResult != g_modifiers.end())
        return findResult->second;

    return {};
}

static std::optional<CsdModifier> g_sceneModifier;
static float g_corners[8];
static bool g_cornerExtract;

//#define CORNER_DEBUG

// Explicit translations don't get affected by gameplay UI downscaling.
// This is required for the medal info in pause menu.
static float g_scenePositionX;
static float g_scenePositionY;

// Chao::CSD::CScene::Render
PPC_FUNC_IMPL(__imp__sub_830BC640);
PPC_FUNC(sub_830BC640)
{
    g_scenePositionX = 0.0f;
    g_scenePositionY = 0.0f;

    uint32_t motionPattern = PPC_LOAD_U32(ctx.r3.u32 + 0x98);
    if (motionPattern != NULL)
    {
        uint32_t member = PPC_LOAD_U32(motionPattern + 0xC);
        if (member != NULL)
        {
            uint32_t x = PPC_LOAD_U32(member + 0x2C);
            uint32_t y = PPC_LOAD_U32(member + 0x30);

            g_scenePositionX = 1280.0f * reinterpret_cast<float&>(x);
            g_scenePositionY = 720.0f * reinterpret_cast<float&>(y);
        }
    }

    __imp__sub_830BC640(ctx, base);
}

// Chao::CSD::Scene::Render
PPC_FUNC_IMPL(__imp__sub_830C6A00);
PPC_FUNC(sub_830C6A00)
{
    g_sceneModifier = FindModifier(ctx.r3.u32);

    if (g_sceneModifier.has_value() && (g_sceneModifier->flags & (OFFSET_SCALE_LEFT | OFFSET_SCALE_RIGHT)) != 0)
    {
        auto r3 = ctx.r3;
        auto r4 = ctx.r4;
        auto r5 = ctx.r5;
        auto r6 = ctx.r6;

        // Queue draw calls, but don't actually draw anything. We just want to extract the corner.
        g_cornerExtract = true;
        __imp__sub_830C6A00(ctx, base);
        g_cornerExtract = false;

#ifdef CORNER_DEBUG
        if (g_sceneModifier->cornerMax == FLT_MAX)
        {
            fmt::print("Corners: ");
            for (auto corner : g_corners)
                fmt::print("{} ", corner);

            fmt::println("");
        }
#endif

        ctx.r3 = r3;
        ctx.r4 = r4;
        ctx.r5 = r5;
        ctx.r6 = r6;
    }

    __imp__sub_830C6A00(ctx, base);
}

static std::optional<CsdModifier> g_castNodeModifier;

void RenderCsdCastNodeMidAsmHook(PPCRegister& r10, PPCRegister& r27)
{
    g_castNodeModifier = FindModifier(r10.u32 + r27.u32);
}

static std::optional<CsdModifier> g_castModifier;

void RenderCsdCastMidAsmHook(PPCRegister& r4)
{
    g_castModifier = FindModifier(r4.u32);
}

static void Draw(PPCContext& ctx, uint8_t* base, PPCFunc* original, uint32_t stride)
{
    CsdModifier modifier{};

    if (g_castModifier.has_value())
    {
        modifier = g_castModifier.value();
    }
    else if (g_castNodeModifier.has_value())
    {
        modifier = g_castNodeModifier.value();
    }
    else if (g_sceneModifier.has_value())
    {
        modifier = g_sceneModifier.value();
    }

    if ((modifier.flags & SKIP) != 0)
    {
        return;
    }

    if (g_cornerExtract)
    {
        if ((modifier.flags & (STORE_LEFT_CORNER | STORE_RIGHT_CORNER)) != 0)
        {
            uint32_t vertexIndex = ((modifier.flags & STORE_LEFT_CORNER) != 0) ? 0 : 3;
            g_corners[modifier.cornerIndex] = *reinterpret_cast<be<float>*>(base + ctx.r4.u32 + vertexIndex * stride);
        }

        return;
    }

    if (Config::UIScaleMode == EUIScaleMode::Centre)
    {
        if (g_aspectRatio >= WIDE_ASPECT_RATIO)
            modifier.flags &= ~(ALIGN_LEFT | ALIGN_RIGHT);
    }

    uint32_t size = ctx.r5.u32 * stride;
    ctx.r1.u32 -= size;

    uint8_t* stack = base + ctx.r1.u32;
    memcpy(stack, base + ctx.r4.u32, size);

    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float pivotX = 0.0f;
    float pivotY = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if ((modifier.flags & STRETCH_HORIZONTAL) != 0)
    {
        scaleX = Video::s_viewportWidth / 1280.0f;
    }
    else
    {
        scaleX = g_aspectRatioScale;

        if ((modifier.flags & ALIGN_RIGHT) != 0)
            offsetX = g_aspectRatioOffsetX * 2.0f;
        else if ((modifier.flags & ALIGN_LEFT) == 0)
            offsetX = g_aspectRatioOffsetX;

        if ((modifier.flags & SCALE) != 0)
        {
            scaleX *= g_aspectRatioGameplayScale;
            pivotX = g_scenePositionX;

            if ((modifier.flags & ALIGN_RIGHT) != 0)
                offsetX += 1280.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;
            else if ((modifier.flags & ALIGN_LEFT) == 0)
                offsetX += 640.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;

            offsetX += pivotX * g_aspectRatioScale;
        }

        if ((modifier.flags & WORLD_MAP) != 0)
        {
            if ((modifier.flags & ALIGN_LEFT) != 0)
                offsetX += (1.0f - g_aspectRatioNarrowScale) * g_aspectRatioScale * -20.0f;
        }
    }

    if ((modifier.flags & STRETCH_VERTICAL) != 0)
    {
        scaleY = Video::s_viewportHeight / 720.0f;
    }
    else
    {
        scaleY = g_aspectRatioScale;

        if ((modifier.flags & ALIGN_BOTTOM) != 0)
            offsetY = g_aspectRatioOffsetY * 2.0f;
        else if ((modifier.flags & ALIGN_TOP) == 0)
            offsetY = g_aspectRatioOffsetY;

        if ((modifier.flags & SCALE) != 0)
        {
            scaleY *= g_aspectRatioGameplayScale;
            pivotY = g_scenePositionY;

            if ((modifier.flags & ALIGN_BOTTOM) != 0)
                offsetY += 720.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;
            else if ((modifier.flags & ALIGN_TOP) == 0)
                offsetY += 360.0f * (1.0f - g_aspectRatioGameplayScale) * g_aspectRatioScale;

            offsetY += pivotY * g_aspectRatioScale;
        }
    }

    if (g_aspectRatio > WIDE_ASPECT_RATIO)
    {
        CsdModifier offsetScaleModifier{};
        float corner = 0.0f;

        if (g_castModifier.has_value())
        {
            offsetScaleModifier = g_castModifier.value();

            uint32_t vertexIndex = ((offsetScaleModifier.flags & STORE_LEFT_CORNER) != 0) ? 0 : 3;
            corner = *reinterpret_cast<be<float>*>(base + ctx.r4.u32 + vertexIndex * stride);
        }

        if (offsetScaleModifier.cornerMax == 0.0f && g_castNodeModifier.has_value())
        {
            offsetScaleModifier = g_castNodeModifier.value();
            corner = g_corners[offsetScaleModifier.cornerIndex];
        }

        if (offsetScaleModifier.cornerMax == 0.0f && g_sceneModifier.has_value())
        {
            offsetScaleModifier = g_sceneModifier.value();
            corner = g_corners[offsetScaleModifier.cornerIndex];
        }

#ifdef CORNER_DEBUG
        if ((offsetScaleModifier.flags & (OFFSET_SCALE_LEFT | OFFSET_SCALE_RIGHT)) != 0 && offsetScaleModifier.cornerMax == FLT_MAX)
            fmt::println("Corner: {}", corner);
#endif

        if ((offsetScaleModifier.flags & OFFSET_SCALE_LEFT) != 0)
            offsetX *= corner / offsetScaleModifier.cornerMax;
        else if ((offsetScaleModifier.flags & OFFSET_SCALE_RIGHT) != 0)
            offsetX = Video::s_viewportWidth - (Video::s_viewportWidth - offsetX) * (1280.0f - corner) / (1280.0f - offsetScaleModifier.cornerMax);
    }

    for (size_t i = 0; i < ctx.r5.u32; i++)
    {
        auto position = reinterpret_cast<be<float>*>(stack + i * stride);

        float x = offsetX + (position[0] - pivotX) * scaleX;
        float y = offsetY + (position[1] - pivotY) * scaleY;

        if ((modifier.flags & EXTEND_LEFT) != 0 && (i == 0 || i == 1))
        {
            x = std::min(x, 0.0f);
        }
        else if ((modifier.flags & EXTEND_RIGHT) != 0 && (i == 2 || i == 3))
        {
            x = std::max(x, float(Video::s_viewportWidth));
        }

        position[0] = round(x);
        position[1] = round(y);
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
    auto scissorRect = reinterpret_cast<GuestRect*>(base + ctx.r4.u32);

    scissorRect->left = scissorRect->left * g_aspectRatioScale + g_aspectRatioOffsetX;
    scissorRect->top = scissorRect->top * g_aspectRatioScale + g_aspectRatioOffsetY;
    scissorRect->right = scissorRect->right * g_aspectRatioScale + g_aspectRatioOffsetX;
    scissorRect->bottom = scissorRect->bottom * g_aspectRatioScale + g_aspectRatioOffsetY;

    __imp__sub_82E16C70(ctx, base);
}

// Store whether the primitive should be stretched in available padding space.
static constexpr size_t PRIMITIVE_2D_PADDING_OFFSET = 0x29;
static constexpr size_t PRIMITIVE_2D_PADDING_SIZE = 0x3;

// Hedgehog::MirageDebug::CPrimitive2D::CPrimitive2D
PPC_FUNC_IMPL(__imp__sub_822D0328);
PPC_FUNC(sub_822D0328)
{
    memset(base + ctx.r3.u32 + PRIMITIVE_2D_PADDING_OFFSET, 0, PRIMITIVE_2D_PADDING_SIZE);
    __imp__sub_822D0328(ctx, base);
}

// Hedgehog::MirageDebug::CPrimitive2D::CPrimitive2D(const Hedgehog::MirageDebug::CPrimitive2D&)
PPC_FUNC_IMPL(__imp__sub_830D2328);
PPC_FUNC(sub_830D2328)
{
    memcpy(base + ctx.r3.u32 + PRIMITIVE_2D_PADDING_OFFSET, base + ctx.r4.u32 + PRIMITIVE_2D_PADDING_OFFSET, PRIMITIVE_2D_PADDING_SIZE);
    __imp__sub_830D2328(ctx, base);
}

void AddPrimitive2DMidAsmHook(PPCRegister& r3)
{
    *(g_memory.base + r3.u32 + PRIMITIVE_2D_PADDING_OFFSET) = 0x01;
}

// Hedgehog::MirageDebug::CPrimitive2D::Draw
PPC_FUNC_IMPL(__imp__sub_830D1EF0);
PPC_FUNC(sub_830D1EF0)
{
    auto r3 = ctx.r3;

    __imp__sub_830D1EF0(ctx, base);

    struct Vertex
    {
        be<float> x;
        be<float> y;
        be<float> z;
        be<float> w;
        be<uint32_t> color;
        be<float> u;
        be<float> v;
    };

    auto vertex = reinterpret_cast<Vertex*>(base + ctx.r4.u32);

    for (size_t i = 0; i < 4; i++)
    {
        float x = vertex[i].x * 640.0f + 640.0f;
        float y = vertex[i].y * -360.0f + 360.0f;

        if (PPC_LOAD_U8(r3.u32 + PRIMITIVE_2D_PADDING_OFFSET))
        {
            // Stretch
            x = ((x + 0.5f) / 1280.0f) * Video::s_viewportWidth;
            y = ((y + 0.5f) / 720.0f) * Video::s_viewportHeight;
        }
        else
        {
            // Center
            x = g_aspectRatioOffsetX + (x + 0.5f) * g_aspectRatioScale;
            y = g_aspectRatioOffsetY + (y + 0.5f) * g_aspectRatioScale;
        }

        vertex[i].x = ((x - 0.5f) / Video::s_viewportWidth) * 2.0f - 1.0f;
        vertex[i].y = ((y - 0.5f) / Video::s_viewportHeight) * -2.0f + 1.0f;
    }

    bool letterboxTop = PPC_LOAD_U8(r3.u32 + PRIMITIVE_2D_PADDING_OFFSET + 0x1);
    bool letterboxBottom = PPC_LOAD_U8(r3.u32 + PRIMITIVE_2D_PADDING_OFFSET + 0x2);

    if (letterboxTop || letterboxBottom)
    {
        float halfPixelX = 1.0f / Video::s_viewportWidth;
        float halfPixelY = 1.0f / Video::s_viewportHeight;

        if (letterboxTop)
        {
            vertex[0].x = -1.0f - halfPixelX;
            vertex[0].y = 1.0f + halfPixelY;

            vertex[1].x = 1.0f - halfPixelX;
            vertex[1].y = 1.0f + halfPixelY;

            vertex[2].x = -1.0f - halfPixelX;
            // vertex[2].y untouched

            vertex[3].x = 1.0f - halfPixelX;
            // vertex[3].y untouched
        }
        else if (letterboxBottom)
        {
            vertex[0].x = -1.0f - halfPixelX;
            // vertex[0].y untouched

            vertex[1].x = 1.0f - halfPixelX;
            // vertex[1].y untouched

            vertex[2].x = -1.0f - halfPixelX;
            vertex[2].y = -1.0f + halfPixelY;

            vertex[3].x = 1.0f - halfPixelX;
            vertex[3].y = -1.0f + halfPixelY;
        }
    }
}

// Objects are pushed forward by 1m, so the coordinates need to compensate for it.
static const double OBJ_GET_ITEM_TANGENT = tan(M_PI / 8.0);

// Coordinates are in [-1, 1] range. Automatically fit and centered.
// The tangent is calculated incorrectly in game, causing distortion.
// The hook makes them move to the correct position regardless of FOV.
static float ComputeObjGetItemTangent(float fieldOfView, float aspectRatio)
{
    return tan(AdjustFieldOfView(fieldOfView, aspectRatio) / 2.0) / OBJ_GET_ITEM_TANGENT;
}

void ObjGetItemFieldOfViewMidAsmHook(PPCRegister& r1, PPCRegister& f1)
{
    if (Config::AspectRatio != EAspectRatio::OriginalNarrow)
        *reinterpret_cast<be<float>*>(g_memory.base + r1.u32 + 0x58) = ComputeObjGetItemTangent(f1.f64, g_aspectRatio);
}

static double ComputeObjGetItemX(uint32_t type)
{
    if (type >= 47) // Ring, Moon Medal, Sun Medal
    {
        double x;

        if (type == 47) // Ring
            x = 142.0;
        else // Medal
            x = 1058.0;

        x *= g_aspectRatioScale;
        x *= g_aspectRatioGameplayScale;

        double scaleOffset = (1280.0 * (1.0 - g_aspectRatioGameplayScale)) * g_aspectRatioScale;

        if (Config::UIScaleMode == EUIScaleMode::Edge)
        {
            if (type != 47) // Medal
                x += g_aspectRatioOffsetX * 2.0 + scaleOffset;
        }
        else if (Config::UIScaleMode == EUIScaleMode::Centre)
        {
            x += g_aspectRatioOffsetX + scaleOffset;
        }

        return (x - (0.5 * Video::s_viewportWidth)) / (0.5 * Video::s_viewportHeight) * OBJ_GET_ITEM_TANGENT;
    }

    return 0.0;
}

// SWA::CObjGetItem::GetX
PPC_FUNC_IMPL(__imp__sub_82690660);
PPC_FUNC(sub_82690660)
{
    if (Config::AspectRatio == EAspectRatio::OriginalNarrow)
    {
        __imp__sub_82690660(ctx, base);
        return;
    }

    uint32_t type = PPC_LOAD_U32(ctx.r3.u32 + 0xE8);
    ctx.f1.f64 = ComputeObjGetItemX(type);
}

static double ComputeObjGetItemY(uint32_t type)
{
    if (type >= 47) // Ring, Moon Medal, Sun Medal
    {
        double y;

        if (type == 47) // Ring
            y = 642.0;
        else if (type == 48) // Moon Medal
            y = 632.0;
        else if (type == 49) // Sun Medal
            y = 582.0;

        y *= g_aspectRatioScale;
        y *= g_aspectRatioGameplayScale;
        y += g_aspectRatioOffsetY * 2.0 + 720.0 * (1.0 - g_aspectRatioGameplayScale) * g_aspectRatioScale;

        return ((0.5 * Video::s_viewportHeight) - y) / (0.5 * Video::s_viewportHeight) * OBJ_GET_ITEM_TANGENT;
    }

    return 0.25;
}

// SWA::CObjGetItem::GetY
PPC_FUNC_IMPL(__imp__sub_826906A8);
PPC_FUNC(sub_826906A8)
{
    if (Config::AspectRatio == EAspectRatio::OriginalNarrow)
    {
        __imp__sub_826906A8(ctx, base);
        // Game scales Y by 1.4 at 4:3 aspect ratio.
        ctx.f1.f64 *= 1.4;
        return;
    }

    uint32_t type = PPC_LOAD_U32(ctx.r3.u32 + 0xE8);
    ctx.f1.f64 = ComputeObjGetItemY(type);
}

void WorldMapProjectionMidAsmHook(PPCVRegister& v63, PPCVRegister& v62)
{
    v63.f32[3] *= std::max(NARROW_ASPECT_RATIO, g_aspectRatio) / WIDE_ASPECT_RATIO;
    v62.f32[2] *= NARROW_ASPECT_RATIO / std::min(NARROW_ASPECT_RATIO, g_aspectRatio);
}

// CViewRing has the same exact incorrect math as CObjGetItem.
void ViewRingFieldOfViewMidAsmHook(PPCRegister& r1, PPCRegister& f1)
{
    if (Config::AspectRatio != EAspectRatio::OriginalNarrow)
        *reinterpret_cast<be<float>*>(g_memory.base + r1.u32 + 0x54) = ComputeObjGetItemTangent(f1.f64, g_aspectRatio);
}

void ViewRingYMidAsmHook(PPCRegister& f1)
{
    if (Config::AspectRatio != EAspectRatio::OriginalNarrow)
        f1.f64 = -ComputeObjGetItemY(47); // Ring
}

void ViewRingXMidAsmHook(PPCRegister& f1, PPCVRegister& v62)
{
    if (Config::AspectRatio == EAspectRatio::OriginalNarrow)
    {
        // Game scales Y by 1.4 at 4:3 aspect ratio.
        for (size_t i = 0; i < 4; i++)
            v62.f32[i] *= 1.4f;
    }
    else
    {
        f1.f64 = -ComputeObjGetItemX(47); // Ring
    }
}

// SWA::Inspire::CLetterbox::Draw
PPC_FUNC_IMPL(__imp__sub_82B8AA40);
PPC_FUNC(sub_82B8AA40)
{
    bool shouldDrawLetterbox = Config::CutsceneAspectRatio != ECutsceneAspectRatio::Unlocked && g_aspectRatio < WIDE_ASPECT_RATIO;

    PPC_STORE_U8(ctx.r3.u32, shouldDrawLetterbox);
    if (shouldDrawLetterbox)
    {
        float aspectRatio = std::max(NARROW_ASPECT_RATIO, g_aspectRatio);
        uint32_t width = aspectRatio * 720;

        PPC_STORE_U32(ctx.r3.u32 + 0xC, width);
        PPC_STORE_U32(ctx.r3.u32 + 0x10, 720);
        PPC_STORE_U32(ctx.r3.u32 + 0x14, (720 - width * 9 / 16) / 2);
    }

    __imp__sub_82B8AA40(ctx, base);
}

void InspireLetterboxTopMidAsmHook(PPCRegister& r3)
{
    *(g_memory.base + r3.u32 + PRIMITIVE_2D_PADDING_OFFSET + 0x1) = 0x01; // Letterbox Top
    *(g_memory.base + r3.u32 + PRIMITIVE_2D_PADDING_OFFSET + 0x2) = 0x00; // Letterbox Bottom
}

void InspireLetterboxBottomMidAsmHook(PPCRegister& r3)
{
    *(g_memory.base + r3.u32 + PRIMITIVE_2D_PADDING_OFFSET + 0x1) = 0x00; // Letterbox Top
    *(g_memory.base + r3.u32 + PRIMITIVE_2D_PADDING_OFFSET + 0x2) = 0x01; // Letterbox Bottom
}

void InspireSubtitleMidAsmHook(PPCRegister& r3)
{
    constexpr float NARROW_OFFSET = 485.0f;
    constexpr float WIDE_OFFSET = 560.0f;

    *reinterpret_cast<be<float>*>(g_memory.base + r3.u32 + 0x3C) = NARROW_OFFSET + (WIDE_OFFSET - NARROW_OFFSET) * g_aspectRatioNarrowScale;
}
