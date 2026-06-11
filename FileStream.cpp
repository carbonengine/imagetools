// Copyright © 2014 CCP ehf.

#include "StdAfx.h"
#include "FileStream.h"


FileStream::FileStream( const wchar_t* filename, Mode mode )
{
#ifdef _WIN32
	if( _wfopen_s( &m_file, filename, mode == READ ? L"rb" : L"wb" ) )
	{
		m_file = nullptr;
	}
#else
	m_file = fopen( CW2A( filename ), mode == READ ? "rb" : "wb" );
#endif
}

FileStream::~FileStream()
{
	if( m_file )
	{
		fclose( m_file );
	}
}

bool FileStream::IsValid() const
{
	return m_file != nullptr;
}

ptrdiff_t FileStream::Read( void* dest, ptrdiff_t count )
{
	if( m_file )
	{
		return fread( dest, 1, count < 0 ? GetSize() - GetPosition() : count, m_file );
	}
	else
	{
		return -1;
	}
}

ptrdiff_t FileStream::Write( const void* source, size_t count )
{
	if( m_file )
	{
		return fwrite( source, 1, count, m_file );
	}
	else
	{
		return -1;
	}
}

ptrdiff_t FileStream::Seek( ptrdiff_t distance, SeekOrigin method )
{
	if( m_file )
	{
		int origin;
		switch( method )
		{
		case SO_BEGIN:
			origin = SEEK_SET;
			break;
		case SO_CURRENT:
			origin = SEEK_CUR;
			break;
		case SO_END:
			origin = SEEK_END;
			break;
		default:
			return -1;
		}
		if( fseek( m_file, long( distance ), origin ) )
		{
			return -1;
		}
		return GetPosition();
	}
	else
	{
		return -1;
	}
}

ptrdiff_t FileStream::GetPosition()
{
	if( m_file )
	{
		return ftell( m_file );
	}
	else
	{
		return -1;
	}
}

ptrdiff_t FileStream::GetSize()
{
	if( m_file )
	{
		auto fp = ftell( m_file );
		fseek( m_file, 0, SEEK_END );
		auto size = ftell( m_file );
		fseek( m_file, fp, SEEK_SET );
		return size;
	}
	else
	{
		return -1;
	}
}
