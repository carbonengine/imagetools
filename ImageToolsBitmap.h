////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#pragma once
#ifndef ImageToolsBitmap_H
#define ImageToolsBitmap_H

BLUE_DECLARE( CompressionOptions );

BLUE_CLASS( ImageToolsBitmap ): public IRoot, public ImageIO::HostBitmap
{
public:
	EXPOSE_TO_BLUE();

	ImageToolsBitmap( IRoot* lockobj = 0 );
	Be::Result<std::string> Load( const wchar_t* filename );
	Be::Result<std::string> Save( const wchar_t* filename );
	Be::Result<std::string> Compress( CompressionOptions* options, ImageToolsBitmapPtr& result );
	Be::Result<std::string> CompressToFile( const wchar_t* filename, CompressionOptions* options );
private:
	Be::Result<std::string> CheckedDownsample2x2();
	Be::Result<std::string> CheckedCrop( unsigned left, unsigned top, unsigned right, unsigned bottom );
	Be::Result<std::string> CheckedGenerateMipMaps();
	Be::Result<std::string> CheckedConvertFormat( Tr2RenderContextEnum::PixelFormat format );

	Be::Result<std::string> CreateNvttInputOptions( CompressionOptions* compressionOptions, nvtt::InputOptions& inputOptions );
	Be::Result<std::string> CompressWithOptions( CompressionOptions* options, const nvtt::OutputOptions& outputOptions );

	Be::Result<std::string> Decompress( Tr2RenderContextEnum::PixelFormat format );
	Be::Result<std::string> Copy( ImageToolsBitmapPtr& result ) const;
};

TYPEDEF_BLUECLASS( ImageToolsBitmap );

#endif