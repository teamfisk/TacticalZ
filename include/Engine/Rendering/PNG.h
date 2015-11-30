#ifndef PNG_h_
#define PNG_h_

#include <stdexcept>

#include <png.h>

#include "../Common.h"
#include "Image.h"

class PNG : public Image
{
public:
	PNG(std::string path);
	~PNG();

private:
	static void pngErrorFunction(png_structp png_ptr, png_const_charp error_msg);
	static void pngWarningFunction(png_structp png_ptr, png_const_charp warning_msg);
};

#endif
