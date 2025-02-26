#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <fstream>
#include <cassert>
#include <mspack.h>
#include <lzx.h>

static std::vector<uint8_t> readAllBytes(const char* path)
{
    std::ifstream file{ path, std::ios::binary };
    std::vector<uint8_t> result{};

    if (!file.good()) {
        return result;
    }

    file.seekg(0, std::ios::end);
    result.resize(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(result.data()), result.size());

    return result;
}

template<typename T>
static T byteSwap(T value)
{
    if constexpr (sizeof(T) == 1)
        return value;
    else if constexpr (sizeof(T) == 2)
        return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(value)));
    else if constexpr (sizeof(T) == 4)
        return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(value)));
    else if constexpr (sizeof(T) == 8)
        return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(value)));

    assert(false && "Unexpected byte size.");
    return value;
}

template<typename T>
static void byteSwapInplace(T& value)
{
    value = byteSwap(value);
}

struct ReadStream
{
    const uint8_t* data = nullptr;
    int size = 0; // Size from every compressed block.
};

static int mspackRead(mspack_file* file, void* buffer, int bytes)
{
    ReadStream* stream = reinterpret_cast<ReadStream*>(file);

    if (stream->size == 0)
    {
        uint16_t size = byteSwap(*reinterpret_cast<const uint16_t*>(stream->data));
        stream->data += sizeof(uint16_t);

        // This indicates there is an uncompressed block size available. We don't need it so we skip it.
        if ((size & 0xFF00) == 0xFF00)
        {
            stream->data += 1;
            size = byteSwap(*reinterpret_cast<const uint16_t*>(stream->data));
            stream->data += sizeof(uint16_t);
        }

        stream->size = size;
    }

    int sizeToRead = std::min(stream->size, bytes);

    memcpy(buffer, stream->data, sizeToRead);
    stream->data += sizeToRead;
    stream->size -= sizeToRead;

    return sizeToRead;
}

struct WriteStream
{
    uint8_t* data = nullptr;
    std::size_t size = 0; // Remaining available space in the stream.
};

static int mspackWrite(mspack_file* file, void* buffer, int bytes)
{
    WriteStream* stream = reinterpret_cast<WriteStream*>(file);

    std::size_t sizeToWrite = std::min(stream->size, static_cast<std::size_t>(bytes));

    memcpy(stream->data, buffer, sizeToWrite);
    stream->data += sizeToWrite;
    stream->size -= sizeToWrite;

    return static_cast<int>(sizeToWrite);
}

static void* mspackAlloc(mspack_system* self, size_t bytes)
{
    return operator new(bytes);
}

static void mspackFree(void* ptr)
{
    operator delete(ptr);
}

static void mspackCopy(void* src, void* dst, size_t bytes)
{
    memcpy(dst, src, bytes);
}

static mspack_system g_lzxSystem =
{
    nullptr,
    nullptr,
    mspackRead,
    mspackWrite,
    nullptr,
    nullptr,
    nullptr,
    mspackAlloc,
    mspackFree,
    mspackCopy
};

// Xbox Compression header definitions.
static constexpr uint32_t XCompressSignature = 0xFF512EE;

struct XCompressHeader
{
    uint32_t signature;
    uint32_t field04;
    uint32_t field08;
    uint32_t field0C;
    uint32_t windowSize;
    uint32_t compressedBlockSize;
    uint64_t uncompressedSize;
    uint64_t compressedSize;
    uint32_t uncompressedBlockSize;
    uint32_t field2C;

    void byteSwap()
    {
        byteSwapInplace(signature);
        byteSwapInplace(field04);
        byteSwapInplace(field08);
        byteSwapInplace(field0C);
        byteSwapInplace(windowSize);
        byteSwapInplace(compressedBlockSize);
        byteSwapInplace(uncompressedSize);
        byteSwapInplace(compressedSize);
        byteSwapInplace(uncompressedBlockSize);
        byteSwapInplace(field2C);
    }
};

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Usage: x_decompress [input file path] [output file path]");
        return EXIT_SUCCESS;
    }

    std::vector<uint8_t> file = readAllBytes(argv[1]);
    if (file.empty())
    {
        fprintf(stderr, "Input file \"%s\" not found or empty", argv[1]);
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> decompressedFile;

    if (file.size() >= sizeof(XCompressHeader) && byteSwap(*reinterpret_cast<uint32_t*>(file.data())) == XCompressSignature)
    {
        XCompressHeader* header = reinterpret_cast<XCompressHeader*>(file.data());
        header->byteSwap();

        decompressedFile.resize(header->uncompressedSize);

        const uint8_t* srcBytes = file.data() + sizeof(XCompressHeader);

        WriteStream dstStream;
        dstStream.data = decompressedFile.data();
        dstStream.size = decompressedFile.size();

        // libmspack wants the bit index. This value is always guaranteed to be a power of two,
        // so we can extract the bit index by counting the amount of leading zeroes.
        int windowBits = 0;
        uint32_t windowSize = header->windowSize;
        while ((windowSize & 0x1) == 0)
        {
            ++windowBits;
            windowSize >>= 1;
        }

        // Loop over compressed blocks.
        while (srcBytes < (file.data() + file.size()) && dstStream.data < (decompressedFile.data() + decompressedFile.size()))
        {
            uint32_t compressedSize = byteSwap(*reinterpret_cast<const uint32_t*>(srcBytes));
            srcBytes += sizeof(uint32_t);

            ReadStream srcStream;
            srcStream.data = srcBytes;

            std::size_t uncompressedBlockSize = std::min(static_cast<std::size_t>(header->uncompressedBlockSize), dstStream.size);

            lzxd_stream* lzx = lzxd_init(
                &g_lzxSystem,
                reinterpret_cast<mspack_file*>(&srcStream),
                reinterpret_cast<mspack_file*>(&dstStream),
                windowBits,
                0,
                static_cast<int>(header->compressedBlockSize),
                static_cast<off_t>(uncompressedBlockSize),
                0);

            lzxd_decompress(lzx, uncompressedBlockSize);
            lzxd_free(lzx);

            srcBytes += compressedSize;
        }
    }
    else
    {
        decompressedFile = std::move(file);
    }

    std::ofstream outputFile(argv[2], std::ios::binary);
    if (!outputFile.good())
    {
        fprintf(stderr, "Cannot open output file \"%s\" for writing", argv[2]);
        return EXIT_FAILURE;
    }

    outputFile.write(reinterpret_cast<const char*>(decompressedFile.data()), decompressedFile.size());

    return EXIT_SUCCESS;
}
