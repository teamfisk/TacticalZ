#ifndef Image_h
#define Image_h

struct Image
{
    virtual ~Image() = default;

	enum class ImageFormat
	{
		Unknown,
		RGB,
		RGBA
	};

	unsigned int Width = 0;
	unsigned int Height = 0;
	ImageFormat Format = ImageFormat::Unknown;
    unsigned int MipMapLevels = 0;
    bool Compressed = false;
	unsigned char* Data = nullptr;
    std::size_t ByteSize = 0;
};

#endif
