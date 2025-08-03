#include "../../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#define g_Gamma (*(reinterpret_cast<device float3*>(g_PushConstants.SharedConstants + 0)))
#define g_TextureDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 12)))

#define g_ViewportOffset (*(reinterpret_cast<device int2*>(g_PushConstants.SharedConstants + 16)))
#define g_ViewportSize (*(reinterpret_cast<device int2*>(g_PushConstants.SharedConstants + 24)))

[[fragment]]
float4 shaderMain(float4 position [[position]],
                  constant Texture2DDescriptorHeap* g_Texture2DDescriptorHeap [[buffer(0)]],
                  constant PushConstants& g_PushConstants [[buffer(8)]])
{
    texture2d<float> texture = g_Texture2DDescriptorHeap[g_TextureDescriptorIndex].tex;
    
    int2 movedPosition = int2(position.xy) - g_ViewportOffset;
    bool boxed = any(movedPosition < 0) || any(movedPosition >= g_ViewportSize);
    if (boxed) movedPosition = 0;
    
    float4 color = boxed ? 0.0 : texture.read(uint2(movedPosition), 0);
    color.rgb = pow(color.rgb, g_Gamma);
    return color;
}
