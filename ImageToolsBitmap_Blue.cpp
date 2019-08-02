////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "ImageToolsBitmap.h"
#include "CompressionOptions.h"

BLUE_DEFINE( ImageToolsBitmap );

#if BLUE_WITH_PYTHON
PyObject* PyGetPixelData( PyObject* self, PyObject* args )
{
	ImageToolsBitmap* pThis = BluePythonCast<ImageToolsBitmap*>( self );
	return PyString_FromStringAndSize( pThis->GetRawData(), pThis->GetRawDataSize() );
}
#endif

const Be::ClassInfo* ImageToolsBitmap::ExposeToBlue()
{
    EXPOSURE_BEGIN( ImageToolsBitmap, "" )

		MAP_INTERFACE( IRoot )
		MAP_INTERFACE( ImageToolsBitmap )

		MAP_PROPERTY_READONLY( "type", GetType, "bitmap type (member of imagetools.BITMAP_TYPE)" );
		MAP_PROPERTY_READONLY( "width", GetWidth, "bitmap width in pixels" );
		MAP_PROPERTY_READONLY( "height", GetHeight, "bitmap height in pixels" );
		MAP_PROPERTY_READONLY( "depth", GetDepth, "bitmap height in pixels (for volume bitmaps)" );
		MAP_PROPERTY_READONLY( "mip_count", GetTrueMipCount, "number of mip levels" );
		MAP_PROPERTY_READONLY( "format", GetFormat, "bitmap pixel format (member of imagetools.PIXEL_FORMAT)" );
		MAP_PROPERTY_READONLY( "array_size", GetArraySize, "texture array size" );

		MAP_METHOD_AND_WRAP( 
			"get_mip_width", 
			GetMipWidth,
			"Returns mip level width\n"
			"Arguments:\n"
			"mip_index - mip level index" );
		MAP_METHOD_AND_WRAP( 
			"get_mip_height", 
			GetMipHeight,
			"Returns mip level height\n"
			"Arguments:\n"
			"mip_index - mip level index" );
		MAP_METHOD_AND_WRAP( 
			"get_mip_depth", 
			GetMipDepth,
			"Returns mip level depth for volume bitmaps\n"
			"Arguments:\n"
			"mip_index - mip level index" );
		MAP_METHOD_AND_WRAP( 
			"save", 
			Save, 
			"Saves bitmap into a file\n"
			"Arguments:\n"
			"filename - path to file" );
		MAP_METHOD_AND_WRAP( "downsample2x2", CheckedDownsample2x2, "Downsamples the bitmap by factor of 2" );
		MAP_METHOD_AND_WRAP( 
			"crop", 
			CheckedCrop, 
			"Crops 2D bitmap\n"
			"Arguments:\n"
			"left - pixel offset to the left of the crop window\n"
			"top - pixel offset to the top of the crop window\n"
			"bottom - pixel offset to the bottom of the crop window\n"
			"right - pixel offset to the right of the crop window" );
		MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( 
			"generate_mips", 
			CheckedGenerateMipMaps,
			1,
			"Generates mip levels for 2D bitmap\n"
			"Arguments:\n"
			"levels - optional amount of mip levels to generate" );
		MAP_METHOD_AND_WRAP(
			"drop_mips",
			DropMipMaps,
			"Removes all mips from the image. This function always returns true." );
		MAP_METHOD_AND_WRAP( 
			"convert_format", 
			CheckedConvertFormat, 
			"Converts bitmap pixel format. Only some transitions are supported:\n"
			"B8G8R8X8_UNORM -> B8G8R8A8_UNORM\n"
			"R8_UNORM -> B8G8R8A8_UNORM\n"
			"R8G8_UNORM -> B8G8R8A8_UNORM\n"
			"Arguments:\n"
			"format - new format" );
		MAP_METHOD_AND_WRAP
		(
			"copy_channel",
			CheckedCopyChannel,
			"Copies a selected color channel from another bitmap to a color channel of this bitmap"
			"Arguments:\n"
			"srcChannel - the index of the color channel to copy\n"
			"dstChannel - the index of the destination color channel which should be overwritten\n"
			"source - the source bitmap from which to copy the channel from"
		);
		MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( 
			"compress", 
			Compress, 
			1, 
			"Compresses the bitmap. Returns compressed bitmap.\n"
			"Arguments:\n"
			"options - (optional) compression options (imagetools.CompressionOptions)" );
		MAP_METHOD_AND_WRAP( 
			"is_compressed", 
			IsCompressed, 
			"Returns True if the image uses a compressed format." );
		MAP_METHOD_AND_WRAP( 
			"copy", 
			Copy, 
			"Returns a copy if this image." );
		MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( 
			"extract_mip_level", 
			ExtractMipLevel, 
			1,
			"Returns a copy if this image mip level as a separate image.\n"
			"Arguments:\n"
			"mip_level - mip level index\n"
			"count - optional number of mip levels to copy, defaults to one\n"
		);
		MAP_METHOD_AND_WRAP
		(
			"rotate_face_clockwise",
			RotateFaceClockwise,
			"Rotates a face clockwise a multiple of 90 degrees\n"
			":param face: bitmap face (for cubemaps)\n"
			":param times: number of times to rotate"
		)
		MAP_METHOD_AND_WRAP
		(
			"convert_crossmap_to_cubemap",
			ConvertCrossmapToCubemap,
			"Converts a 3x4 2D crossmap into a cubemap"
		)
		MAP_METHOD_AND_WRAP( 
			"flatten", 
			FlattenSlices, 
			"Returns a 2D image with all slices (cube faces) copied to it in a horizontal or vertical strip.\n"
			"Arguments:\n"
			"horizontally - copy slices in horizontal or vertical strip" );
		MAP_METHOD_AND_WRAP( 
			"yuv", 
			ToYuv, 
			"Returns a (y, uv) tuple of YUV422 channels of the bitmap." );
		MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( 
			"set_mip_level_data", 
			SetMipData, 
			1,
			"Copies mip pixels from source image to this image.\n"
			"Arguments:\n"
			"mip_level - destination mip level index\n"
			"source - source image\n"
			"source_mip - (optional) source image mip level, defaults to 0" );
		MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( 
			"compress_to_file", 
			CompressToFile, 
			1, 
			"Compresses the bitmap and writes compressed image into a file.\n"
			"Arguments:\n"
			"filename - path to output dds file\n"
			"options - (optional) compression options (imagetools.CompressionOptions)" );
#if BLUE_WITH_PYTHON
		MAP_METHOD(
			"get_pixel_data",
			PyGetPixelData,
			"Returns pixel data of the bitmap as a string." );
#endif
	EXPOSURE_END()
}

namespace
{

StdOrImageIOResult LoadHostBitmap( const wchar_t* filename, ImageToolsBitmapPtr& result )
{
	result.CreateInstance();
	auto ret = result->Load( filename );
	if( !BeIsSuccess( ret ) )
	{
		result = nullptr;
	}
	return ret;
}

StdOrImageIOResult CreateFromArray( const std::vector<ImageToolsBitmap*>& elements, ImageToolsBitmapPtr& result )
{
	result.CreateInstance();
	auto ret = result->CreateFromArray( elements );
	if( !BeIsSuccess( ret ) )
	{
		result = nullptr;
	}
	return ret;
}

Be::Result<std::string> CreateBitmap2D( 
	uint32_t width, 
	uint32_t height, 
	uint32_t mipCount, 
	Tr2RenderContextEnum::PixelFormat format, 
	ImageToolsBitmapPtr& result )
{
	result.CreateInstance();
	if( !result->Create( width, height, mipCount, format ) )
	{
		result = nullptr;
		return Be::Result<std::string>( "error creating 2D bitmap" );
	}
	return std::string();
}

Be::Result<std::string> CreateBitmap2DFromString( 
	uint32_t width, 
	uint32_t height, 
	uint32_t mipCount, 
	Tr2RenderContextEnum::PixelFormat format, 
	const char* pixelData, 
	ImageToolsBitmapPtr& result )
{
	result.CreateInstance();
	if( !result->Create( width, height, mipCount, format ) )
	{
		result = nullptr;
		return Be::Result<std::string>( "error creating 2D bitmap" );
	}
	memcpy( result->GetRawData(), pixelData, result->GetRawDataSize() );
	return std::string();
}

Be::Result<std::string> CreateBitmapCube( 
	uint32_t width, 
	uint32_t mipCount, 
	Tr2RenderContextEnum::PixelFormat format, 
	ImageToolsBitmapPtr& result )
{
	result.CreateInstance();
	if( !result->CreateCube( width, mipCount, format ) )
	{
		result = nullptr;
		return Be::Result<std::string>( "error creating cube bitmap" );
	}
	return std::string();
}

Be::Result<std::string> CreateBitmapVolume( 
	uint32_t width, 
	uint32_t height, 
	uint32_t depth, 
	uint32_t mipCount, 
	Tr2RenderContextEnum::PixelFormat format, 
	ImageToolsBitmapPtr& result )
{
	result.CreateInstance();
	if( !result->CreateVolume( width, height, depth, mipCount, format ) )
	{
		result = nullptr;
		return Be::Result<std::string>( "error creating volume bitmap" );
	}
	return std::string();
}

bool IsCudaAvailable()
{
	nvtt::Compressor compressor;
	return compressor.isCudaAccelerationEnabled();
}

}

MAP_FUNCTION_AND_WRAP( 
	"load", 
	LoadHostBitmap, 
	"Loads bitmap from file.\n"
	"Arguments:\n"
	"filename - path to image file" );

MAP_FUNCTION_AND_WRAP( 
	"create_from_array", 
	CreateFromArray, 
	"Creates an array bitmap from the given list of individual bitmaps. All bitmaps\n"
	"in the list must be valid 2D bitmaps with the same sizes and formats.\n"
	"Arguments:\n"
	"elements - list of array element bitmaps" );

MAP_FUNCTION_AND_WRAP( 
	"create_2d", 
	CreateBitmap2D, 
	"Creates 2D bitmap.\n"
	"Arguments:\n"
	"width - bitmap width in pixels\n"
	"height - bitmap height in pixels\n"
	"mip_count - number of mip levels (pass 0 to create all)\n"
	"format - bitmap pixel format (imagetools.PIXEL_FORMAT)" );

MAP_FUNCTION_AND_WRAP( 
	"create_2d_from_string", 
	CreateBitmap2DFromString, 
	"Creates 2D bitmap and initializes its pixel data with data\n"
	"provided in a string.\n"
	"Arguments:\n"
	"width - bitmap width in pixels\n"
	"height - bitmap height in pixels\n"
	"mip_count - number of mip levels (pass 0 to create all)\n"
	"format - bitmap pixel format (imagetools.PIXEL_FORMAT)\n"
	"pixel_data - pixel data as a string\n" );

MAP_FUNCTION_AND_WRAP( 
	"create_cube", 
	CreateBitmapCube, 
	"Creates cube bitmap.\n"
	"Arguments:\n"
	"width - bitmap width/height in pixels\n"
	"mip_count - number of mip levels (pass 0 to create all)\n"
	"format - bitmap pixel format (imagetools.PIXEL_FORMAT)" );

MAP_FUNCTION_AND_WRAP( 
	"create_volume", 
	CreateBitmapVolume, 
	"Creates volume bitmap.\n"
	"Arguments:\n"
	"width - bitmap width in pixels\n"
	"height - bitmap height in pixels\n"
	"depth - bitmap depth in pixels\n"
	"mip_count - number of mip levels (pass 0 to create all)\n"
	"format - bitmap pixel format (imagetools.PIXEL_FORMAT)" );

MAP_FUNCTION_AND_WRAP( 
	"is_cuda_available", 
	IsCudaAvailable, 
	"Checks if CUDA is available in the system. DDS compression works\n"
	"faster on systems with CUDA. Returns True if CUDA is available.\n" );
