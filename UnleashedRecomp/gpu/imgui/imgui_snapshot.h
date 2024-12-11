#pragma once

// https://github.com/ocornut/imgui/issues/1860#issuecomment-1927630727

// Usage:
//  static ImDrawDataSnapshot snapshot; // Important: make persistent accross frames to reuse buffers.
//  snapshot.SnapUsingSwap(ImGui::GetDrawData(), ImGui::GetTime());
//  [...]
//  ImGui_ImplDX11_RenderDrawData(&snapshot.DrawData);

struct ImDrawDataSnapshotEntry
{
    ImDrawList* SrcCopy = NULL;
    ImDrawList* OurCopy = NULL;
    double LastUsedTime = 0.0;
};

struct ImDrawDataSnapshot
{
    // Members
    ImDrawData DrawData;
    ImPool<ImDrawDataSnapshotEntry> Cache;
    float MemoryCompactTimer = 20.0f; // Discard unused data after 20 seconds

    ~ImDrawDataSnapshot() { Clear(); }
    void Clear();
    void SnapUsingSwap(ImDrawData* src, double current_time); // Efficient snapshot by swapping data, meaning "src_list" is unusable.

    // Internals
    ImGuiID GetDrawListID(ImDrawList* src_list) { return ImHashData(&src_list, sizeof(src_list)); } // Hash pointer
    ImDrawDataSnapshotEntry* GetOrAddEntry(ImDrawList* src_list) { return Cache.GetOrAddByKey(GetDrawListID(src_list)); }
};

// Undefine this to generate a font atlas file in working directory.
// You also need to do this if you are testing localization, as only
// characters in the locale get added to the atlas.
#define ENABLE_IM_FONT_ATLAS_SNAPSHOT

struct ImFontAtlasSnapshot
{
    std::vector<uint8_t> data;
    ankerl::unordered_dense::map<const void*, size_t> objects;
    std::vector<uint32_t> offsets;

    template<typename T1, typename T2>
    void SnapPointer(size_t offset, const T1& value, const T2& ptr, size_t count);

    template<typename T>
    void Traverse(size_t offset, const T& value);

    template<typename T>
    size_t Snap(const T& value);

    void Snap();

    static ImFontAtlas* Load();

    static void GenerateGlyphRanges();

    // When ENABLE_IM_FONT_ATLAS_SNAPSHOT is undefined, this creates the font runtime instead. 
    static ImFont* GetFont(const char* name);
};
