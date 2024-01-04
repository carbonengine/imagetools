////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2023
//    Copyright: CCP 2023
//

#pragma once

#pragma warning( push )
#pragma warning( disable : 4996 )
#include <NanoVdbSupport.h>
#pragma warning( pop )

BLUE_CLASS( GridEncodingInfo ) :
	public IRoot
{
public:
	EXPOSE_TO_BLUE();

	ImageIO::RasterizeGridInfo m_info;
};

TYPEDEF_BLUECLASS( GridEncodingInfo );


BLUE_CLASS( NanoVDBGridMetadata ) :
	public IRoot
{
public:
	EXPOSE_TO_BLUE();

	ImageIO::NanoVDBGridMetadata m_metadata;
};

TYPEDEF_BLUECLASS( NanoVDBGridMetadata );


BLUE_CLASS( VtaGridInfo ) :
	public IRoot
{
public:
	EXPOSE_TO_BLUE();

	ImageIO::Vta::GridInfo m_info;
};

TYPEDEF_BLUECLASS( VtaGridInfo );


BLUE_CLASS( VtaInfo ) :
	public IRoot
{
public:
	EXPOSE_TO_BLUE();

	std::vector<VtaGridInfoPtr> GetGrids() const
	{
		return m_grids;
	}
	ImageIO::MetadataStrings GetMetadata() const
	{
		return m_metadata;
	}

	std::vector<VtaGridInfoPtr> m_grids;
	uint32_t m_frames = 0;
	uint32_t m_fileVersion = 0;
	ImageIO::MetadataStrings m_metadata;
};

TYPEDEF_BLUECLASS( VtaInfo );
