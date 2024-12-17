#include "imgui_snapshot.h"

#include <locale/locale.h>
#include <res/font/im_font_atlas.bin.h>
#include <user/config.h>
#include <decompressor.h>
#include <kernel/xdbf.h>
#include <app.h>

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

template<typename T1, typename T2>
void ImFontAtlasSnapshot::SnapPointer(size_t offset, const T1& value, const T2& ptr, size_t count)
{
    if (ptr != nullptr && count != 0)
    {
        if (!objects.contains(ptr))
        {
            constexpr size_t ALIGN = alignof(std::remove_pointer_t<T2>);
            constexpr size_t SIZE = sizeof(std::remove_pointer_t<T2>);

            size_t ptrOffset = (data.size() + ALIGN - 1) & ~(ALIGN - 1);
            data.resize(ptrOffset + SIZE * count);
            memcpy(&data[ptrOffset], ptr, SIZE * count);

            for (size_t i = 0; i < count; i++)
            {
                size_t curPtrOffset = ptrOffset + SIZE * i;
                objects[&ptr[i]] = curPtrOffset;
                Traverse(curPtrOffset, ptr[i]);
            }
        }

        size_t fieldOffset = offset + (reinterpret_cast<ptrdiff_t>(&ptr) - reinterpret_cast<ptrdiff_t>(&value));
        *reinterpret_cast<size_t*>(&data[fieldOffset]) = objects[ptr];
        offsets.push_back(fieldOffset);
    }
}

template<typename T>
void ImFontAtlasSnapshot::Traverse(size_t offset, const T& value)
{
    if constexpr (std::is_pointer_v<T>)
    {
        SnapPointer(offset, value, value, 1);
    }
    else if constexpr (std::is_same_v<T, ImFontAtlas>)
    {
        SnapPointer(offset, value, value.ConfigData.Data, value.ConfigData.Size);
        SnapPointer(offset, value, value.CustomRects.Data, value.CustomRects.Size);
        SnapPointer(offset, value, value.Fonts.Data, value.Fonts.Size);
    }
    else if constexpr (std::is_same_v<T, ImFont>)
    {
        SnapPointer(offset, value, value.IndexAdvanceX.Data, value.IndexAdvanceX.Size);
        SnapPointer(offset, value, value.IndexLookup.Data, value.IndexLookup.Size);
        SnapPointer(offset, value, value.Glyphs.Data, value.Glyphs.Size);
        SnapPointer(offset, value, value.FallbackGlyph, 1);
        SnapPointer(offset, value, value.ContainerAtlas, 1);
        SnapPointer(offset, value, value.ConfigData, value.ConfigDataCount);
    }
    else if constexpr (std::is_same_v<T, ImFontAtlasCustomRect>)
    {
        SnapPointer(offset, value, value.Font, 1);
    }  
    else if constexpr (std::is_same_v<T, ImFontConfig>)
    {
        SnapPointer(offset, value, value.GlyphRanges, value.GlyphRanges != nullptr ? wcslen(reinterpret_cast<const wchar_t*>(value.GlyphRanges)) + 1 : 0);
        SnapPointer(offset, value, value.DstFont, 1);
    }
}

template<typename T>
size_t ImFontAtlasSnapshot::Snap(const T& value)
{
    size_t offset = (data.size() + alignof(T) - 1) & ~(alignof(T) - 1);
    data.resize(offset + sizeof(T));
    memcpy(&data[offset], &value, sizeof(T));
    objects[&value] = offset;
    Traverse(offset, value);
    return offset;
}

struct ImFontAtlasSnapshotHeader
{
    uint32_t imguiVersion;
    uint32_t dataOffset;
    uint32_t offsetCount;
    uint32_t offsetsOffset;
};

void ImFontAtlasSnapshot::Snap()
{
    data.resize(sizeof(ImFontAtlasSnapshotHeader));
    size_t dataOffset = Snap(*ImGui::GetIO().Fonts);

    size_t offsetsOffset = data.size();
    std::sort(offsets.begin(), offsets.end());

    data.insert(data.end(), 
        reinterpret_cast<uint8_t*>(offsets.data()),
        reinterpret_cast<uint8_t*>(offsets.data() + offsets.size()));

    auto header = reinterpret_cast<ImFontAtlasSnapshotHeader*>(data.data());
    header->imguiVersion = IMGUI_VERSION_NUM;
    header->dataOffset = dataOffset;
    header->offsetCount = offsets.size();
    header->offsetsOffset = offsetsOffset;
}

static std::unique_ptr<uint8_t[]> g_imFontAtlas;

ImFontAtlas* ImFontAtlasSnapshot::Load()
{
    g_imFontAtlas = decompressZstd(g_im_font_atlas, g_im_font_atlas_uncompressed_size);

    auto header = reinterpret_cast<ImFontAtlasSnapshotHeader*>(g_imFontAtlas.get());
    assert(header->imguiVersion == IMGUI_VERSION_NUM && "ImGui version mismatch, the font atlas needs to be regenerated!");

    auto offsetTable = reinterpret_cast<uint32_t*>(g_imFontAtlas.get() + header->offsetsOffset);
    for (size_t i = 0; i < header->offsetCount; i++)
    {
        *reinterpret_cast<size_t*>(g_imFontAtlas.get() + (*offsetTable)) += reinterpret_cast<size_t>(g_imFontAtlas.get());
        ++offsetTable;
    }

    return reinterpret_cast<ImFontAtlas*>(g_imFontAtlas.get() + header->dataOffset);
}


static void GetGlyphs(std::set<ImWchar>& glyphs, const std::string_view& value)
{
    const char* cur = value.data();
    while (cur < value.data() + value.size())
    {
        unsigned int c;
        cur += ImTextCharFromUtf8(&c, cur, value.data() + value.size());
        glyphs.emplace(c);
    }
}

static std::vector<ImWchar> g_glyphRanges;

void ImFontAtlasSnapshot::GenerateGlyphRanges()
{
    std::vector<std::string_view> localeStrings;

    for (auto& config : g_configDefinitions)
        config->GetLocaleStrings(localeStrings);

    std::set<ImWchar> glyphs;

    for (size_t i = 0x20; i <= 0xFF; i++)
        glyphs.emplace(i);

    for (auto& localeString : localeStrings)
        GetGlyphs(glyphs, localeString);

    for (auto& [name, locale] : g_locale)
    {
        for (auto& [language, value] : locale)
            GetGlyphs(glyphs, value);
    }

    for (size_t i = XDBF_LANGUAGE_ENGLISH; i <= XDBF_LANGUAGE_ITALIAN; i++)
    {
        auto achievements = g_xdbfWrapper.GetAchievements(static_cast<EXDBFLanguage>(i));
        for (auto& achievement : achievements)
        {
            GetGlyphs(glyphs, achievement.Name);
            GetGlyphs(glyphs, achievement.UnlockedDesc);
            GetGlyphs(glyphs, achievement.LockedDesc);
        }
    }

    for (auto glyph : glyphs)
    {
        if (g_glyphRanges.empty() || (g_glyphRanges.back() + 1) != glyph)
        {
            g_glyphRanges.push_back(glyph);
            g_glyphRanges.push_back(glyph);
        }
        else
        {
            g_glyphRanges.back() = glyph;
        }
    }

    g_glyphRanges.push_back(0);
}

ImFont* ImFontAtlasSnapshot::GetFont(const char* name)
{
    auto fontAtlas = ImGui::GetIO().Fonts;
    for (auto& configData : fontAtlas->ConfigData)
    {
        if (strstr(configData.Name, name) != nullptr)
        {
            assert(configData.DstFont != nullptr);
            return configData.DstFont;
        }
    }

#ifdef ENABLE_IM_FONT_ATLAS_SNAPSHOT
    assert(false && "Unable to locate equivalent font in the atlas file.");
#endif

    return fontAtlas->AddFontFromFileTTF(name, 24.0f, nullptr, g_glyphRanges.data());
}
