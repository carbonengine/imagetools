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
	BlueStdResult CompressToFile( const wchar_t* filename, CompressionOptions* options );
	BlueStdResult CreateFromArray( const std::vector<ImageToolsBitmap*>& elements );
	ImageIO::MetadataStrings GetMetadataStrings() const;
	void SetMetadataStrings( const ImageIO::MetadataStrings& strings );
	void ClearMetadata();
private:
	Be::Result<std::string> CheckedCopyChannel( ImageToolsBitmap* source, unsigned srcChannel, unsigned dstChannel );
	Be::Result<std::string> CheckedDownsample2x2();
	Be::Result<std::string> CheckedCrop( unsigned left, unsigned top, unsigned right, unsigned bottom );
	Be::Result<std::string> CheckedGenerateMipMaps( unsigned levels = 0 );
	BlueStdResult CheckedConvertFormat( ImageIO::PixelFormat format );

	BlueStdResult CreateNvttInputOptions( CompressionOptions* compressionOptions, nvtt::InputOptions& inputOptions );
	BlueStdResult CompressWithOptions( CompressionOptions* options, const nvtt::OutputOptions& outputOptions );

	BlueStdResult Decompress( ImageIO::PixelFormat format );
	BlueStdResult Copy( ImageToolsBitmapPtr& result ) const;
	BlueStdResult ExtractMipLevel( uint32_t mipLevel, Be::OptionalWithDefaultValue<uint32_t, 1> count, ImageToolsBitmapPtr& result ) const;
	BlueStdResult SetMipData( uint32_t mipLevel, ImageToolsBitmap* result, uint32_t sourceMip );
	BlueStdResult ToYuv( std::pair<ImageToolsBitmapPtr, ImageToolsBitmapPtr>& result ) const;
	BlueStdResult FlattenSlices( bool horizontally, ImageToolsBitmapPtr& result ) const;

	ImageIO::Metadata m_metadata;
};

TYPEDEF_BLUECLASS( ImageToolsBitmap );

#endif