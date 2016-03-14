#include "Rendering/DDS.h"

DDS::DDS(std::string path)
{
    std::ifstream in(path.c_str(), std::ios_base::binary | std::ios_base::ate);
    if (!in.is_open()) {
        throw Resource::FailedLoadingException("Failed to open file.");
    }
    unsigned int fileSize = static_cast<unsigned int>(in.tellg());
    in.seekg(0, std::ios_base::beg);

    if (fileSize < sizeof(MagicNumber) + sizeof(DDS::Header_t)) {
        throw Resource::FailedLoadingException("Invalid DDS file; not even big enough to contain the DDS header!");
    }

    // Read magic number
    uint32_t magicNumber;
    in.read(reinterpret_cast<char*>(&magicNumber), sizeof(MagicNumber));
    if (magicNumber != DDS::MagicNumber) {
        throw Resource::FailedLoadingException("Invalid DDS file; magic number doesn't match.");
    }

    // Read header
    in.read(reinterpret_cast<char*>(&m_Header), sizeof(DDS::Header_t));

    // Validate header
    if (m_Header.Size != sizeof(DDS::Header_t) || m_Header.PixelFormat.Size != sizeof(DDS::Header_t::PixelFormat_t)) {
        throw Resource::FailedLoadingException("Corrupt DDS file; header size doesn't match!");
    }

    // Make sure it's DC3 (DXT5)
    char format[5];
    memcpy(&format, &m_Header.PixelFormat.FourCC, 4);
    format[4] = '\0';
    if (strcmp(format, "DXT5")) {
        std::stringstream ss;
        ss << "Invalid DDS file; Unsupported compression format \"" << format << "\"";
        throw Resource::FailedLoadingException(ss.str().c_str());
    }

    this->Width = m_Header.Width;
    this->Height = m_Header.Height;
    this->Format = Image::ImageFormat::RGBA;
    this->Compressed = true;
    if (m_Header.MipMapCount != 0) {
        this->MipMapLevels = m_Header.MipMapCount - 1;
    }

    // Read data
    std::size_t dataSize = fileSize - sizeof(MagicNumber) - sizeof(DDS::Header_t);
    this->Data = new unsigned char[dataSize];
    this->ByteSize = dataSize;
    in.read(reinterpret_cast<char*>(this->Data), dataSize);

    // Flip!
    unsigned int w = this->Width;
    unsigned int h = this->Height;
    unsigned char* mipMapData = this->Data;
    for (unsigned int i = 0; i < this->MipMapLevels; ++i) {
        std::size_t bpe = 16;
        std::size_t numBlocksWide = std::max<std::size_t>(1, (w + 3) / 4);
        std::size_t numBlocksHigh = std::max<std::size_t>(1, (h + 3) / 4);
        std::size_t rowBytes = numBlocksWide * bpe;
        std::size_t numRows = numBlocksHigh;
        std::size_t numBytes = rowBytes * numBlocksHigh;
        FlipY(mipMapData, rowBytes, numBytes, numBlocksWide);
        w = w >> 1;
        h = h >> 1;
        mipMapData += numBytes;
    } 
}

DDS::~DDS()
{
    if (this->Data != nullptr) {
        delete[] this->Data;
        this->Data = nullptr;
    }
}

void DDS::FlipY(unsigned char* data, std::size_t rowBytes, std::size_t totalSize, std::size_t numBlocksWide)
{
    std::size_t height = totalSize / rowBytes;
    unsigned char* topLine = data;
    unsigned char* bottomLine = data + (height - 1) * rowBytes;
    for (std::size_t line = 0; line < height / 2; ++line) {
        Block_t* topBlocks = reinterpret_cast<Block_t*>(topLine);
        Block_t* bottomBlocks = reinterpret_cast<Block_t*>(bottomLine);
        for (std::size_t block = 0; block < numBlocksWide; ++block) {
            flipBlock(&topBlocks[block]);
            flipBlock(&bottomBlocks[block]);
            std::swap(topBlocks[block], bottomBlocks[block]);
        }
        topLine += rowBytes;
        bottomLine -= rowBytes;
    }
}

void DDS::flipBlock(Block_t* block)
{
    // Extract both rows of pixel Data into a 32 bit sized values.
    uint32_t pixelRow0 = block->Alpha.Data[0] + ((block->Alpha.Data[1] + (block->Alpha.Data[2] << 8)) << 8);
    uint32_t pixelRow1 = block->Alpha.Data[3] + ((block->Alpha.Data[4] + (block->Alpha.Data[5] << 8)) << 8);

    // Swap the extracted row Data.  These values will be read from and shifted into the final block locations.
    uint32_t pixelRow0Swapped = ((pixelRow0 & 0x000fff) << 12) | ((pixelRow0 & 0xfff000) >> 12);
    uint32_t pixelRow1Swapped = ((pixelRow1 & 0x000fff) << 12) | ((pixelRow1 & 0xfff000) >> 12);

    // Swapped Data from row 1 is written
    block->Alpha.Data[0] = static_cast<uint8_t>(pixelRow1Swapped  & 0x0000ff);
    block->Alpha.Data[1] = static_cast<uint8_t>((pixelRow1Swapped & 0x00ff00) >> 8);
    block->Alpha.Data[2] = static_cast<uint8_t>((pixelRow1Swapped & 0xff0000) >> 16);

    // Swapped Data from row 0 is written
    block->Alpha.Data[3] = static_cast<uint8_t>(pixelRow0Swapped  & 0x0000ff);
    block->Alpha.Data[4] = static_cast<uint8_t>((pixelRow0Swapped & 0x00ff00) >> 8);
    block->Alpha.Data[5] = static_cast<uint8_t>((pixelRow0Swapped & 0xff0000) >> 16);

    std::swap(block->Data.Data[0], block->Data.Data[3]);
    std::swap(block->Data.Data[1], block->Data.Data[2]);
}
