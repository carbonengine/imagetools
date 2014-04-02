////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "MemoryOutputHandler.h"

MemoryOutputHandler::MemoryOutputHandler()
	:m_streamSize( 0 )
{
}

size_t MemoryOutputHandler::GetSize() const
{
	return m_streamSize;
}

const void* MemoryOutputHandler::GetData() const
{
	return m_buffer.get();
}

void MemoryOutputHandler::beginImage(int size, int width, int height, int depth, int face, int miplevel)
{
}

bool MemoryOutputHandler::writeData(const void * data, int size)
{
	if( size <= 0 )
	{
		return true;
	}
	if( m_streamSize + size > m_buffer.size() )
	{
		const size_t MEGABYTE = 1024 * 1024;
		m_buffer.resize( "MemoryOutputHandler", ( ( m_streamSize + size + MEGABYTE - 1 ) / MEGABYTE ) * MEGABYTE );
		if( !m_buffer )
		{
			return false;
		}
	}
	memcpy( m_buffer.get() + m_streamSize, data, size );
	m_streamSize += size;
	return true;
}

void MemoryOutputHandler::endImage()
{
}
