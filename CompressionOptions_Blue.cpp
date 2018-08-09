////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "CompressionOptions.h"


const Be::VarChooser Compressor_Chooser[] =
{
	{ "NVTT", BeCast( CompressionOptions::NVTT), "nVidia texture tools" },
	{ "COMPRESSONATOR", BeCast( CompressionOptions::COMPRESSONATOR ), "AMD Compressonator" },
	{ 0 }
};

BLUE_REGISTER_ENUM_EX(
	"COMPRESSOR",
	CompressionOptions::Compressor,
	Compressor_Chooser,
	ENUM_REG_ENUM_OBJECT_ON_MODULE
);

BLUE_DEFINE( CompressionOptions );

const Be::ClassInfo* ::CompressionOptions::ExposeToBlue()
{
    EXPOSURE_BEGIN( CompressionOptions, "" )
		MAP_INTERFACE( IRoot )
		MAP_INTERFACE( CompressionOptions )

		MAP_PROPERTY( "format", GetFormat, SetFormat, "compressed pixel format" );
		MAP_ATTRIBUTE( "alpha_for_bc1", m_alphaForBc1, "use binary alpha for BC1 format", Be::READWRITE );
		MAP_ATTRIBUTE( "quality", m_quality, "compression quality", Be::READWRITE );
		MAP_ATTRIBUTE( "generate_mips", m_generateMips, "True if mip levels need to be generated during compression", Be::READWRITE );
		MAP_ATTRIBUTE( "red_channel_weight", m_redWeight, "importance weight for red channel", Be::READWRITE );
		MAP_ATTRIBUTE( "green_channel_weight", m_greenWeight, "importance weight for green channel", Be::READWRITE );
		MAP_ATTRIBUTE( "blue_channel_weight", m_blueWeight, "importance weight for blue channel", Be::READWRITE );
		MAP_ATTRIBUTE( "alpha_channel_weight", m_alphaWeight, "importance weight for alpha channel", Be::READWRITE );
		MAP_ATTRIBUTE( "use_cuda", m_useCuda, "usage CUDA compression on GPU if available", Be::READWRITE );
		MAP_ATTRIBUTE( "compressor", m_compressor, "which compressor to use (see COMPRESSOR enum)", Be::READWRITE );

		MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( 
			"__init__", 
			Create, 
			4,
			"Initializer for CompressionOptions\n"
			"Arguments:\n"
			"format - (optional) compressed pixel format (member of imagetools.PIXEL_FORMAT, only compressed formats supported)\n"
			"quality - (optional) compression quality (member of imagetools.COMPRESSION_QUALITY)\n"
			"generate_mips - (optional) True if mip levels need to be generated during compression\n"
			"alpha_for_bc1 - (optional) True if BC1 compressed output should contain transparency" );

	EXPOSURE_END()
}