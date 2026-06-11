// Copyright © 2014 CCP ehf.

#pragma once
#ifndef MemoryOutputHandler_H
#define MemoryOutputHandler_H

class MemoryOutputHandler: public nvtt::OutputHandler
{
public:
	MemoryOutputHandler();

	size_t GetSize() const;
	const void* GetData() const;
protected:
	virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel);
	virtual bool writeData(const void * data, int size);
	virtual void endImage();
private:
	MemoryOutputHandler( const MemoryOutputHandler& ) /* = delete */;
	MemoryOutputHandler& operator=( const MemoryOutputHandler& ) /* = delete */;

	size_t m_streamSize;
	CcpMallocBuffer m_buffer;
};

#endif