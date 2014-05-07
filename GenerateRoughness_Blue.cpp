#include "StdAfx.h"
#include "ImageToolsBitmap.h"
#include "AllowThreads.h"

using namespace Tr2RenderContextEnum;

inline float Length( const Vector3& x )
{
	return sqrt( x.x * x.x + x.y * x.y + x.z * x.z );
}

inline uint32_t KernelPosition( 
	uint32_t center, 
	uint32_t kernelIndex, 
	uint32_t kernelSize, 
	uint32_t inputSize )
{
	uint32_t x = center + kernelIndex;
	uint32_t kernelRadius = kernelSize / 2;
	if( x >= kernelRadius )
	{
		x -= kernelRadius;
	}
	else
	{
		x = 0;
	}
	if( x >= inputSize )
	{
		--x;
	}
	return std::min( x, inputSize - 1 );
}

inline float ToNormalChannel( uint8_t pixelValue )
{
	return float( pixelValue ) / 255.0f * 2.0f - 1.0f;
}

inline void SafeNormalize( Vector3& normal )
{
	float length = Length( normal );
	float norm = length == 0 ? 1.f : 1.f / length;
	normal.x *= norm;
	normal.y *= norm;
	normal.z *= norm;
}

inline void GetNormal( 
	const uint8_t* pixel, 
	uint32_t xChannel, 
	uint32_t yChannel, 
	uint32_t zChannel, 
	Vector3& normal )
{
	normal.x = ToNormalChannel( pixel[xChannel] );
	normal.y = ToNormalChannel( pixel[yChannel] );
	if( zChannel < 4 )
	{
		normal.z = ToNormalChannel( pixel[zChannel] );
	}
	else
	{
		normal.z = sqrt( 1.0f - normal.x * normal.x - normal.y * normal.y );
	}
	SafeNormalize( normal );
}

Be::Result<std::string> GenerateRoughnessChannel( 
	const ImageToolsBitmap* normalMap, 
	uint32_t xChannel, 
	uint32_t yChannel, 
	uint32_t zChannel, 
	ImageToolsBitmap* destination, 
	uint32_t roughnessChannel )
{
	AllowThreads allowThreads;

	if( !normalMap || !normalMap->IsValid() )
	{
		return "invalid normal map";
	}
	if( !destination )
	{
		return "invalid destination map";
	}
	if( normalMap->GetType() != TEX_TYPE_2D )
	{
		return "normal map needs to be a 2D image";
	}
	switch( normalMap->GetFormat() )
	{
	case PIXEL_FORMAT_B8G8R8A8_UNORM:
	case PIXEL_FORMAT_B8G8R8A8_UNORM_SRGB:
	case PIXEL_FORMAT_B8G8R8X8_UNORM:
	case PIXEL_FORMAT_B8G8R8X8_UNORM_SRGB:
		break;
	default:
		return "normal map image must be 8 bit per channel image";
	}

	if( destination->GetWidth() != normalMap->GetWidth() || 
		destination->GetHeight() != normalMap->GetHeight() || 
		destination->GetTrueMipCount() != normalMap->GetTrueMipCount() || 
		destination->GetFormat() != normalMap->GetFormat() ||
		destination->GetType() != normalMap->GetType() )
	{
		return "destination map dimensions don't match normal map dimensions";
	}

	const uint32_t kernelSize = 3;

	for( uint32_t mip = 0; mip < normalMap->GetTrueMipCount(); ++mip )
	{
		const uint8_t* src = reinterpret_cast<const uint8_t*>( normalMap->GetMipRawData( mip ) );
		const uint32_t pitch = normalMap->GetMipPitch( mip );
		uint8_t* dest = reinterpret_cast<uint8_t*>( destination->GetMipRawData( mip ) );
		const uint32_t width = normalMap->GetMipWidth( mip );
		const uint32_t height = normalMap->GetMipHeight( mip );
		for( uint32_t j = 0; j < height; ++j )
		{
			for( uint32_t i = 0; i < width; ++i )
			{
				Vector3 normals = { 0.f, 0.f, 0.f };
				for( uint32_t dj = 0; dj < kernelSize; ++dj )
				{
					uint32_t y = KernelPosition( j, dj, kernelSize, height );
					for( int32_t di = 0; di < kernelSize; ++di )
					{
						uint32_t x = KernelPosition( i, di, kernelSize, width );

						const uint8_t* pixel = src + y * pitch + x * 4;
						Vector3 n;
						GetNormal( pixel, xChannel, yChannel, zChannel, n );
						normals.x += n.x;
						normals.y += n.y;
						normals.z += n.z;
					}
				}
				float length = Length( normals ) / 9;
				float roughness = 1 - length;
				// we clamp roughness at 254 to avoid checking for INF in shaders
				dest[j * pitch + i * 4 + roughnessChannel] = uint8_t( std::min( std::max( roughness * 255.f, 0.f ), 254.f ) );
			}
		}
	}
	return "";
}

MAP_FUNCTION_AND_WRAP( 
	"generate_roughness_channel", 
	GenerateRoughnessChannel, 
	"Generates normal roughness data in one of the image channels.\n"
	"Arguments:\n"
	"normal_map - image containing normal map data (needs to be 8-bit per channel)\n"
	"x_channel - channel index with normal x component in normal_map (0 is blue, 1 - green, 2 - red, 3 - alpha)\n"
	"y_channel - channel index with normal y component in normal_map (0 is blue, 1 - green, 2 - red, 3 - alpha)\n"
	"z_channel - optional channel index with normal z component in normal_map (0 is blue, 1 - green, 2 - red, 3 - "
	"alpha, 4 - generate z from x and y)\n"
	"dest_map - destination image, must be of the same size and format as the normal_map (can be the same bitmap as "
	"the normal_map as long as output channel is not used as input)\n"
	"dest_channel - channel index to output roughness data to (0 is blue, 1 - green, 2 - red, 3 - alpha)" );
