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

#define CBR_RETURN_BR( x ) { auto ret = x; if( !BeIsSuccess( ret ) ) { return ret; } }

namespace
{
#if WITH_COMPRESSONATOR
	CMP_FORMAT PixelFormatToCmpFormat( Tr2RenderContextEnum::PixelFormat format )
	{
		switch( format )
		{
		case Tr2RenderContextEnum::PIXEL_FORMAT_B8G8R8A8_UNORM:
		case Tr2RenderContextEnum::PIXEL_FORMAT_B8G8R8X8_UNORM:
			return CMP_FORMAT_BGRA_8888;
		case Tr2RenderContextEnum::PIXEL_FORMAT_R8_UNORM:
			return CMP_FORMAT_R_8;
		case Tr2RenderContextEnum::PIXEL_FORMAT_BC1_UNORM:
			return CMP_FORMAT_BC1;
		case Tr2RenderContextEnum::PIXEL_FORMAT_BC2_UNORM:
			return CMP_FORMAT_BC2;
		case Tr2RenderContextEnum::PIXEL_FORMAT_BC3_UNORM:
			return CMP_FORMAT_BC3;
		case Tr2RenderContextEnum::PIXEL_FORMAT_BC4_UNORM:
			return CMP_FORMAT_BC4;
		case Tr2RenderContextEnum::PIXEL_FORMAT_BC5_UNORM:
			return CMP_FORMAT_BC5;
		case Tr2RenderContextEnum::PIXEL_FORMAT_BC6H_UF16:
			return CMP_FORMAT_BC6H;
		case Tr2RenderContextEnum::PIXEL_FORMAT_BC7_UNORM:
			return CMP_FORMAT_BC7;
		default:
			return CMP_FORMAT_Unknown;
		}
	}
#endif
}

ImageToolsBitmap::ImageToolsBitmap( IRoot* lockobj )
{
}

StdOrImageIOResult ImageToolsBitmap::Load( const wchar_t* filename )
{
	AllowThreads allowThreads;

	FileStream stream( filename, FileStream::READ );
	if( !stream.IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_IO_ERROR, "failed to open file" );
	}

	return ImageIOResult( ImageIO::ReadImage( stream, ImageIO::LoadParameters( filename ), *this ) );
}

StdOrImageIOResult ImageToolsBitmap::Save( const wchar_t* filename )
{
	AllowThreads allowThreads;

	if( !IsValid() )
	{
		return StdOrImageIOResult( ImageIO::Result( ImageIO::Result::INVALID_BITMAP, "cannot save invalid bitmap" ) );
	}

	FileStream stream( filename, FileStream::WRITE );
	if( !stream.IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_IO_ERROR, "could not open file for saving" );
	}

	return StdOrImageIOResult( ImageIO::SaveImage( filename, *this, stream ) );
}

Be::Result<std::string> ImageToolsBitmap::CheckedCopyChannel( ImageToolsBitmap* source, unsigned srcChannel, unsigned dstChannel )
{
	if( !CopyChannel( source, srcChannel, dstChannel ) )
	{
		return Be::Result<std::string>( "Copy channel failed" );
	}
	return std::string();
}

Be::Result<std::string> ImageToolsBitmap::CheckedDownsample2x2()
{
	AllowThreads allowThreads;

	if( !Downsample2x2() )
	{
		return Be::Result<std::string>( "error while downsampling the bitmap" );
	}
	return std::string();
}

Be::Result<std::string> ImageToolsBitmap::CheckedCrop( unsigned left, unsigned top, unsigned right, unsigned bottom )
{
	if( !Crop( left, top, right, bottom ) )
	{
		return Be::Result<std::string>( "error while cropping the bitmap" );
	}
	return std::string();
}

Be::Result<std::string> ImageToolsBitmap::CheckedGenerateMipMaps( unsigned levels )
{
	AllowThreads allowThreads;

	if( !GenerateMipMaps( levels ) )
	{
		return Be::Result<std::string>( "error while generating mip levels" );
	}
	return std::string();
}

BlueStdResult ImageToolsBitmap::CheckedConvertFormat( Tr2RenderContextEnum::PixelFormat format )
{
	AllowThreads allowThreads;

	if( IsCompressed() && ( format == PIXEL_FORMAT_B8G8R8A8_UNORM || format == PIXEL_FORMAT_B8G8R8X8_UNORM ) )
	{
		return Decompress( format );
	}
	if( !ConvertFormat( format ) )
	{
		return BlueStdResult( BLUE_STD_RESULT_RUNTIME_ERROR, "error converting pixel format" );
	}
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::CreateFromArray( const std::vector<ImageToolsBitmap*>& elements )
{
	if( elements.empty() )
	{
		Destroy();
		return BLUE_STD_RESULT_OK;
	}
	if( !elements[0] || elements[0]->GetType() != TEX_TYPE_2D || elements[0]->GetArraySize() != 1 || elements[0] == this )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid first bitmap in the list" );
	}
	for( size_t i = 1; i < elements.size(); ++i )
	{
		if( !elements[i] || elements[i]->GetArraySize() != 1 || elements[i] == this )
		{
			return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid bitmap in the list" );
		}
		if( elements[0]->GetType() != elements[i]->GetType() || 
			elements[0]->GetFormat() != elements[i]->GetFormat() || 
			elements[0]->GetWidth() != elements[i]->GetWidth() || 
			elements[0]->GetHeight() != elements[i]->GetHeight() || 
			elements[0]->GetTrueMipCount() != elements[i]->GetTrueMipCount() )
		{
			return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "bitmap formats/sizes do not match" );
		}
	}
	Destroy();
	*static_cast<Tr2BitmapDimensions*>( this ) = *elements[0];
	m_arraySize = uint32_t( elements.size() );
	auto elementSize = elements[0]->m_data.size();
	m_data.resize( "HostBitmap::m_data", elementSize * elements.size() );
	for( size_t i = 0; i < elements.size(); ++i )
	{
		memcpy( m_data.get() + elementSize * i, elements[i]->m_data.get(), elementSize );
	}
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::Decompress( Tr2RenderContextEnum::PixelFormat format )
{
	if( !IsCompressed() )//|| GetType() != TEX_TYPE_2D )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "cannot decompress an uncompressed image" );
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
    default:
        return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid image format" );
	}
	const size_t bpp = 4;
	const size_t pixelCount = GetRawDataSize() * 16 / GetBlockByteSize( m_format );
	const size_t newSize = pixelCount * bpp;
	CcpMallocBuffer data( "HostBitmap::m_data", newSize );
	size_t mipStart = 0;

	for( uint32_t mip = 0; mip < GetTrueMipCount(); ++mip )
	{
		const size_t mipSize = GetMipWidth( mip ) * GetMipHeight( mip );
		for( uint32_t face = 0; face < GetArraySize(); ++face )
		{
			if( !surface.setImage2D( nvttFormat, nvtt::Decoder_D3D10, GetMipWidth( mip ), GetMipHeight( mip ), GetMipRawData( mip, face ) ) )
			{
				Destroy();
				return BlueStdResult( BLUE_STD_RESULT_RUNTIME_ERROR, "could not decompress image" );
			}
	
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
	}
	m_data.swap( data );
	m_format = format;
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::Copy( ImageToolsBitmapPtr& result ) const
{
	result.CreateInstance();
	if( !result )
	{
		return BLUE_STD_RESULT_MEMORY_ERROR;
	}
	static_cast<Tr2BitmapDimensions&>( *result ) = *this;
	result->m_name = m_name;
	result->m_data.resize( "HostBitmap::m_data", m_data.size() );
	if( !result->m_data.get() )
	{
		result = nullptr;
		return BLUE_STD_RESULT_MEMORY_ERROR;
	}
	memcpy( result->m_data.get(), m_data.get(), m_data.size() );
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::FlattenSlices( bool horizontally, ImageToolsBitmapPtr& result ) const
{
	if( !IsValid() || IsCompressed() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "source bitmap is invalid" );
	}
	result.CreateInstance();
	uint32_t sliceCount = 1;
	switch( GetType() )
	{
	case TEX_TYPE_2D:
		sliceCount = GetArraySize();
		break;
	case TEX_TYPE_CUBE:
		sliceCount = 6;
		break;
	case TEX_TYPE_3D:
		sliceCount = GetDepth();
		break;
	default:
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "unsupported bitmap" );
	}
	if( !result->Create( horizontally ? GetWidth() * sliceCount : GetWidth(), !horizontally ? GetHeight() * sliceCount : GetHeight(), 0, GetFormat() ) )
	{
		return BLUE_STD_RESULT_MEMORY_ERROR;
	}
	auto dst = result->GetRawData();
	for( uint32_t i = 0; i < sliceCount; ++i )
	{
		auto src = GetType() == TEX_TYPE_3D ? GetMipRawData( 0 ) + GetMipPitch( 0 ) * GetMipHeight( 0 ) * i : GetMipRawData( 0, i );
		auto size = GetMipSize( 0 ) / sliceCount;
		auto pitch = GetMipPitch( 0 );

		if( horizontally )
		{
			auto row = GetMipWidth( 0 ) * Tr2RenderContextEnum::GetBytesPerPixel( GetFormat() );
			auto d = dst + row * i;
			for( uint32_t y = 0; y < GetMipHeight( 0 ); ++y )
			{
				memcpy( d, src, row );
				d += result->GetMipPitch( 0 );
				src += GetMipPitch( 0 );
			}
		}
		else
		{
			auto row = GetMipWidth( 0 ) * Tr2RenderContextEnum::GetBytesPerPixel( GetFormat() );
			auto d = dst + result->GetMipPitch( 0 ) * GetMipHeight( 0 ) * i;
			memcpy( d, src, size );
		}
	}
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::ExtractMipLevel( uint32_t mipLevel, Be::OptionalWithDefaultValue<uint32_t, 1> count, ImageToolsBitmapPtr& result ) const
{
	if( !IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "source bitmap is invalid" );
	}
	if( mipLevel >= GetTrueMipCount() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid mip level" );
	}
	result.CreateInstance();
	if( !result )
	{
		return BLUE_STD_RESULT_MEMORY_ERROR;
	}
	bool createResult = false;
	uint32_t faceCount = 1;
	uint32_t mipCount = std::min( uint32_t( count ), GetTrueMipCount() - mipLevel );
	switch( GetType() )
	{
	case TEX_TYPE_CUBE:
		createResult = result->CreateCube( GetMipWidth( mipLevel ), mipCount, GetFormat() );
		faceCount = 6;
		break;
	case TEX_TYPE_3D:
		createResult = result->CreateVolume( GetMipWidth( mipLevel ), GetMipHeight( mipLevel ), GetMipDepth( mipLevel ), mipCount, GetFormat() );
		break;
	default:
		createResult = result->Create( GetMipWidth( mipLevel ), GetMipHeight( mipLevel ), mipCount, GetFormat() );
	}
	if( !createResult )
	{
		return BLUE_STD_RESULT_MEMORY_ERROR;
	}
	for( uint32_t face = 0; face < faceCount; ++face )
	{
		for( uint32_t m = 0; m < mipCount; ++m )
		{
			memcpy( result->GetMipRawData( m, CubemapFace( face ) ), GetMipRawData( mipLevel + m, CubemapFace( face ) ), result->GetMipSize( m ) );
		}
	}
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::ToYuv( std::pair<ImageToolsBitmapPtr, ImageToolsBitmapPtr>& result ) const
{
	if( !IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "source bitmap is invalid" );
	}
	if( GetFormat() != Tr2RenderContextEnum::PIXEL_FORMAT_B8G8R8A8_UNORM && GetFormat() != Tr2RenderContextEnum::PIXEL_FORMAT_B8G8R8X8_UNORM )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid format" );
	}
	result.first.CreateInstance();
	result.second.CreateInstance();
	if( !result.first || !result.second )
	{
		return BLUE_STD_RESULT_MEMORY_ERROR;
	}
	bool createResult = false;
	uint32_t faceCount = 1;
	switch( GetType() )
	{
	case TEX_TYPE_CUBE:
		createResult = result.first->CreateCube( GetWidth(), GetMipCount(), Tr2RenderContextEnum::PIXEL_FORMAT_R8_UNORM );
		createResult = result.second->CreateCube( GetWidth() / 2, GetMipCount() ? std::max( GetMipCount() - 1, 1u ) : 0, Tr2RenderContextEnum::PIXEL_FORMAT_B8G8R8A8_UNORM ) && createResult;
		faceCount = 6;
		break;
    default:
        break;
	}
	if( !createResult )
	{
		return BLUE_STD_RESULT_MEMORY_ERROR;
	}
	memset( result.second->GetRawData(), 0, result.second->GetRawDataSize() );
	for( uint32_t mip = 0; mip < GetMipCount(); ++mip )
	{
		for( uint32_t face = 0; face < faceCount; ++face )
		{
			const uint8_t* src = reinterpret_cast<const uint8_t*>( GetMipRawData( mip, CubemapFace( face ) ) );
			uint8_t* y = reinterpret_cast<uint8_t*>( result.first->GetMipRawData( mip, CubemapFace( face ) ) );
			uint8_t* u = nullptr;
			uint8_t* v = nullptr;
			if( mip < result.second->GetMipCount() )
			{
				u = reinterpret_cast<uint8_t*>( result.second->GetMipRawData( mip, CubemapFace( face ) ) ) + 1;
				v = reinterpret_cast<uint8_t*>( result.second->GetMipRawData( mip, CubemapFace( face ) ) ) + 2;
			}
			for( uint32_t j = 0; j < GetMipHeight( mip ); ++j )
			{
				for( uint32_t i = 0; i < GetMipWidth( mip ); ++i )
				{
					float r = src[i * 4 + 2];
					float g = src[i * 4 + 1];
					float b = src[i * 4 + 0];
					int yy = int( r * 0.299f + g * 0.587f + b * 0.114f );
					y[i] = yy;
					if( mip < result.second->GetMipCount() && i % 2 == 0 && j % 2 == 0 )
					{
						u[i * 2] = int( r * -0.168736f + g * -0.331264f + b * 0.500000f + 128 );
						v[i * 2] = int( r *  0.500000f + g * -0.418688f + b * -0.081312f + 128 );
					}
				}
				src += GetMipPitch( mip );
				y += result.first->GetMipPitch( mip );
				if( mip < result.second->GetMipCount() && j % 2 == 0 )
				{
					u += result.second->GetMipPitch( mip );
					v += result.second->GetMipPitch( mip );
				}
			}
		}
	}
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::SetMipData( uint32_t mipLevel, ImageToolsBitmap* result, uint32_t sourceMip )
{
	if( !IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid bitmap" );
	}
	if( !result || !result->IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid source bitmap" );
	}
	if( mipLevel >= GetTrueMipCount() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid mip level" );
	}
	if( sourceMip >= result->GetTrueMipCount() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "invalid source mip level" );
	}
	if( GetType() != result->GetType() || GetFormat() != result->GetFormat() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "incompatible bitmaps" );
	}
	if( GetMipWidth( mipLevel ) != result->GetMipWidth( sourceMip ) ||
		GetMipHeight( mipLevel ) != result->GetMipHeight( sourceMip ) ||
		GetMipDepth( mipLevel ) != result->GetMipDepth( sourceMip ) )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "mip sizes do not match" );
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
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::CreateNvttInputOptions( 
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
		if( GetMipCount() != 1 || ( compressionOptions && compressionOptions->GetGenerateMipsMaps() ) )
		{
			return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "compressing volume textures with mip maps is not supported" );
		}
		if( GetWidth() % 4 || GetHeight() % 4 || GetDepth() % 4 )
		{
			return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "compressing volume textures with sizes not divisible by 4 is not supported" );
		}
		inputOptions.setTextureLayout( nvtt::TextureType_2D, GetWidth(), GetHeight() * GetDepth() );
		break;
	default:
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "unexpected bitmap type" );
	}
	PixelFormat inputFormat = GetFormat();
	switch( inputFormat )
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
	case PIXEL_FORMAT_B8G8R8X8_UNORM:
	case PIXEL_FORMAT_R8_UNORM:
	case PIXEL_FORMAT_R8G8_UNORM:
		inputOptions.setFormat( nvtt::InputFormat_BGRA_8UB );
		break;
	default:
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "unsupported input pixel format" );
	}
	
	if( compressionOptions )
	{
		PixelFormat compressionFormat = compressionOptions->GetFormat();
		if( (compressionFormat == PIXEL_FORMAT_BC7_UNORM || compressionFormat == PIXEL_FORMAT_BC7_UNORM_SRGB) &&
			( inputFormat != PIXEL_FORMAT_B8G8R8A8_UNORM && inputFormat != PIXEL_FORMAT_B8G8R8X8_UNORM ))
		{
			// NVTT compression on BC7 floating point formatted textures seems to result in assertion errors.
			return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "Unsupported input pixel format for BC7" );
		}
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
			uint32_t width = GetMipWidth( i );
			uint32_t height = GetType() == TEX_TYPE_3D ? GetMipHeight( i ) * GetMipDepth( i ) : GetMipHeight( i );
			auto data = GetMipRawData( i, CubemapFace( face ) );
			switch( inputFormat )
			{
			case PIXEL_FORMAT_R8_UNORM:
				{
					std::unique_ptr<uint8_t, TrackableDelete<uint8_t>> copy( CCP_NEW( "ImageToolsBitmap::CreateNvttInputOptions/copy" ) uint8_t[width * height * 4] );
					auto src = reinterpret_cast<const uint8_t*>( data );
					auto dst = copy.get();
					for( uint32_t p = 0; p < width * height; ++p )
					{
						*dst++ = *src;
						*dst++ = *src;
						*dst++ = *src;
						*dst++ = *src;
						++src;
					}
					inputOptions.setMipmapData( copy.get(), width, height, 1, face, i );
			}
				break;
			case PIXEL_FORMAT_R8G8_UNORM:
				{
					std::unique_ptr<uint8_t, TrackableDelete<uint8_t>> copy( CCP_NEW( "ImageToolsBitmap::CreateNvttInputOptions/copy" ) uint8_t[width * height * 4] );
					auto src = reinterpret_cast<const uint8_t*>( data );
					auto dst = copy.get();
					for( uint32_t p = 0; p < width * height; ++p )
					{
						*dst++ = *src;
						*dst++ = *src++;
						*dst++ = *src;
						*dst++ = *src;
						++src;
					}
					inputOptions.setMipmapData( copy.get(), width, height, 1, face, i );
				}
				break;
			default:
				inputOptions.setMipmapData( data, width, height, 1, face, i );
			}
		}
	}
	return BLUE_STD_RESULT_OK;
}

BlueStdResult ImageToolsBitmap::CompressWithOptions( CompressionOptions* options, const nvtt::OutputOptions& output )
{
	if( !IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "cannot compress invalid bitmap" );
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
		return BlueStdResult( BLUE_STD_RESULT_RUNTIME_ERROR, "error during image compression" );
	}
	return BLUE_STD_RESULT_OK;
}

StdOrImageIOResult ImageToolsBitmap::Compress( CompressionOptions* options, ImageToolsBitmapPtr& result )
{
	AllowThreads allowThreads;

	CompressionOptions::Compressor compressor = CompressionOptions::NVTT;
	if( options )
	{
		compressor = options->GetCompressor();
	}

#if WITH_COMPRESSONATOR
	if( compressor == CompressionOptions::COMPRESSONATOR )
	{
		result.CreateInstance();

		Tr2RenderContextEnum::PixelFormat destFormat;
		CMP_CompressOptions cmpOptions = { 0 };
		if( !options )
		{
			CompressionOptionsPtr defaultOptions;
			defaultOptions.CreateInstance();
			defaultOptions->FillCompressonatorOptions( cmpOptions );
			destFormat = defaultOptions->GetFormat();
		}
		else
		{
			options->FillCompressonatorOptions( cmpOptions );
			destFormat = options->GetFormat();
		}

		Tr2BitmapDimensions dim( GetType(), destFormat, GetWidth(), GetHeight(), GetDepth(), GetTrueMipCount(), GetArraySize() );
		if( !result->CreateFromBitmapDimensions( dim ) )
		{
			return BlueStdResult( BLUE_STD_RESULT_MEMORY_ERROR );
		}

		for( uint32_t ai = 0; ai < GetArraySize(); ++ai )
		{
			for( uint32_t mi = 0; mi < GetTrueMipCount(); ++mi )
			{
				CMP_Texture srcTexture;
				srcTexture.dwSize = sizeof( srcTexture );
				srcTexture.dwWidth = GetMipWidth( mi );
				srcTexture.dwHeight = GetMipHeight( mi );
				srcTexture.dwPitch = 0;
				srcTexture.format = PixelFormatToCmpFormat( GetFormat() );
				srcTexture.dwDataSize = GetMipSize( mi );
				srcTexture.pData = (CMP_BYTE*)GetMipRawData( mi, ai );

				CMP_Texture destTexture;
				destTexture.dwSize = sizeof( destTexture );
				destTexture.dwWidth = GetMipWidth( mi ); result->GetMipWidth( mi );
				destTexture.dwHeight = GetMipHeight( mi ); result->GetMipHeight( mi );
				destTexture.dwPitch = 0;
				destTexture.format = PixelFormatToCmpFormat( destFormat );
				destTexture.dwDataSize = result->GetMipSize( mi );
				destTexture.pData = (CMP_BYTE*)result->GetMipRawData( mi, ai );

				auto cmpStatus = CMP_ConvertTexture( &srcTexture, &destTexture, &cmpOptions, nullptr, 0, 0 );
				if( cmpStatus != CMP_OK )
				{
					return BlueStdResult( BLUE_STD_RESULT_RUNTIME_ERROR, "compressonator error" );
				}
			}
		}
	}
	else
#endif
	{
		MemoryOutputHandler outputHandler;

		nvtt::OutputOptions output;
		output.setOutputHandler( &outputHandler );

		if( IsDds10Format( options->GetFormat() ) )
		{
			output.setContainer( nvtt::Container_DDS10 );
		}

		CBR_RETURN_BR( CompressWithOptions( options, output ) );

		MemoryStream memStream( outputHandler.GetData(), outputHandler.GetSize() );

		result.CreateInstance();
		CBR_RETURN_BR( ImageIOResult( ImageIO::ReadImage( memStream, ImageIO::LoadParameters( L"out.dds" ), *result ) ) );
		if( result && GetType() == TEX_TYPE_3D )
		{
			result->m_type = GetType();
			result->m_height /= GetDepth();
			result->m_volumeDepth = GetDepth();
		}
	}

	return BlueStdResult( BLUE_STD_RESULT_OK );
}

BlueStdResult ImageToolsBitmap::CompressToFile( const wchar_t* filename, CompressionOptions* options )
{
	AllowThreads allowThreads;

	nvtt::OutputOptions output;
	output.setFileName( CW2A( filename ) );

	return CompressWithOptions( options, output );
}