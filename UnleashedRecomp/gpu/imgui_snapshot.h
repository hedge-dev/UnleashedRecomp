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
