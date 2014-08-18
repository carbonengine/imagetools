////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   June 2014
//    Copyright: CCP 2014
//

#pragma once
#ifndef ImageIOResultBlue_H
#define ImageIOResultBlue_H

namespace Be
{

template <>
struct Result<ImageIO::Result>
{
	Result()
		:m_result( ImageIO::Result::OK )
	{
	}

	Result( ImageIO::Result result )
		:m_result( result )
	{
	}

	Result( ImageIO::Result::Code c )
		:m_result( c )
	{
	}

	Result( ImageIO::Result::Code code, const char* message, ... )
	{
		va_list args;
		va_start( args, message );
		m_result = ImageIO::Result::FormatVAList( code, message, args ); 
	}

	operator bool() const
	{
		return m_result;
	}

	ImageIO::Result m_result;
};

template<> inline bool IsSuccess( const Result<ImageIO::Result>& result )
{
	return result;
}

inline const char* GetErrorMessage( const Result<ImageIO::Result>& result )
{
	return result.m_result.GetErrorMessage().c_str();
}

BLUE_DECLARE_GET_EXCEPTION( Result<ImageIO::Result> );

}

typedef Be::Result<ImageIO::Result> ImageIOResult;
typedef Be::BlueWithStdResult<ImageIOResult>::type StdOrImageIOResult;

#endif