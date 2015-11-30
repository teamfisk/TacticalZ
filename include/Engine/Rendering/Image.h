#ifndef Image_h
#define Image_h

struct Image
{
	enum class ImageFormat
	{
		Unknown,
		RGB,
		RGBA
	};

	unsigned int Width = 0;
	unsigned int Height = 0;
	ImageFormat Format = ImageFormat::Unknown;
	unsigned char* Data = nullptr;
};

#endif
