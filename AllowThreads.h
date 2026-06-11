// Copyright © 2014 CCP ehf.

#pragma once
#ifndef AllowThreads_H
#define AllowThreads_H

class AllowThreads
{
public:
	AllowThreads()
	{
#if BLUE_WITH_PYTHON
		m_save = PyEval_SaveThread();
#endif
	}
	~AllowThreads()
	{
#if BLUE_WITH_PYTHON
		PyEval_RestoreThread( m_save );
#endif
	}
private:
#if BLUE_WITH_PYTHON
	PyThreadState* m_save;
#endif
};

#endif