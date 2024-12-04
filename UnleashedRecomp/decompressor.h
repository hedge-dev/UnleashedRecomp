#pragma once

template<size_t N>
static std::unique_ptr<uint8_t[]> decompressZstd(const uint8_t(&data)[N], size_t decompressedSize)
{
    auto decompressedData = std::make_unique<uint8_t[]>(decompressedSize);
    ZSTD_decompress(decompressedData.get(), decompressedSize, data, N);
    return decompressedData;
}
