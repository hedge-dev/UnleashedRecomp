#pragma once


namespace Reddog
{
    struct Vector3
    {
        float x, y, z;
        constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    };

    enum SDrawTextFlags : uint8_t
    {
        eDrawTextFlags_None     = 0,
        eDrawTextFlags_Overlay  = 1 << 0,
        eDrawTextFlags_NoShadow = 1 << 1,
    };

    struct SDrawLine
    {
        Vector3 Start { 0,0,0 };
        Vector3 End { 0,0,0 };
        ImU32 Colour { IM_COL32(255, 255, 255, 255) };
        float Size { 2 };
    };

    struct SDrawText
    {
        ImVec2 Position { 0,0 };
        std::string Text;
        float Time { 0 };
        float Scale { 1 };
        ImU32 Colour { IM_COL32(255, 255, 255, 255) };
        SDrawTextFlags Flags { eDrawTextFlags_None };
        ImU16 Priority { 0 };
    };


    ImVec2 GetNDCCoordinate(const Vector3& in_rPosition);


    class DebugDraw
    {
    public:
        static inline bool ms_IsRendering = false;

        static inline std::vector<SDrawLine> ms_LineList = {};
        static inline std::vector<SDrawText> ms_FreeTextList = {};
        static inline std::vector<SDrawText> ms_LogTextList = {};

        static void DrawLine(const SDrawLine& in_rLine);

        static void DrawText2D(const SDrawText& in_rText);
        static void DrawText2D(const SDrawText& in_rText, const Vector3& in_rPosition);

        static void DrawTextLog(const SDrawText& in_rText);
        static void DrawTextLog(const char* in_Text, float in_Time = 0, ImU32 in_Colour = IM_COL32(255, 255, 255, 255), ImU16 in_Priority = 0);

        static void Render(ImFont* font);

        static bool GetIsDrawDebug();
        static bool GetIsDrawText();
        static bool GetIsDrawPosition();
    };
}
