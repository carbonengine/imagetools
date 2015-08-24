////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#pragma once
#ifndef ImageToolsBitmap_H
#define ImageToolsBitmap_H

#include "ImageIOResultBlue.h"

BLUE_DECLARE( CompressionOptions );

BLUE_CLASS( ImageToolsBitmap ): public IRoot, public ImageIO::HostBitmap
{
public:
	EXPOSE_TO_BLUE();

	ImageToolsBitmap( IRoot* lockobj = 0 );
	StdOrImageIOResult Load( const wchar_t* filename );
	StdOrImageIOResult Save( const wchar_t* filename );
	StdOrImageIOResult Compress( CompressionOptions* options, ImageToolsBitmapPtr& result );
	Be::BlueStdResult CompressToFile( const wchar_t* filename, CompressionOptions* options );
	Be::BlueStdResult CreateFromArray( const std::vector<ImageToolsBitmap*>& elements );
private:
	Be::Result<std::string> CheckedDownsample2x2();
	Be::Result<std::string> CheckedCrop( unsigned left, unsigned top, unsigned right, unsigned bottom );
	Be::Result<std::string> CheckedGenerateMipMaps();
	Be::BlueStdResult CheckedConvertFormat( Tr2RenderContextEnum::PixelFormat format );

	Be::BlueStdResult CreateNvttInputOptions( CompressionOptions* compressionOptions, nvtt::InputOptions& inputOptions );
	Be::BlueStdResult CompressWithOptions( CompressionOptions* options, const nvtt::OutputOptions& outputOptions );

	Be::BlueStdResult Decompress( Tr2RenderContextEnum::PixelFormat format );
	Be::BlueStdResult Copy( ImageToolsBitmapPtr& result ) const;
	Be::BlueStdResult ExtractMipLevel( uint32_t mipLevel, ImageToolsBitmapPtr& result ) const;
	Be::BlueStdResult SetMipData( uint32_t mipLevel, ImageToolsBitmap* result, uint32_t sourceMip );
};

TYPEDEF_BLUECLASS( ImageToolsBitmap );

#endif