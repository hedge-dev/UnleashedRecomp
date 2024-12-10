#include "imgui_font_builder.h"

#include <msdf-atlas-gen/msdf-atlas-gen.h>

static bool FontBuilder_Build(ImFontAtlas* atlas)
{
    auto freeType = msdfgen::initializeFreetype();

    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    std::vector<std::pair<size_t, size_t>> ranges;

    for (auto& configData : atlas->ConfigData)
    {
        msdf_atlas::Charset charset;
        const ImWchar* glyphRanges = configData.GlyphRanges;
        while (*glyphRanges != NULL)
        {
            for (ImWchar i = glyphRanges[0]; i <= glyphRanges[1]; i++)
                charset.add(i);

            glyphRanges += 2;
        }

        size_t index = glyphs.size();

        auto font = msdfgen::loadFontData(freeType, reinterpret_cast<const msdfgen::byte*>(configData.FontData), configData.FontDataSize);

        msdf_atlas::FontGeometry fontGeometry(&glyphs);
        fontGeometry.loadCharset(font, 1.0, charset);

        msdfgen::destroyFont(font);

        ranges.emplace_back(index, glyphs.size() - index);
    }

    for (auto& glyph : glyphs)
        glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, 3.0, 0);

    msdf_atlas::TightAtlasPacker packer;
    packer.setMinimumScale(32.0);
    packer.pack(glyphs.data(), glyphs.size());

    int width = 0, height = 0;
    packer.getDimensions(width, height);

    msdf_atlas::ImmediateAtlasGenerator<float, 4, &msdf_atlas::mtsdfGenerator, msdf_atlas::BitmapAtlasStorage<uint8_t, 4>> generator(width, height);
    generator.generate(glyphs.data(), glyphs.size());

    for (size_t i = 0; i < atlas->ConfigData.size(); i++)
    {
        auto& config = atlas->ConfigData[i];
        config.DstFont->FontSize = config.SizePixels;
        config.DstFont->ConfigData = &config;
        config.DstFont->ConfigDataCount = 1;
        config.DstFont->ContainerAtlas = atlas;
        // TODO: ascent? descent? wat do they mean

        auto& [index, count] = ranges[i];
        for (size_t j = 0; j < count; j++)
        {
            auto& glyph = glyphs[index + j];
            double x0, y0, x1, y1, u0, v0, u1, v1;
            glyph.getQuadPlaneBounds(x0, y0, x1, y1);
            glyph.getQuadAtlasBounds(u0, v0, u1, v1);
            config.DstFont->AddGlyph(&config, glyph.getCodepoint(), x0, y0, x1, y1, u0, v0, u1, v1, glyph.getAdvance());
        }

        config.DstFont->BuildLookupTable();
    }

    atlas->TexReady = true;
    atlas->TexPixelsUseColors = true;
    atlas->TexPixelsRGBA32 = (unsigned int*)IM_ALLOC(width * height * 4);
    atlas->TexWidth = width;
    atlas->TexHeight = height;
    atlas->TexUvScale = { 1.0f / width, 1.0f / height };

    auto bitmapRef = (msdfgen::BitmapConstRef<uint8_t, 4>)generator.atlasStorage();
    memcpy(atlas->TexPixelsRGBA32, bitmapRef.pixels, width * height * 4);

    msdfgen::deinitializeFreetype(freeType);

    return true;
}

ImFontBuilderIO g_fontBuilderIO = { FontBuilder_Build };
