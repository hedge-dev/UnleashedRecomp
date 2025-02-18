#pragma once

#include <SWA.inl>

namespace SWA
{
    struct SGlobals
    {
        // ms_DrawLightFieldSamplingPoint: サンプリング点をデバッグ表示
        static inline bool* ms_DrawLightFieldSamplingPoint;

        // N/A
        static inline be<float>* ms_FreeCameraSpeed;

        // N/A
        static inline bool* ms_IsAutoSaveWarningShown;

        // ms_IgnoreLightFieldData: データを無視する
        static inline bool* ms_IgnoreLightFieldData;

        // IsCollisionRender
        static inline bool* ms_IsCollisionRender;

        // N/A
        static inline bool* ms_IsLoading;

        // IsObjectCollisionRender
        static inline bool* ms_IsObjectCollisionRender;

        // ms_IsRenderDebugDraw: デバッグ描画
        static inline bool* ms_IsRenderDebugDraw;

        // ms_IsRenderDebugDrawText: デバッグ文字描画
        static inline bool* ms_IsRenderDebugDrawText;

        // ms_IsRenderDebugPositionDraw: デバッグ位置描画
        static inline bool* ms_IsRenderDebugPositionDraw;

        // N/A
        static inline bool* ms_IsRenderDepthOfField;

        // ms_IsRenderGameMainHud: ゲームメインHUD 描画
        static inline bool* ms_IsRenderGameMainHud;

        // ms_IsRenderHud: 全 HUD 描画
        static inline bool* ms_IsRenderHud;

        // ms_IsRenderHudPause: ポーズメニュー 描画
        static inline bool* ms_IsRenderHudPause;

        // N/A
        static inline bool* ms_IsTitleStateMenu;

        // IsTriggerRender
        static inline bool* ms_IsTriggerRender;

        // ms_LightFieldDebug: 値をデバッグ表示
        static inline bool* ms_LightFieldDebug;

        // VisualizeLoadedLevel: ミップレベルを視覚化 赤=0, 緑=1, 青=2, 黄=未ロード
        static inline bool* ms_VisualizeLoadedLevel;

        static void Init()
        {
            ms_DrawLightFieldSamplingPoint = (bool*)MmGetHostAddress(0x83367BCE);
            ms_FreeCameraSpeed = (be<float>*)MmGetHostAddress(0x83366DF8);
            ms_IgnoreLightFieldData = (bool*)MmGetHostAddress(0x83367BCF);
            ms_IsAutoSaveWarningShown = (bool*)MmGetHostAddress(0x83367BC1);
            ms_IsCollisionRender = (bool*)MmGetHostAddress(0x833678A6);
            ms_IsLoading = (bool*)MmGetHostAddress(0x83367A4C);
            ms_IsObjectCollisionRender = (bool*)MmGetHostAddress(0x83367905);
            ms_IsRenderDebugDraw = (bool*)MmGetHostAddress(0x8328BB23);
            ms_IsRenderDebugDrawText = (bool*)MmGetHostAddress(0x8328BB25);
            ms_IsRenderDebugPositionDraw = (bool*)MmGetHostAddress(0x8328BB24);
            ms_IsRenderDepthOfField = (bool*)MmGetHostAddress(0x83302720);
            ms_IsRenderGameMainHud = (bool*)MmGetHostAddress(0x8328BB27);
            ms_IsRenderHud = (bool*)MmGetHostAddress(0x8328BB26);
            ms_IsRenderHudPause = (bool*)MmGetHostAddress(0x8328BB28);
            ms_IsTitleStateMenu = (bool*)MmGetHostAddress(0x83367BC0);
            ms_IsTriggerRender = (bool*)MmGetHostAddress(0x83367904);
            ms_LightFieldDebug = (bool*)MmGetHostAddress(0x83367BCD);
            ms_VisualizeLoadedLevel = (bool*)MmGetHostAddress(0x833678C1);
        }
    };
}
