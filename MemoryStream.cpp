////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "MemoryStream.h"

MemoryStream::MemoryStream( const void* memory, size_t size )
	:m_memory( memory ),
	m_size( size ),
	m_position( 0 )
{
}

ptrdiff_t MemoryStream::Read( void* dest, ptrdiff_t count )
{
	if( count < 0 )
	{
		count = m_size - m_position;
	}
	count = std::min( count, ptrdiff_t( m_size - m_position ) );
	memcpy( dest, static_cast<const uint8_t*>( m_memory ) + m_position, count );
	m_position += count;
	return count;
}

ptrdiff_t MemoryStream::Write( const void* source, size_t count )
{
	return -1;
}

ptrdiff_t MemoryStream::Seek( ptrdiff_t distance, SeekOrigin method )
{
	switch( method )
	{
	case SO_BEGIN:
		m_position = size_t( std::min( ptrdiff_t( m_size ), std::max( distance, ptrdiff_t( 0 ) ) ) );
		break;
	case SO_CURRENT:
		m_position = std::min( size_t( std::max( ptrdiff_t( m_position ) + distance, ptrdiff_t( 0 ) ) ), m_size );
		break;
	case SO_END:
		m_position = m_size - std::min( size_t( std::max( distance, ptrdiff_t( 0 ) ) ), m_size );
		break;
	default:
		return -1;
	}
	return GetPosition();
}

ptrdiff_t MemoryStream::GetPosition()
{
	return m_position;
}

ptrdiff_t MemoryStream::GetSize()
{
	return m_size;
}
