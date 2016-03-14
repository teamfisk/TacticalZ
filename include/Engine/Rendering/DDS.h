#ifndef DDS_h__
#define DDS_h__

#include <cstdint>

#include "../Common.h"
#include "../Core/ResourceManager.h"
#include "Image.h"

// DXT5
class DDS : public Image, public Resource
{
public:
    DDS(std::string path);
    ~DDS();

    void FlipY(unsigned char* data, std::size_t rowBytes, std::size_t totalSize, std::size_t numBlocksWide);

private:
    static const uint32_t MagicNumber = 0x20534444; // "DDS "

    struct Header_t
    {
        uint32_t Size;
        uint32_t Flags;
        uint32_t Height;
        uint32_t Width;
        uint32_t PitchOrLinearSize;
        uint32_t Depth;
        uint32_t MipMapCount;
        uint32_t RESERVED1[11];
        struct PixelFormat_t
        {
            uint32_t Size;
            uint32_t Flags;
            uint32_t FourCC;
            uint32_t RGBBitCount;
            uint32_t RBitMask;
            uint32_t GBitMask;
            uint32_t BBitMask;
            uint32_t ABitMask;
        } PixelFormat;
        uint32_t Caps;
        uint32_t Caps2;
        uint32_t Caps3;
        uint32_t Caps4;
        uint32_t RESERVED2;
    } m_Header;

    struct Block_t
    {
        struct Alpha_t
        {
            uint8_t Colors[2];
            uint8_t Data[6];
        } Alpha;
        struct Data_t
        {
            uint16_t Colors[2];
            uint8_t Data[4];
        } Data;
    };

    std::size_t m_NumBlocksWide;
    std::size_t m_NumBlocksHigh;
    std::size_t m_RowBytes;

    void flipBlock(Block_t* block);
};

#endif