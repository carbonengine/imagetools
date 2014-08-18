////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "ImageToolsBitmap.h"
#include "FileStream.h"
#include "CompressionOptions.h"
#include "MemoryOutputHandler.h"
#include "MemoryStream.h"
#include "AllowThreads.h"

using namespace Tr2RenderContextEnum;

#define CBR_RETURN_BR( x ) { auto ret = x; if( !Be::IsSuccess( ret ) ) { return ret; } }

ImageToolsBitmap::ImageToolsBitmap( IRoot* lockobj )
{
}

StdOrImageIOResult ImageToolsBitmap::Load( const wchar_t* filename )
{
	AllowThreads allowThreads;

	FileStream stream( filename, FileStream::READ );
	if( !stream.IsValid() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_IO_ERROR, "failed to open file" );
	}

	return ImageIOResult( ImageIO::ReadImage( stream, ImageIO::LoadParameters( filename ), *this ) );
}

StdOrImageIOResult ImageToolsBitmap::Save( const wchar_t* filename )
{
	AllowThreads allowThreads;

	if( !IsValid() )
	{
		return ImageIO::Result( ImageIO::Result::INVALID_BITMAP, "cannot save invalid bitmap" );
	}

	FileStream stream( filename, FileStream::WRITE );
	if( !stream.IsValid() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_IO_ERROR, "could not open file for saving" );
	}

	return ImageIO::SaveImage( filename, *this, stream );
}

Be::Result<std::string> ImageToolsBitmap::CheckedDownsample2x2()
{
	AllowThreads allowThreads;

	if( !Downsample2x2() )
	{
		return "error while downsampling the bitmap";
	}
	return std::string();
}

Be::Result<std::string> ImageToolsBitmap::CheckedCrop( unsigned left, unsigned top, unsigned right, unsigned bottom )
{
	if( !Crop( left, top, right, bottom ) )
	{
		return "error while cropping the bitmap";
	}
	return std::string();
}

Be::Result<std::string> ImageToolsBitmap::CheckedGenerateMipMaps()
{
	AllowThreads allowThreads;

	if( !GenerateMipMaps() )
	{
		return "error while generating mip levels";
	}
	return std::string();
}

Be::BlueStdResult ImageToolsBitmap::CheckedConvertFormat( Tr2RenderContextEnum::PixelFormat format )
{
	AllowThreads allowThreads;

	if( IsCompressed() && ( format == PIXEL_FORMAT_B8G8R8A8_UNORM || format == PIXEL_FORMAT_B8G8R8X8_UNORM ) )
	{
		return Decompress( format );
	}
	if( !ConvertFormat( format ) )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_RUNTIME_ERROR, "error converting pixel format" );
	}
	return Be::BLUE_STD_RESULT_OK;
}

Be::BlueStdResult ImageToolsBitmap::Decompress( Tr2RenderContextEnum::PixelFormat format )
{
	if( !IsCompressed() || GetType() != TEX_TYPE_2D )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "cannot decompress an uncompressed image" );
	}
	nvtt::Surface surface;
	nvtt::Format nvttFormat;
	size_t channels = 4;
	switch( m_format )
	{
	case PIXEL_FORMAT_BC1_TYPELESS:
	case PIXEL_FORMAT_BC1_UNORM:
	case PIXEL_FORMAT_BC1_UNORM_SRGB:
		nvttFormat = nvtt::Format_BC1;
		break;
	case PIXEL_FORMAT_BC2_TYPELESS:
	case PIXEL_FORMAT_BC2_UNORM:
	case PIXEL_FORMAT_BC2_UNORM_SRGB:
		nvttFormat = nvtt::Format_BC2;
		break;
	case PIXEL_FORMAT_BC3_TYPELESS:
	case PIXEL_FORMAT_BC3_UNORM:
	case PIXEL_FORMAT_BC3_UNORM_SRGB:
		nvttFormat = nvtt::Format_BC3;
		break;
	case PIXEL_FORMAT_BC4_TYPELESS:
	case PIXEL_FORMAT_BC4_UNORM:
	case PIXEL_FORMAT_BC4_SNORM:
		nvttFormat = nvtt::Format_BC4;
		channels = 1;
		break;
	case PIXEL_FORMAT_BC5_TYPELESS:
	case PIXEL_FORMAT_BC5_UNORM:
	case PIXEL_FORMAT_BC5_SNORM:
		nvttFormat = nvtt::Format_BC5;
		channels = 2;
		break;
	case PIXEL_FORMAT_BC6H_TYPELESS:
	case PIXEL_FORMAT_BC6H_UF16:
	case PIXEL_FORMAT_BC6H_SF16:
		nvttFormat = nvtt::Format_BC6;
		break;
	case PIXEL_FORMAT_BC7_TYPELESS:
	case PIXEL_FORMAT_BC7_UNORM:
	case PIXEL_FORMAT_BC7_UNORM_SRGB:
		nvttFormat = nvtt::Format_BC7;
		break;
	}
	const size_t bpp = 4;
	const size_t pixelCount = GetRawDataSize() * 16 / GetBlockByteSize( m_format );
	const size_t newSize = pixelCount * bpp;
	CcpMallocBuffer data( "HostBitmap::m_data", newSize );
	size_t mipStart = 0;

	for( uint32_t mip = 0; mip < GetTrueMipCount(); ++mip )
	{
		if( !surface.setImage2D( nvttFormat, nvtt::Decoder_D3D10, GetMipWidth( mip ), GetMipHeight( mip ), GetMipRawData( mip ) ) )
		{
			Destroy();
			return Be::BlueStdResult( Be::BLUE_STD_RESULT_RUNTIME_ERROR, "could not decompress image" );
		}
	
		const size_t mipSize = GetMipWidth( mip ) * GetMipHeight( mip );
		for( size_t channel = 0; channel < 4; ++channel )
		{
			static const int channelMap[] = { 2, 1, 0, 3 };
			const float* src = surface.channel( channelMap[channel] );
			uint8_t* dest = reinterpret_cast<uint8_t*>( data.get() ) + mipStart + channel;
			for( size_t i = 0; i < mipSize; ++i )
			{
				*dest = uint8_t( std::max( std::min( int( *src++ * 255.f + 0.5f ), 255 ), 0 ) );
				dest += bpp;
			}
		}
		mipStart += mipSize * bpp;
	}
	m_data.swap( data );
	m_format = format;
	return Be::BLUE_STD_RESULT_OK;
}

Be::BlueStdResult ImageToolsBitmap::Copy( ImageToolsBitmapPtr& result ) const
{
	result.CreateInstance();
	if( !result )
	{
		return Be::BLUE_STD_RESULT_MEMORY_ERROR;
	}
	static_cast<Tr2BitmapDimensions&>( *result ) = *this;
	result->m_name = m_name;
	result->m_data.resize( "HostBitmap::m_data", m_data.size() );
	if( !result->m_data.get() )
	{
		result = nullptr;
		return Be::BLUE_STD_RESULT_MEMORY_ERROR;
	}
	memcpy( result->m_data.get(), m_data.get(), m_data.size() );
	return Be::BLUE_STD_RESULT_OK;
}

Be::BlueStdResult ImageToolsBitmap::ExtractMipLevel( uint32_t mipLevel, ImageToolsBitmapPtr& result ) const
{
	if( !IsValid() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "source bitmap is invalid" );
	}
	if( mipLevel >= GetTrueMipCount() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "invalid mip level" );
	}
	result.CreateInstance();
	if( !result )
	{
		return Be::BLUE_STD_RESULT_MEMORY_ERROR;
	}
	bool createResult = false;
	uint32_t faceCount = 1;
	switch( GetType() )
	{
	case TEX_TYPE_CUBE:
		createResult = result->CreateCube( GetMipWidth( mipLevel ), 1, GetFormat() );
		faceCount = 6;
		break;
	case TEX_TYPE_3D:
		createResult = result->CreateVolume( GetMipWidth( mipLevel ), GetMipHeight( mipLevel ), GetMipDepth( mipLevel ), 1, GetFormat() );
		break;
	default:
		createResult = result->Create( GetMipWidth( mipLevel ), GetMipHeight( mipLevel ), 1, GetFormat() );
	}
	if( !createResult )
	{
		return Be::BLUE_STD_RESULT_MEMORY_ERROR;
	}
	for( uint32_t face = 0; face < faceCount; ++face )
	{
		memcpy( result->GetMipRawData( 0, CubemapFace( face ) ), GetMipRawData( mipLevel, CubemapFace( face ) ), result->GetMipSize( 0 ) );
	}
	return Be::BLUE_STD_RESULT_OK;
}

Be::BlueStdResult ImageToolsBitmap::SetMipData( uint32_t mipLevel, ImageToolsBitmap* result, uint32_t sourceMip )
{
	if( !IsValid() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "invalid bitmap" );
	}
	if( !result || !result->IsValid() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "invalid source bitmap" );
	}
	if( mipLevel >= GetTrueMipCount() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "invalid mip level" );
	}
	if( sourceMip >= result->GetTrueMipCount() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "invalid source mip level" );
	}
	if( GetType() != result->GetType() || GetFormat() != result->GetFormat() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "incompatible bitmaps" );
	}
	if( GetMipWidth( mipLevel ) != result->GetMipWidth( sourceMip ) ||
		GetMipHeight( mipLevel ) != result->GetMipHeight( sourceMip ) ||
		GetMipDepth( mipLevel ) != result->GetMipDepth( sourceMip ) )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "mip sizes do not match" );
	}
	uint32_t faces = 1;
	if( GetType() == TEX_TYPE_CUBE )
	{
		faces = 6;
	}
	for( uint32_t face = 0; face < faces; ++face )
	{
		memcpy( GetMipRawData( mipLevel, CubemapFace( face ) ), result->GetMipRawData( sourceMip, CubemapFace( face ) ), GetMipSize( mipLevel ) );
	}
	return Be::BLUE_STD_RESULT_OK;
}

Be::BlueStdResult ImageToolsBitmap::CreateNvttInputOptions( 
	CompressionOptions* compressionOptions, 
	nvtt::InputOptions& inputOptions )
{
	switch( GetType() )
	{
	case TEX_TYPE_1D:
	case TEX_TYPE_2D:
		inputOptions.setTextureLayout( nvtt::TextureType_2D, GetWidth(), GetHeight() );
		break;
	case TEX_TYPE_CUBE:
		inputOptions.setTextureLayout( nvtt::TextureType_Cube, GetWidth(), GetHeight() );
		break;
	case TEX_TYPE_3D:
		inputOptions.setTextureLayout( nvtt::TextureType_3D, GetWidth(), GetHeight(), GetDepth() );
		break;
	default:
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "unexpected bitmap type" );
	}
	switch( GetFormat() )
	{
	case PIXEL_FORMAT_R32G32B32A32_FLOAT:
		inputOptions.setFormat( nvtt::InputFormat_RGBA_32F );
		break;
	case PIXEL_FORMAT_R32G32B32_FLOAT:
		inputOptions.setFormat( nvtt::InputFormat_RGBA_32F );
		break;
	case PIXEL_FORMAT_R16G16B16A16_FLOAT:
		inputOptions.setFormat( nvtt::InputFormat_RGBA_16F );
		break;
	case PIXEL_FORMAT_B8G8R8A8_UNORM:
		inputOptions.setFormat( nvtt::InputFormat_BGRA_8UB );
		break;
	case PIXEL_FORMAT_B8G8R8X8_UNORM:
		inputOptions.setFormat( nvtt::InputFormat_BGRA_8UB );
		break;
	default:
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "unsupported input pixel format" );
	}
	inputOptions.setAlphaMode( nvtt::AlphaMode_None );
	if( compressionOptions && compressionOptions->GetGenerateMipsMaps() )
	{
		inputOptions.setMipmapGeneration( true );
	}
	else
	{
		if( GetMipCount() == 1 )
		{
			inputOptions.setMipmapGeneration( false );
		}
		else
		{
			inputOptions.setMipmapGeneration( true, GetTrueMipCount() );
		}
	}
	int faceCount = GetType() == TEX_TYPE_CUBE ? 6 : 1;
	uint32_t mipCount = compressionOptions && compressionOptions->GetGenerateMipsMaps() ? 1 : GetTrueMipCount();
	for( int face = 0; face < faceCount; ++face )
	{
		for( uint32_t i = 0; i < mipCount; ++i )
		{
			inputOptions.setMipmapData( GetMipRawData( i, CubemapFace( face ) ), GetMipWidth( i ), GetMipHeight( i ), GetMipDepth( i ), face, i );
		}
	}
	return Be::BLUE_STD_RESULT_OK;
}

Be::BlueStdResult ImageToolsBitmap::CompressWithOptions( CompressionOptions* options, const nvtt::OutputOptions& output )
{
	if( !IsValid() )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_VALUE_ERROR, "cannot compress invalid bitmap" );
	}

	nvtt::InputOptions input;
	CBR_RETURN_BR( CreateNvttInputOptions( options, input ) );

	nvtt::CompressionOptions compression;
	nvtt::Compressor compressor;
	if( options )
	{
		options->FillNvttOptions( compression );
		compressor.enableCudaAcceleration( options->UseCuda() );
	}

	if( !compressor.process( input, compression, output ) )
	{
		return Be::BlueStdResult( Be::BLUE_STD_RESULT_RUNTIME_ERROR, "error during image compression" );
	}
	return Be::BLUE_STD_RESULT_OK;
}

StdOrImageIOResult ImageToolsBitmap::Compress( CompressionOptions* options, ImageToolsBitmapPtr& result )
{
	AllowThreads allowThreads;

	MemoryOutputHandler outputHandler;

	nvtt::OutputOptions output;
	output.setOutputHandler( &outputHandler );

	CBR_RETURN_BR( CompressWithOptions( options, output ) );

	MemoryStream memStream( outputHandler.GetData(), outputHandler.GetSize() );

	result.CreateInstance();
	return ImageIO::ReadImage( memStream, ImageIO::LoadParameters( L"out.dds" ), *result );
}

Be::BlueStdResult ImageToolsBitmap::CompressToFile( const wchar_t* filename, CompressionOptions* options )
{
	AllowThreads allowThreads;

	nvtt::OutputOptions output;
	output.setFileName( CW2A( filename ) );

	return CompressWithOptions( options, output );
}