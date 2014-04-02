////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#pragma once
#ifndef CompressionOptions_H
#define CompressionOptions_H

BLUE_CLASS( CompressionOptions ): public IRoot
{
public:
	EXPOSE_TO_BLUE();

	CompressionOptions( IRoot* lockobj = 0 );

	Be::Result<std::string> Create( 
		Be::OptionalWithDefaultValue<Tr2RenderContextEnum::PixelFormat, Tr2RenderContextEnum::PIXEL_FORMAT_BC1_UNORM> format,
		Be::OptionalWithDefaultValue<nvtt::Quality, nvtt::Quality_Production> quality,
		Be::OptionalWithDefaultValue<bool, false> generateMips,
		Be::OptionalWithDefaultValue<bool, false> alphaForBc1 );


	Tr2RenderContextEnum::PixelFormat GetFormat() const;
	Be::Result<std::string> SetFormat( Tr2RenderContextEnum::PixelFormat );

	void FillNvttOptions( nvtt::CompressionOptions& options );

	bool GetGenerateMipsMaps() const;
private:
	Tr2RenderContextEnum::PixelFormat m_format;
	bool m_alphaForBc1;
	nvtt::Quality m_quality;
	bool m_generateMips;
	float m_redWeight;
	float m_greenWeight;
	float m_blueWeight;
	float m_alphaWeight;
};

TYPEDEF_BLUECLASS( CompressionOptions );

#endif