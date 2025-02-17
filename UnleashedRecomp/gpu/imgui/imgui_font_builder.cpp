#include "imgui_font_builder.h"

#include <msdf-atlas-gen/msdf-atlas-gen.h>

// Taken directly from msdf-atlas-gen, modified to support custom rectangles.
struct TightAtlasPacker {

    TightAtlasPacker() :
        width(-1), height(-1),
        spacing(0),
        dimensionsConstraint(msdf_atlas::DimensionsConstraint::POWER_OF_TWO_SQUARE),
        scale(-1),
        minScale(1),
        unitRange(0),
        pxRange(0),
        miterLimit(0),
        pxAlignOriginX(false), pxAlignOriginY(false),
        scaleMaximizationTolerance(.001)
    {
    }

    int pack(msdf_atlas::GlyphGeometry* glyphs, int count, msdf_atlas::Rectangle* customRects, int customRectCount) {
        double initialScale = scale > 0 ? scale : minScale;
        if (initialScale > 0) {
            if (int remaining = tryPack(glyphs, count, customRects, customRectCount, dimensionsConstraint, width, height, initialScale))
                return remaining;
        }
        else if (width < 0 || height < 0)
            return -1;
        if (scale <= 0)
            scale = packAndScale(glyphs, count, customRects, customRectCount);
        if (scale <= 0)
            return -1;
        return 0;
    }

    int width, height;
    int spacing;
    msdf_atlas::DimensionsConstraint dimensionsConstraint;
    double scale;
    double minScale;
    msdfgen::Range unitRange;
    msdfgen::Range pxRange;
    double miterLimit;
    bool pxAlignOriginX, pxAlignOriginY;
    msdf_atlas::Padding innerUnitPadding, outerUnitPadding;
    msdf_atlas::Padding innerPxPadding, outerPxPadding;
    double scaleMaximizationTolerance;

    int tryPack(msdf_atlas::GlyphGeometry* glyphs, int count, msdf_atlas::Rectangle* customRects, int customRectCount, msdf_atlas::DimensionsConstraint dimensionsConstraint, int& width, int& height, double scale) const {
        // Wrap glyphs into boxes
        std::vector<msdf_atlas::Rectangle> rectangles;
        std::vector<msdf_atlas::GlyphGeometry*> rectangleGlyphs;
        rectangles.reserve(count + customRectCount);
        rectangleGlyphs.reserve(count);
        msdf_atlas::GlyphGeometry::GlyphAttributes attribs = { };
        attribs.scale = scale;
        attribs.range = { unitRange.lower + pxRange.lower / scale, unitRange.upper + pxRange.upper / scale };
        attribs.innerPadding = innerUnitPadding + innerPxPadding / scale;
        attribs.outerPadding = outerUnitPadding + outerPxPadding / scale;
        attribs.miterLimit = miterLimit;
        attribs.pxAlignOriginX = pxAlignOriginX;
        attribs.pxAlignOriginY = pxAlignOriginY;
        for (msdf_atlas::GlyphGeometry* glyph = glyphs, *end = glyphs + count; glyph < end; ++glyph) {
            if (!glyph->isWhitespace()) {
                msdf_atlas::Rectangle rect = { };
                glyph->wrapBox(attribs);
                glyph->getBoxSize(rect.w, rect.h);
                if (rect.w > 0 && rect.h > 0) {
                    rectangles.push_back(rect);
                    rectangleGlyphs.push_back(glyph);
                }
            }
        }
        rectangles.insert(rectangles.end(), customRects, customRects + customRectCount);
        // No non-zero size boxes?
        if (rectangles.empty()) {
            if (width < 0 || height < 0)
                width = 0, height = 0;
            return 0;
        }
        // Box rectangle packing
        if (width < 0 || height < 0) {
            std::pair<int, int> dimensions = std::make_pair(width, height);
            switch (dimensionsConstraint) {
            case msdf_atlas::DimensionsConstraint::POWER_OF_TWO_SQUARE:
                dimensions = msdf_atlas::packRectangles<msdf_atlas::SquarePowerOfTwoSizeSelector>(rectangles.data(), rectangles.size(), spacing);
                break;
            case msdf_atlas::DimensionsConstraint::POWER_OF_TWO_RECTANGLE:
                dimensions = msdf_atlas::packRectangles<msdf_atlas::PowerOfTwoSizeSelector>(rectangles.data(), rectangles.size(), spacing);
                break;
            case msdf_atlas::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE:
                dimensions = msdf_atlas::packRectangles<msdf_atlas::SquareSizeSelector<4> >(rectangles.data(), rectangles.size(), spacing);
                break;
            case msdf_atlas::DimensionsConstraint::EVEN_SQUARE:
                dimensions = msdf_atlas::packRectangles<msdf_atlas::SquareSizeSelector<2> >(rectangles.data(), rectangles.size(), spacing);
                break;
            case msdf_atlas::DimensionsConstraint::SQUARE:
            default:
                dimensions = msdf_atlas::packRectangles<msdf_atlas::SquareSizeSelector<> >(rectangles.data(), rectangles.size(), spacing);
                break;
            }
            if (!(dimensions.first > 0 && dimensions.second > 0))
                return -1;
            width = dimensions.first, height = dimensions.second;
        }
        else {
            if (int result = packRectangles(rectangles.data(), rectangles.size(), width, height, spacing))
                return result;
        }

        // Set glyph box placement
        for (size_t i = 0; i < rectangleGlyphs.size(); ++i)
            rectangleGlyphs[i]->placeBox(rectangles[i].x, height - (rectangles[i].y + rectangles[i].h));     

        for (int i = 0; i < customRectCount; ++i) {
            customRects[i].x = rectangles[rectangleGlyphs.size() + i].x;
            customRects[i].y = height - (rectangles[rectangleGlyphs.size() + i].y + rectangles[rectangleGlyphs.size() + i].h);
        }

        return 0;
    }

    double packAndScale(msdf_atlas::GlyphGeometry* glyphs, int count, msdf_atlas::Rectangle* customRects, int customRectCount) const {
        bool lastResult = false;
        int w = width, h = height;
#define TRY_PACK(scale) (lastResult = !tryPack(glyphs, count, customRects, customRectCount, msdf_atlas::DimensionsConstraint(), w, h, (scale)))
        double minScale = 1, maxScale = 1;
        if (TRY_PACK(1)) {
            while (maxScale < 1e+32 && ((maxScale = 2 * minScale), TRY_PACK(maxScale)))
                minScale = maxScale;
        }
        else {
            while (minScale > 1e-32 && ((minScale = .5 * maxScale), !TRY_PACK(minScale)))
                maxScale = minScale;
        }
        if (minScale == maxScale)
            return 0;
        while (minScale / maxScale < 1 - scaleMaximizationTolerance) {
            double midScale = .5 * (minScale + maxScale);
            if (TRY_PACK(midScale))
                minScale = midScale;
            else
                maxScale = midScale;
        }
        if (!lastResult)
            TRY_PACK(minScale);
        return minScale;
    }
};

extern void ImFontAtlasBuildInit(ImFontAtlas* atlas);
extern void ImFontAtlasBuildFinish(ImFontAtlas* atlas);

static bool FontBuilder_Build(ImFontAtlas* atlas)
{
    ImFontAtlasBuildInit(atlas);

    auto freeType = msdfgen::initializeFreetype();

    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    std::vector<std::pair<size_t, size_t>> ranges;
    std::vector<msdf_atlas::Rectangle> customRects;

    for (auto& config : atlas->ConfigData)
    {
        msdf_atlas::Charset charset;
        const ImWchar* glyphRanges = config.GlyphRanges;
        while (*glyphRanges != NULL)
        {
            for (ImWchar i = glyphRanges[0]; i <= glyphRanges[1]; i++)
                charset.add(i);

            glyphRanges += 2;
        }

        size_t index = glyphs.size();

        auto font = msdfgen::loadFontData(freeType, reinterpret_cast<const msdfgen::byte*>(config.FontData), config.FontDataSize);

        msdf_atlas::FontGeometry fontGeometry(&glyphs);
        fontGeometry.loadCharset(font, config.SizePixels, charset);

        auto& metrics = fontGeometry.getMetrics();
        config.DstFont->FontSize = config.SizePixels;
        config.DstFont->ConfigData = &config;
        config.DstFont->ConfigDataCount = 1;
        config.DstFont->ContainerAtlas = atlas;
        config.DstFont->Ascent = metrics.ascenderY;
        config.DstFont->Descent = metrics.descenderY;

        msdfgen::destroyFont(font);

        ranges.emplace_back(index, glyphs.size() - index);
    }

    for (auto& glyph : glyphs)
        glyph.edgeColoring(&msdfgen::edgeColoringByDistance, 3.0, 0);

    for (auto& customRect : atlas->CustomRects)
        customRects.emplace_back(0, 0, int(customRect.Width), int(customRect.Height));

    TightAtlasPacker packer;
    packer.spacing = 1;
    packer.dimensionsConstraint = msdf_atlas::DimensionsConstraint::POWER_OF_TWO_RECTANGLE;
    packer.miterLimit = 1.0;
    packer.pxRange = 8.0;
    packer.pack(glyphs.data(), glyphs.size(), customRects.data(), customRects.size());

    for (size_t i = 0; i < customRects.size(); i++)
    {
        auto& srcRect = customRects[i];
        auto& dstRect = atlas->CustomRects[i];
        dstRect.X = srcRect.x;
        dstRect.Y = srcRect.y;
    }

    msdf_atlas::ImmediateAtlasGenerator<float, 3, &msdf_atlas::msdfGenerator, msdf_atlas::BitmapAtlasStorage<uint8_t, 3>> generator(packer.width, packer.height);
    generator.generate(glyphs.data(), glyphs.size());

    for (size_t i = 0; i < atlas->ConfigData.size(); i++)
    {
        auto& config = atlas->ConfigData[i];
        bool increaseSpacing = strstr(config.Name, "Seurat") != nullptr;

        auto& [index, count] = ranges[i];
        for (size_t j = 0; j < count; j++)
        {
            auto& glyph = glyphs[index + j];
            double x0, y0, x1, y1, u0, v0, u1, v1;
            glyph.getQuadPlaneBounds(x0, y0, x1, y1);
            glyph.getQuadAtlasBounds(u0, v0, u1, v1);

            double advance = glyph.getAdvance();
            if (increaseSpacing && glyph.getCodepoint() == ' ')
                advance *= 1.5;

            config.DstFont->AddGlyph(
                &config,
                glyph.getCodepoint(),
                x0, 
                -y1 + config.DstFont->Ascent, 
                x1,
                -y0 + config.DstFont->Ascent,
                u0 / packer.width, 
                v1 / packer.height, 
                u1 / packer.width, 
                v0 / packer.height, 
                advance);

            config.DstFont->AddGlyph(
                &config,
                0x200B,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f);
        }

        config.DstFont->BuildLookupTable();
    }

    atlas->TexPixelsRGBA32 = (unsigned int*)IM_ALLOC(packer.width * packer.height * 4);
    atlas->TexWidth = packer.width;
    atlas->TexHeight = packer.height;
    atlas->TexUvScale = { 1.0f / packer.width, 1.0f / packer.height };

    auto bitmapRef = (msdfgen::BitmapConstRef<uint8_t, 3>)generator.atlasStorage();
    for (int y = 0; y < packer.height; y++)
    {
        for (int x = 0; x < packer.width; x++)
        {
            auto* srcPixels = bitmapRef(x, y);
            auto* dstPixels = (uint8_t*)&atlas->TexPixelsRGBA32[y * packer.width + x];
            dstPixels[0] = srcPixels[0];
            dstPixels[1] = srcPixels[1];
            dstPixels[2] = srcPixels[2];
            dstPixels[3] = 0xFF;
        }
    }

    msdfgen::deinitializeFreetype(freeType);

    ImFontAtlasBuildFinish(atlas);

    return true;
}

ImFontBuilderIO g_fontBuilderIO = { FontBuilder_Build };
