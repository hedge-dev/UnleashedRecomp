#pragma once

#include <gpu/video.h>
#include <xdbf_wrapper.h>

extern XDBFWrapper g_xdbfWrapper;
extern std::unordered_map<uint16_t, GuestTexture*> g_xdbfTextureCache;

namespace xdbf
{
    inline std::string& FixInvalidSequences(std::string& str)
    {
        static std::array<std::string_view, 1> invalidSequences =
        {
            "\xE2\x80\x99"
        };

        static std::array<std::string_view, 1> replaceSequences =
        {
            "'"
        };

        for (int i = 0; i < invalidSequences.size(); i++)
        {
            size_t pos = 0;

            auto& invalidSeq = invalidSequences[i];
            auto& replaceSeq = replaceSequences[i];

            while ((pos = str.find(invalidSequences[i], pos)) != std::string::npos)
            {
                str = str.replace(pos, invalidSeq.length(), replaceSeq);

                pos += replaceSeq.length();
            }
        }

        return str;
    }
}
