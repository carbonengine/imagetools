////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#pragma once
#ifndef FileStream_H
#define FileStream_H

class FileStream: public ICcpStream
{
public:
	enum Mode
	{
		READ,
		WRITE,
	};

	FileStream( const wchar_t* filename, Mode mode );
	~FileStream();

	bool IsValid() const;

	virtual ptrdiff_t Read( void* dest, ptrdiff_t count );
	virtual ptrdiff_t Write( const void* source, size_t count );
	virtual ptrdiff_t Seek( ptrdiff_t distance, SeekOrigin method );
	virtual ptrdiff_t GetPosition();
	virtual ptrdiff_t GetSize();
private:
	FileStream( const FileStream& ) /* = delete */;
	FileStream& operator=( const FileStream& ) /* = delete */;

	FILE* m_file;
};

#endif