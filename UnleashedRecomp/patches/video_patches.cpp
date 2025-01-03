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

    EXTEND_LEFT = 1 << 6,
    EXTEND_RIGHT = 1 << 7,

    UNSTRETCH_TOP = 1 << 8,
    UNSTRETCH_LEFT = 1 << 9,
    UNSTRETCH_BOTTOM = 1 << 10,
    UNSTRETCH_RIGHT = 1 << 11,
    UNSTRETCH_CENTER = 1 << 12,
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

    // ui_gate
    { HashStr("ui_gate/header/status_title"), UNSTRETCH_LEFT },
    { HashStr("ui_gate/header/status_title/title_bg/center"), STRETCH_HORIZONTAL },
    { HashStr("ui_gate/header/status_title/title_bg/center/h_light"), STRETCH_HORIZONTAL },
    { HashStr("ui_gate/header/status_title/title_txt_12"), UNSTRETCH_RIGHT },
    
    // ui_general
    { HashStr("ui_general/bg"), STRETCH },

    // ui_itemresult
    { HashStr("ui_itemresult/main/iresult_title"), UNSTRETCH_LEFT },
    { HashStr("ui_itemresult/main/iresult_title/title_bg/center"), STRETCH_HORIZONTAL },
    { HashStr("ui_itemresult/main/iresult_title/title_bg/center/h_light"), STRETCH_HORIZONTAL },
    { HashStr("ui_itemresult/main/iresult_title/title_txt_S"), UNSTRETCH_RIGHT },

    // ui_lcursor
    { HashStr("ui_lcursor/cursor"), UNSTRETCH_CENTER },  
    
    // ui_lcursor_enemy
    { HashStr("ui_lcursor/cursor"), UNSTRETCH_CENTER },

    // ui_mediaroom
    { HashStr("ui_mediaroom/header/bg/img_1"), EXTEND_LEFT },
    { HashStr("ui_mediaroom/header/bg/img_10"), EXTEND_RIGHT },
    { HashStr("ui_mediaroom/header/frame/img_1"), EXTEND_LEFT },
    { HashStr("ui_mediaroom/header/frame/img_5"), EXTEND_RIGHT },
    
    // ui_missionscreen
    { HashStr("ui_missionscreen/player_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/time_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/score_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/item_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/laptime_count"), ALIGN_TOP_LEFT },
    { HashStr("ui_missionscreen/lap_count"), ALIGN_TOP_RIGHT },

    // ui_misson
    { HashStr("ui_misson/bg"), STRETCH },

    { HashStr("ui_misson/header/misson_title_B"), UNSTRETCH_LEFT },
    { HashStr("ui_misson/header/misson_title_B/title_bg/center"), STRETCH_HORIZONTAL },
    { HashStr("ui_misson/header/misson_title_B/title_bg/center/h_light"), STRETCH_HORIZONTAL },
    { HashStr("ui_misson/header/misson_title_B/title_txt_13"), UNSTRETCH_RIGHT },

    // ui_pause
    { HashStr("ui_pause/header/status_title"), UNSTRETCH_LEFT },
    { HashStr("ui_pause/header/status_title/title_bg/center"), STRETCH_HORIZONTAL },
    { HashStr("ui_pause/header/status_title/title_bg/center/h_light"), STRETCH_HORIZONTAL },
    { HashStr("ui_pause/header/status_title/title_txt_11"), UNSTRETCH_RIGHT },

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

    // ui_result
    { HashStr("ui_result/main/result_title"), UNSTRETCH_LEFT },
    { HashStr("ui_result/main/result_title/title_bg/center"), STRETCH_HORIZONTAL },
    { HashStr("ui_result/main/result_title/title_bg/center/h_light"), STRETCH_HORIZONTAL },
    { HashStr("ui_result/main/result_title/title_txt_S"), UNSTRETCH_RIGHT },

    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1/right"), STRETCH },
    { HashStr("ui_result/main/result_num_1/num_bg/position_1/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1/right"), STRETCH },
    { HashStr("ui_result/main/result_num_2/num_bg/position_2/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1/right"), STRETCH },
    { HashStr("ui_result/main/result_num_3/num_bg/position_3/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1/right"), STRETCH },
    { HashStr("ui_result/main/result_num_4/num_bg/position_4/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1/right"), STRETCH },
    { HashStr("ui_result/main/result_num_5/num_bg/position_5/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center"), EXTEND_LEFT },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center/h_light"), EXTEND_LEFT },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center/left"), STRETCH },
    { HashStr("ui_result/main/result_num_6/num_bg/position_6/center/left/h_light"), STRETCH }, 
    
    // ui_result_ex
    { HashStr("ui_result_ex/main/result_title"), UNSTRETCH_LEFT },
    { HashStr("ui_result_ex/main/result_title/title_bg/center"), STRETCH_HORIZONTAL },
    { HashStr("ui_result_ex/main/result_title/title_bg/center/h_light"), STRETCH_HORIZONTAL },
    { HashStr("ui_result_ex/main/result_title/title_txt_S"), UNSTRETCH_RIGHT },

    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1/right"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_1/position_1/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_2/position_2/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_2/position_2/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_2/position_2/center_1/right"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_2/position_2/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_3/position_3/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_3/position_3/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_3/position_3/center_1/right"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_3/position_3/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_4/position_4/center_1"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_4/position_4/center_1/h_light"), EXTEND_RIGHT },
    { HashStr("ui_result_ex/main/number/result_num_4/position_4/center_1/right"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_4/position_4/center_1/right/h_light"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_6/position_6/center"), EXTEND_LEFT },
    { HashStr("ui_result_ex/main/number/result_num_6/position_6/center/h_light"), EXTEND_LEFT },
    { HashStr("ui_result_ex/main/number/result_num_6/position_6/center/left"), STRETCH },
    { HashStr("ui_result_ex/main/number/result_num_6/position_6/center/left/h_light"), STRETCH },

    // ui_saveicon
    { HashStr("ui_saveicon/icon"), ALIGN_BOTTOM_RIGHT },

    // ui_status
    { HashStr("ui_status/header/status_title"), UNSTRETCH_LEFT },
    { HashStr("ui_status/header/status_title/title_bg/center"), STRETCH_HORIZONTAL },
    { HashStr("ui_status/header/status_title/title_bg/center/h_light"), STRETCH_HORIZONTAL },
    { HashStr("ui_status/header/status_title/title_txt_6"), UNSTRETCH_RIGHT },

    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center"), EXTEND_LEFT },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center/h_light"), EXTEND_LEFT },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center/left"), STRETCH },
    { HashStr("ui_status/main/tag/bg/tag_bg_1/total_1_bg/center/left/h_light"), STRETCH },

    // ui_townscreen
    { HashStr("ui_townscreen/time"), ALIGN_TOP_RIGHT },
    { HashStr("ui_townscreen/time_effect"), ALIGN_TOP_RIGHT },
    { HashStr("ui_townscreen/info"), ALIGN_TOP_LEFT },
    { HashStr("ui_townscreen/cam"), ALIGN_TOP_RIGHT },

    // ui_worldmap
    { HashStr("ui_worldmap/contents/guide/cts_guide_1_hiscore"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_2_besttime"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_3_rank"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_4_misson"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_5_medal"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_bg"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_icon"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_ss"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_txt"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/guide/cts_guide_window"), ALIGN_RIGHT },
    { HashStr("ui_worldmap/contents/info/bg/cts_info_bg"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/bg/info_bg_1"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_1"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_2"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_3"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/info/img/info_img_4"), ALIGN_TOP_LEFT },
    { HashStr("ui_worldmap/contents/stage/cts_stage_scroll_bar"), ALIGN_LEFT },
    { HashStr("ui_worldmap/contents/stage/cts_stage_scroll_bg"), ALIGN_LEFT },
    { HashStr("ui_worldmap/contents/stage/cts_stage_select"), ALIGN_LEFT },
    { HashStr("ui_worldmap/contents/stage/cts_stage_window"), ALIGN_LEFT },
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
    float minX = 1280.0f;
    float minY = 720.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    for (size_t i = 0; i < ctx.r5.u32; i++)
    {
        auto position = reinterpret_cast<be<float>*>(base + ctx.r4.u32 + i * stride);
        minX = std::min<float>(position[0], minX);
        minY = std::min<float>(position[1], minY);
        maxX = std::max<float>(position[0], maxX);
        maxY = std::max<float>(position[1], maxY);
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;

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
    else
    {
        if (minX < 0.01f && maxX > 1279.99f)
            flags |= STRETCH_HORIZONTAL;

        if (minY < 0.01f && maxY > 719.99f)
            flags |= STRETCH_VERTICAL;
    }

    uint32_t size = ctx.r5.u32 * stride;
    ctx.r1.u32 -= size;

    uint8_t* stack = base + ctx.r1.u32;
    memcpy(stack, base + ctx.r4.u32, size);

    constexpr float ORIGINAL_ASPECT_RATIO = 1280.0f / 720.0f;
    float aspectRatio = float(GameWindow::s_width) / GameWindow::s_height;

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float pivotX = 0.0f;
    float pivotY = 0.0f;

    if ((flags & STRETCH_HORIZONTAL) == 0)
    {
        scaleX = ORIGINAL_ASPECT_RATIO / aspectRatio;

        if ((flags & (ALIGN_LEFT | UNSTRETCH_LEFT | UNSTRETCH_RIGHT | UNSTRETCH_CENTER)) == 0)
        {
            offsetX = (1.0f - scaleX) * 1280.0f;

            if ((flags & ALIGN_RIGHT) == 0)
                offsetX *= 0.5f;
        }
    }

    if ((flags & STRETCH_VERTICAL) == 0)
    {
        scaleY = aspectRatio / ORIGINAL_ASPECT_RATIO;

        if ((flags & (ALIGN_TOP | UNSTRETCH_TOP | UNSTRETCH_BOTTOM | UNSTRETCH_CENTER)) == 0)
        {
            offsetY = (1.0f - scaleY) * 720.0f;

            if ((flags & ALIGN_BOTTOM) == 0)
                offsetY *= 0.5f;
        }
    }

    if ((flags & UNSTRETCH_LEFT) != 0)
    {
        pivotX = minX;
    }
    else if ((flags & UNSTRETCH_RIGHT) != 0)
    {
        pivotX = maxX;
    }
    else if ((flags & UNSTRETCH_TOP) != 0)
    {
        pivotY = minY;
    }
    else if ((flags & UNSTRETCH_BOTTOM) != 0)
    {
        pivotY = maxY;
    }
    else if ((flags & UNSTRETCH_CENTER) != 0)
    {
        pivotX = centerX;
        pivotY = centerY;
    }

    offsetX += pivotX;
    offsetY += pivotY;

    for (size_t i = 0; i < ctx.r5.u32; i++)
    {
        auto position = reinterpret_cast<be<float>*>(stack + i * stride);

        if (aspectRatio >= ORIGINAL_ASPECT_RATIO)
        {
            if ((flags & EXTEND_LEFT) != 0 && (position[0] <= centerX))
            {
                position[0] = 0.0f;
            }
            else if ((flags & EXTEND_RIGHT) != 0 && (position[0] >= centerX))
            {
                position[0] = 1280.0f;
            }
            else 
            {
                position[0] = (position[0] - pivotX) * scaleX + offsetX;
            }
        }
        else
        {
            position[1] = (position[1] - pivotY) * scaleY + offsetY;
        }
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
