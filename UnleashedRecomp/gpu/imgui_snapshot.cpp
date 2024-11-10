#include "imgui_snapshot.h"

void ImDrawDataSnapshot::Clear()
{
    for (int n = 0; n < Cache.GetMapSize(); n++)
        if (ImDrawDataSnapshotEntry* entry = Cache.TryGetMapData(n))
            IM_DELETE(entry->OurCopy);
    Cache.Clear();
    DrawData.Clear();
}

void ImDrawDataSnapshot::SnapUsingSwap(ImDrawData* src, double current_time)
{
    ImDrawData* dst = &DrawData;
    IM_ASSERT(src != dst && src->Valid);

    // Copy all fields except CmdLists[]
    ImVector<ImDrawList*> backup_draw_list;
    backup_draw_list.swap(src->CmdLists);
    IM_ASSERT(src->CmdLists.Data == NULL);
    *dst = *src;
    backup_draw_list.swap(src->CmdLists);

    // Swap and mark as used
    for (ImDrawList* src_list : src->CmdLists)
    {
        ImDrawDataSnapshotEntry* entry = GetOrAddEntry(src_list);
        if (entry->OurCopy == NULL)
        {
            entry->SrcCopy = src_list;
            entry->OurCopy = IM_NEW(ImDrawList)(src_list->_Data);
        }
        IM_ASSERT(entry->SrcCopy == src_list);
        entry->SrcCopy->CmdBuffer.swap(entry->OurCopy->CmdBuffer); // Cheap swap
        entry->SrcCopy->IdxBuffer.swap(entry->OurCopy->IdxBuffer);
        entry->SrcCopy->VtxBuffer.swap(entry->OurCopy->VtxBuffer);
        entry->SrcCopy->CmdBuffer.reserve(entry->OurCopy->CmdBuffer.Capacity); // Preserve bigger size to avoid reallocs for two consecutive frames
        entry->SrcCopy->IdxBuffer.reserve(entry->OurCopy->IdxBuffer.Capacity);
        entry->SrcCopy->VtxBuffer.reserve(entry->OurCopy->VtxBuffer.Capacity);
        entry->LastUsedTime = current_time;
        dst->CmdLists.push_back(entry->OurCopy);
    }

    // Cleanup unused data
    const double gc_threshold = current_time - MemoryCompactTimer;
    for (int n = 0; n < Cache.GetMapSize(); n++)
        if (ImDrawDataSnapshotEntry* entry = Cache.TryGetMapData(n))
        {
            if (entry->LastUsedTime > gc_threshold)
                continue;
            IM_DELETE(entry->OurCopy);
            Cache.Remove(GetDrawListID(entry->SrcCopy), entry);
        }
};
