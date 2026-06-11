// Copyright © 2014 CCP ehf.

#pragma once
#ifndef MemoryStream_H
#define MemoryStream_H

class MemoryStream: public ICcpStream
{
public:
	MemoryStream( const void* memory, size_t size );

	virtual ptrdiff_t Read( void* dest, ptrdiff_t count );
	virtual ptrdiff_t Write( const void* source, size_t count );
	virtual ptrdiff_t Seek( ptrdiff_t distance, SeekOrigin method );
	virtual ptrdiff_t GetPosition();
	virtual ptrdiff_t GetSize();
private:
	MemoryStream( const MemoryStream& ) /* = delete */;
	MemoryStream& operator=( const MemoryStream& ) /* = delete */;

	const void* m_memory;
	size_t m_size;
	size_t m_position;
};

#endif