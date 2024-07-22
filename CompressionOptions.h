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

	enum Compressor
	{
		NVTT,
		COMPRESSONATOR,
	};

	CompressionOptions( IRoot* lockobj = 0 );

	BlueStdResult Create( 
		Be::OptionalWithDefaultValue<ImageIO::PixelFormat, ImageIO::PIXEL_FORMAT_BC1_UNORM> format,
		Be::OptionalWithDefaultValue<nvtt::Quality, nvtt::Quality_Production> quality,
		Be::OptionalWithDefaultValue<bool, false> generateMips,
		Be::OptionalWithDefaultValue<bool, false> alphaForBc1 );


	ImageIO::PixelFormat GetFormat() const;
	BlueStdResult SetFormat( ImageIO::PixelFormat );

	Compressor GetCompressor() const;


	void FillNvttOptions( nvtt::CompressionOptions& options );
#if WITH_COMPRESSONATOR
	void FillCompressonatorOptions( CMP_CompressOptions& options );
#endif
	bool UseCuda() const;

	bool GetGenerateMipsMaps() const;
private:
	Compressor m_compressor;
	ImageIO::PixelFormat m_format;
	nvtt::Quality m_quality;
	bool m_alphaForBc1;
	bool m_generateMips;
	bool m_useCuda;
	float m_redWeight;
	float m_greenWeight;
	float m_blueWeight;
	float m_alphaWeight;

};

TYPEDEF_BLUECLASS( CompressionOptions );

#endif