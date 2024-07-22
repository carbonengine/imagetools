#include "StdAfx.h"
#include "ImageToolsBitmap.h"
#include "AllowThreads.h"

using namespace ImageIO;

namespace
{

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
	Vector3& normal )
{
	normal.x = ToNormalChannel( pixel[xChannel] );
	normal.y = ToNormalChannel( pixel[yChannel] );
	normal.z = sqrt( std::max( 0.f, 1.0f - normal.x * normal.x - normal.y * normal.y ) );
}

float GaussianWeight( float x, float y, float sigma )
{
    const float pi = 3.1415927f;
    float v = 2.0f * sigma * sigma;
    return exp( -( x * x + y * y ) / v ) / ( pi * v );
}

bool Is8BitFormat( PixelFormat format )
{
	switch( format )
	{
	case PIXEL_FORMAT_B8G8R8A8_UNORM:
	case PIXEL_FORMAT_B8G8R8A8_UNORM_SRGB:
	case PIXEL_FORMAT_B8G8R8X8_UNORM:
	case PIXEL_FORMAT_B8G8R8X8_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}

BlueStdResult CheckInputBitmap( const ImageToolsBitmap* bitmap, const char* mapName )
{
	if( !bitmap || !bitmap->IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, ( std::string( "invalid " ) + mapName ).c_str() );
	}
	if( bitmap->GetType() != TEX_TYPE_2D )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, ( mapName + std::string( " needs to be a 2D image" ) ).c_str() );
	}
	if( !Is8BitFormat( bitmap->GetFormat() ) )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, ( mapName + std::string( " image must be 8 bit per channel image" ) ).c_str() );
	}
	return BlueStdResult( BLUE_STD_RESULT_OK );
}

void ExtractNormals( 
	const ImageToolsBitmap* normalMap, 
	uint32_t xChannel, 
	uint32_t yChannel, 
	std::vector<Vector3>& normals )
{
	const uint32_t width = normalMap->GetWidth();
	const uint32_t height = normalMap->GetHeight();
	normals.resize( normalMap->GetWidth() * normalMap->GetHeight() );
	const uint8_t* src = reinterpret_cast<const uint8_t*>( normalMap->GetRawData() );
	const uint32_t pitch = normalMap->GetPitch();
	for( uint32_t j = 0; j < height; ++j )
	{
		for( uint32_t i = 0; i < width; ++i )
		{
			const uint8_t* pixel = src + j * pitch + i * 4;
			GetNormal( pixel, xChannel, yChannel, normals[i + j * width] );
		}
	}
}

void GetAverageNormal( 
	const std::vector<Vector3>& normals, 
	uint32_t kernelRadius, 
	uint32_t x, 
	uint32_t y, 
	uint32_t originalWidth,
	uint32_t originalHeight,
	uint32_t mipWidth, 
	uint32_t mipHeight,
	float sigma, 
	Vector3& avgNormal )
{
	avgNormal.x = avgNormal.y = avgNormal.z = 0.f;
	float weight = 0.f;
	uint32_t beginX = std::min( std::max( kernelRadius, x * originalWidth / mipWidth ) - kernelRadius, originalWidth - 1 );
	uint32_t endX = std::min( x * originalWidth / mipWidth + kernelRadius, originalWidth - 1 );
	uint32_t beginY = std::min( std::max( kernelRadius, y * originalHeight / mipHeight ) - kernelRadius, originalHeight - 1 );
	uint32_t endY = std::min( y * originalHeight / mipHeight + kernelRadius, originalHeight - 1 );
	int32_t centerX = int32_t( x * originalWidth / mipWidth );
	int32_t centerY = int32_t( y * originalHeight / mipHeight );
	for( uint32_t dj = beginY; dj <= endY; ++dj )
	{
		for( uint32_t di = beginX; di <= endX; ++di )
		{
			float w = GaussianWeight( 
				float( int32_t( di ) - centerX ) * mipWidth / originalWidth, 
				float( int32_t( dj ) - centerY ) * mipHeight / originalHeight,
				sigma );
			const Vector3& n = normals[di + dj * originalWidth];
			avgNormal.x += n.x * w;
			avgNormal.y += n.y * w;
			avgNormal.z += n.z * w;
			weight += w;
		}
	}
	avgNormal.x /= weight;
	avgNormal.y /= weight;
	avgNormal.z /= weight;
}

}


typedef Be::BlueWithStdResult<Be::Result<std::string>>::type StdOrStringResult;


StdOrStringResult FilterRoughnessChannel( 
	const ImageToolsBitmap* normalMap, 
	uint32_t xChannel, 
	uint32_t yChannel, 
	ImageToolsBitmap* destination, 
	uint32_t roughnessChannel, 
	float sigma,
	float glossFactor )
{
	AllowThreads allowThreads;

	auto result = CheckInputBitmap( normalMap, "normal map" );
	if( !BeIsSuccess( result ) )
	{
		return result;
	}
	result = CheckInputBitmap( destination, "destination map" );
	if( !BeIsSuccess( result ) )
	{
		return result;
	}
	if( destination->GetWidth() > normalMap->GetWidth() || destination->GetHeight() > normalMap->GetHeight() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "destination map size should not be larger than normal map" );
	}

	std::vector<Vector3> normals;
	ExtractNormals( normalMap, xChannel, yChannel, normals );

	const uint32_t originalWidth = normalMap->GetMipWidth( 0 );
	const uint32_t originalHeight = normalMap->GetMipHeight( 0 );

	for( uint32_t mip = 1; mip < normalMap->GetTrueMipCount(); ++mip )
	{
		const uint32_t pitch = normalMap->GetMipPitch( mip );
		uint8_t* dest = reinterpret_cast<uint8_t*>( destination->GetMipRawData( mip ) );
		const uint32_t width = normalMap->GetMipWidth( mip );
		const uint32_t height = normalMap->GetMipHeight( mip );
		if( !width || !height )
		{
			break;
		}
		uint32_t kernelRadius = std::max( originalWidth / width, originalHeight / height ) / 2;
		for( uint32_t j = 0; j < height; ++j )
		{
			for( uint32_t i = 0; i < width; ++i )
			{
				Vector3 avgNormal = { 0.f, 0.f, 0.f };
				GetAverageNormal( normals, kernelRadius, i, j, originalWidth, originalHeight, width, height, sigma, avgNormal );

				float length = Length( avgNormal );
				float originalGloss;
				if( roughnessChannel == 4 )
				{
					originalGloss = float( dest[j * pitch + i * 4] ) / 255.f;
				}
				else
				{
					originalGloss = float( dest[j * pitch + i * 4 + roughnessChannel] ) / 255.f;
				}
				float originalRoughness = std::max( 1.f - originalGloss * glossFactor, 1.f / 255.f );
				float power = 2.f / ( originalRoughness * originalRoughness ) - 1;
				float toksvig = length / ( power * ( 1 - length ) + length );
				float gloss = std::min( std::max( toksvig * originalGloss, 0.f ), 1.f );
				auto color = uint8_t( std::min( std::max( gloss * 255.f, 0.f ), 255.f ) );
				if( roughnessChannel == 4 )
				{
					dest[j * pitch + i * 4 + 0] = color;
					dest[j * pitch + i * 4 + 1] = color;
					dest[j * pitch + i * 4 + 2] = color;
					dest[j * pitch + i * 4 + 3] = color;
				}
				else
				{
					dest[j * pitch + i * 4 + roughnessChannel] = color;
				}
			}
		}
	}
	return BlueStdResult( BLUE_STD_RESULT_OK );
}

MAP_FUNCTION_AND_WRAP( 
	"filter_roughness_channel", 
	FilterRoughnessChannel, 
	"Generates normal roughness data in one of the image channels.\n"
	"Arguments:\n"
	"normal_map - image containing normal map data (needs to be 8-bit per channel)\n"
	"x_channel - channel index with normal x component in normal_map (0 is blue, 1 - green, 2 - red, 3 - alpha)\n"
	"y_channel - channel index with normal y component in normal_map (0 is blue, 1 - green, 2 - red, 3 - alpha)\n"
	"dest_map - destination roughness image\n"
	"dest_channel - channel index to output roughness data to (0 is blue, 1 - green, 2 - red, 3 - alpha, 4 - all channels)\n"
	"sigma - standard deviation for normal map distribution\n"
	"gloss_factor - additional gloss factor" );
