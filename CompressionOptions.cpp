////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "CompressionOptions.h"

using namespace Tr2RenderContextEnum;

CompressionOptions::CompressionOptions( IRoot* lockobj )
	:m_format( PIXEL_FORMAT_BC1_UNORM ),
	m_alphaForBc1( false ),
	m_quality( nvtt::Quality_Production ),
	m_generateMips( false ),
	m_redWeight( 1.0f ),
	m_greenWeight( 1.0f ),
	m_blueWeight( 1.0f ),
	m_alphaWeight( 1.0f )
{
}

Be::Result<std::string> CompressionOptions::Create( 
	Be::OptionalWithDefaultValue<Tr2RenderContextEnum::PixelFormat, Tr2RenderContextEnum::PIXEL_FORMAT_BC1_UNORM> format,
	Be::OptionalWithDefaultValue<nvtt::Quality, nvtt::Quality_Production> quality,
	Be::OptionalWithDefaultValue<bool, false> generateMips,
	Be::OptionalWithDefaultValue<bool, false> alphaForBc1 )
{
	auto result = SetFormat( format );
	if( !Be::IsSuccess( result ) )
	{
		return result;
	}
	m_quality = quality;
	m_alphaForBc1 = alphaForBc1;
	m_generateMips = generateMips;
	return std::string();
}

Tr2RenderContextEnum::PixelFormat CompressionOptions::GetFormat() const
{
	return m_format;
}

Be::Result<std::string> CompressionOptions::SetFormat( Tr2RenderContextEnum::PixelFormat format )
{
	if( !IsCompressedFormat( format ) )
	{
		return "format needs to be a compressed format (imagetools.PIXEL_FORMAT.BC#_...)";
	}
	m_format = format;
	return std::string();
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
	}
	options.setQuality( m_quality );
	options.setColorWeights( m_redWeight, m_greenWeight, m_blueWeight, m_alphaWeight );
}

bool CompressionOptions::GetGenerateMipsMaps() const
{
	return m_generateMips;
}
