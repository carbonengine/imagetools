// Copyright © 2024 CCP ehf.

#include "StdAfx.h"

#pragma warning( push )
#pragma warning( disable : 4996 )
#include <NanoVdbSupport.h>
#pragma warning( pop )

#include "NanoVDBToolSupport.h"
#include "ImageToolsBitmap.h"
#include "FileStream.h"
#include "AllowThreads.h"
#include <VtaHandler.h>

const Be::VarChooser VtaEncoding_Chooser[] = {
	{ "NONE", BeCast( ImageIO::Vta::Encoding::None ), "no encoding" },
	{ "RLE_7", BeCast( ImageIO::Vta::Encoding::Rle7 ), "7bit RLE encoding" },
	{ "RLE_7_5", BeCast( ImageIO::Vta::Encoding::Rle7_5 ), "7bit for the first frame and 5bit for subsequent frame diffs RLE encoding" },
	{ "RLE_6", BeCast( ImageIO::Vta::Encoding::Rle6 ), "6bit RLE encoding" },
	{ 0 }
};

BLUE_REGISTER_ENUM_EX(
	"VtaEncoding",
	ImageIO::Vta::Encoding,
	VtaEncoding_Chooser,
	ENUM_REG_ENUM_OBJECT_ON_MODULE );


BLUE_DEFINE( GridEncodingInfo );

const Be::ClassInfo* GridEncodingInfo::ExposeToBlue()
{
	EXPOSURE_BEGIN( GridEncodingInfo, "" )
		MAP_INTERFACE( IRoot )
		MAP_INTERFACE( GridEncodingInfo )

		MAP_ATTRIBUTE( "index", m_info.index, "grid index in the source VDB file", Be::READWRITE );
	MAP_ATTRIBUTE_WITH_CHOOSER( "encoding", m_info.encoding, "grid encoding", Be::READWRITE, VtaEncoding_Chooser );
	MAP_ATTRIBUTE( "format", m_info.format, "grid pixel format", Be::READWRITE );

	EXPOSURE_END()
}


BLUE_DEFINE( NanoVDBGridMetadata );

const Be::ClassInfo* NanoVDBGridMetadata::ExposeToBlue()
{
	EXPOSURE_BEGIN( NanoVDBGridMetadata, "" )
		MAP_INTERFACE( IRoot )
		MAP_INTERFACE( NanoVDBGridMetadata )

		MAP_ATTRIBUTE( "name", m_metadata.name, "grid name", Be::READWRITE );
	MAP_ATTRIBUTE( "width", m_metadata.width, "grid width", Be::READWRITE );
	MAP_ATTRIBUTE( "height", m_metadata.height, "grid height", Be::READWRITE );
	MAP_ATTRIBUTE( "depth", m_metadata.depth, "grid depth", Be::READWRITE );
	MAP_ATTRIBUTE( "type", m_metadata.type, "grid type", Be::READWRITE );

	EXPOSURE_END()
}


BLUE_DEFINE( VtaGridInfo );

const Be::ClassInfo* VtaGridInfo::ExposeToBlue()
{
	EXPOSURE_BEGIN( VtaGridInfo, "" )
		MAP_INTERFACE( IRoot )
		MAP_INTERFACE( VtaGridInfo )

		MAP_ATTRIBUTE( "name", m_info.name, "grid name", Be::READWRITE );
	MAP_ATTRIBUTE( "width", m_info.width, "grid width", Be::READWRITE );
	MAP_ATTRIBUTE( "height", m_info.height, "grid height", Be::READWRITE );
	MAP_ATTRIBUTE( "depth", m_info.depth, "grid depth", Be::READWRITE );
	MAP_ATTRIBUTE( "format", m_info.format, "grid pixel format", Be::READWRITE );
	MAP_ATTRIBUTE( "encoding", m_info.encoding, "grid encoding", Be::READWRITE );

	EXPOSURE_END()
}


BLUE_DEFINE( VtaInfo );

const Be::ClassInfo* VtaInfo::ExposeToBlue()
{
	EXPOSURE_BEGIN( VtaInfo, "" )
		MAP_INTERFACE( IRoot )
		MAP_INTERFACE( VtaInfo )

		MAP_ATTRIBUTE( "frames", m_frames, "number of animation frames in the file", Be::READ );
	MAP_ATTRIBUTE( "version", m_fileVersion, "VTA file version", Be::READ );
	MAP_METHOD_AND_WRAP( "grids", GetGrids, "returns grid information" )
		MAP_METHOD_AND_WRAP( "metadata", GetMetadata, "returns metadata strings" )

		EXPOSURE_END()
}


Be::Result<std::string> RasterizeNanoVDB( const char* vdbPath, uint32_t gridNumber, ImageIO::PixelFormat format, int32_t bpp, ImageToolsBitmapPtr& bitmap )
{
	bitmap.CreateInstance();

	if( !ImageIO::RasterizeNanoVDB( vdbPath, gridNumber, format, bpp, *bitmap ) )
	{
		bitmap = nullptr;
		return Be::Result<std::string>( "Failed" );
	}
	return Be::Result<std::string>();
}

MAP_FUNCTION_AND_WRAP(
	"rasterize_nano_vdb",
	RasterizeNanoVDB,
	"Rasterizes a NanoVDB file into a bitmap.\n"
	":param path: path to a valid NanoVDB file\n"
	":param grid: index of the grid in the NanoVDB file; we only support Float type grids ATM\n"
	":param pixel_format: pixel format to save to: can be either R8_UNORM or R32_FLOAT" );


Be::Result<std::string> NanoVDBToVTA( const std::vector<std::string>& vdbPath, const std::vector<GridEncodingInfo*>& grids, const wchar_t* destPath )
{
	std::vector<ImageIO::RasterizeGridInfo> gridInfo;
	for( auto& grid : grids )
	{
		gridInfo.push_back( grid->m_info );
	}
	auto vta = ImageIO::NanoVDBToVTA( vdbPath, gridInfo );
	if( vta.empty() )
	{
		return Be::Result<std::string>( "Failed" );
	}

	FileStream stream( destPath, FileStream::WRITE );
	stream.Write( vta.data(), vta.size() );
	return Be::Result<std::string>();
}

MAP_FUNCTION_AND_WRAP(
	"nano_vdb_to_vta",
	NanoVDBToVTA,
	"Rasterizes a NanoVDB file into a bitmap.\n"
	":param path: path to a valid NanoVDB file\n"
	":param grids: per-grid options"
	":param dest_path: path to the destination VTA file" );


Be::Result<std::string> DownsampleVTA( const wchar_t* sourcePath, const wchar_t* destPath )
{
	FileStream srcStream( sourcePath, FileStream::READ );
	if( !srcStream.IsValid() )
	{
		return Be::Result<std::string>( "Failed to read srouce file" );
	}
	std::vector<uint8_t> src;
	src.resize( srcStream.GetSize() );
	auto r = srcStream.Read( src.data(), src.size() );
	if( r != ptrdiff_t( src.size() ) )
	{
		return Be::Result<std::string>( "Failed to read srouce file" );
	}

	auto vta = ImageIO::DownsampleVTA( src.data(), src.size() );

	FileStream stream( destPath, FileStream::WRITE );
	stream.Write( vta.data(), vta.size() );
	return Be::Result<std::string>();
}

MAP_FUNCTION_AND_WRAP(
	"downsample_vta",
	DownsampleVTA,
	"Downsamples source VTA file.\n"
	":param src_path: path to a valid VTA file\n"
	":param dest_path: path to the destination VTA file" );


Be::Result<std::string> GetNanoVDBMetaData( const char* filename, std::vector<NanoVDBGridMetadataPtr>& result )
{
	std::vector<ImageIO::NanoVDBGridMetadata> md;
	if( !ImageIO::GetNanoVDBMetaData( filename, md ) )
	{
		return Be::Result<std::string>( "Failed to read from file" );
	}
	for( auto& m : md )
	{
		NanoVDBGridMetadataPtr metadata;
		metadata.CreateInstance();
		metadata->m_metadata = m;
		result.push_back( metadata );
	}
	return {};
}

MAP_FUNCTION_AND_WRAP(
	"get_nano_vdb_meta_data",
	GetNanoVDBMetaData,
	"Reads meta-data from a NanoVDB file. Returns a list of grid meta-data.\n"
	":param path: path to a NanoVDB file" );

StdOrImageIOResult LoadVtaInfo( const wchar_t* path, VtaInfoPtr& info )
{
	AllowThreads allowThreads;

	FileStream stream( path, FileStream::READ );
	if( !stream.IsValid() )
	{
		return BlueStdResult( BLUE_STD_RESULT_IO_ERROR, "failed to open file" );
	}

	ImageIO::Vta::FileReader reader;
	auto result = reader.SetStream( &stream );
	if( !result )
	{
		return StdOrImageIOResult( result );
	}

	info.CreateInstance();
	info->m_fileVersion = reader.GetVersion();
	info->m_frames = reader.GetFrameCount();
	info->m_metadata = reader.GetMetadata();
	for( uint32_t i = 0; i < reader.GetGridCount(); ++i )
	{
		VtaGridInfoPtr grid;
		grid.CreateInstance();
		grid->m_info = reader.GetGridInfo( i );
		info->m_grids.push_back( grid );
	}
	return {};
}

MAP_FUNCTION_AND_WRAP(
	"load_vta_info",
	LoadVtaInfo,
	"Reads header information from a VTA file.\n"
	":param path: path to a VTA file" );
