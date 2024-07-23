////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "CompressionOptions.h"

using namespace ImageIO;

CompressionOptions::CompressionOptions( IRoot* lockobj )
	:m_compressor( NVTT ),
	m_format( PIXEL_FORMAT_BC1_UNORM ),
	m_quality( nvtt::Quality_Production ),
	m_alphaForBc1( false ),
	m_generateMips( false ),
	m_useCuda( true ),
	m_redWeight( 1.0f ),
	m_greenWeight( 1.0f ),
	m_blueWeight( 1.0f ),
	m_alphaWeight( 1.0f )
{
}

BlueStdResult CompressionOptions::Create( 
	Be::OptionalWithDefaultValue<PixelFormat, PIXEL_FORMAT_BC1_UNORM> format,
	Be::OptionalWithDefaultValue<nvtt::Quality, nvtt::Quality_Production> quality,
	Be::OptionalWithDefaultValue<bool, false> generateMips,
	Be::OptionalWithDefaultValue<bool, false> alphaForBc1 )
{
	auto result = SetFormat( format );
	if( !BeIsSuccess( result ) )
	{
		return result;
	}
	m_quality = quality;
	m_alphaForBc1 = alphaForBc1;
	m_generateMips = generateMips;
	return BLUE_STD_RESULT_OK;
}

PixelFormat CompressionOptions::GetFormat() const
{
	return m_format;
}

BlueStdResult CompressionOptions::SetFormat( PixelFormat format )
{
	if( !IsCompressedFormat( format ) )
	{
		return BlueStdResult( BLUE_STD_RESULT_VALUE_ERROR, "format needs to be a compressed format (imagetools.PIXEL_FORMAT.BC#_...)" );
	}
	m_format = format;
	return BLUE_STD_RESULT_OK;
}

CompressionOptions::Compressor CompressionOptions::GetCompressor() const
{
	return m_compressor;
}

void CompressionOptions::FillNvttOptions( nvtt::CompressionOptions& options )
{
	switch( m_format )
	{
	case PIXEL_FORMAT_BC1_TYPELESS:
	case PIXEL_FORMAT_BC1_UNORM:
	case PIXEL_FORMAT_BC1_UNORM_SRGB:
		options.setFormat( m_alphaForBc1 ? nvtt::Format_BC1a : nvtt::Format_BC1 );
		break;
	case PIXEL_FORMAT_BC2_TYPELESS:
	case PIXEL_FORMAT_BC2_UNORM:
	case PIXEL_FORMAT_BC2_UNORM_SRGB:
		options.setFormat( nvtt::Format_BC2 );
		break;
	case PIXEL_FORMAT_BC3_TYPELESS:
	case PIXEL_FORMAT_BC3_UNORM:
	case PIXEL_FORMAT_BC3_UNORM_SRGB:
		options.setFormat( nvtt::Format_BC3 );
		break;
	case PIXEL_FORMAT_BC4_TYPELESS:
	case PIXEL_FORMAT_BC4_UNORM:
	case PIXEL_FORMAT_BC4_SNORM:
		options.setFormat( nvtt::Format_BC4 );
		break;
	case PIXEL_FORMAT_BC5_TYPELESS:
	case PIXEL_FORMAT_BC5_UNORM:
	case PIXEL_FORMAT_BC5_SNORM:
		options.setFormat( nvtt::Format_BC5 );
		break;
	case PIXEL_FORMAT_BC6H_TYPELESS:
	case PIXEL_FORMAT_BC6H_UF16:
	case PIXEL_FORMAT_BC6H_SF16:
		options.setFormat( nvtt::Format_BC6 );
		break;
	case PIXEL_FORMAT_BC7_TYPELESS:
	case PIXEL_FORMAT_BC7_UNORM:
	case PIXEL_FORMAT_BC7_UNORM_SRGB:
		options.setFormat( nvtt::Format_BC7 );
		break;
    default:
        break;
	}
	options.setQuality( m_quality );
	options.setColorWeights( m_redWeight, m_greenWeight, m_blueWeight, m_alphaWeight );
}

#if WITH_COMPRESSONATOR

void CompressionOptions::FillCompressonatorOptions( CMP_CompressOptions& options )
{
	memset( &options, 0, sizeof( options ) );
	options.dwSize = sizeof( options );

	options.bUseGPUCompress = m_useCuda ? 1 : 0;
	switch( m_quality )
	{
	case nvtt::Quality_Fastest:
		options.fquality = 0.05f;
		break;
	case nvtt::Quality_Production:
		options.fquality = 0.75f;
		break;
	case nvtt::Quality_Highest:
		options.fquality = 1.0f;
		break;
	default:
		options.fquality = 0.5f;
	}
	if( m_alphaForBc1 )
	{
		options.bDXT1UseAlpha = 1;
	}
	if( m_redWeight != 1 || m_greenWeight != 1 || m_blueWeight != 1 )
	{
		options.bUseChannelWeighting = 1;
		options.fWeightingRed = m_redWeight;
		options.fWeightingGreen = m_greenWeight;
		options.fWeightingBlue = m_blueWeight;
	}
}

#endif

bool CompressionOptions::UseCuda() const
{
	return m_useCuda;
}

bool CompressionOptions::GetGenerateMipsMaps() const
{
	return m_generateMips;
}
