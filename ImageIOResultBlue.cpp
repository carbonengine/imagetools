////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   June 2014
//    Copyright: CCP 2014
//

#include "StdAfx.h"
#include "ImageIOResultBlue.h"

BLUE_DEFINE_EXCEPTION( ImageIOError, BlueStdRuntimeError );
BLUE_DEFINE_EXCEPTION( UnrecognizedImageTypeError, ImageIOError );
BLUE_DEFINE_EXCEPTION( UnsupportedOperationError, ImageIOError );

namespace Be
{

BLUE_BEGIN_GET_EXCEPTION( Result<ImageIO::Result> )
	switch( result.m_result.code )
	{
	case ImageIO::Result::UNRECOGNIZED_IMAGE_TYPE:
		return BLUE_GET_EXCEPTION( UnrecognizedImageTypeError );
	case ImageIO::Result::METHOD_NOT_SUPPORTED:
		return BLUE_GET_EXCEPTION( UnsupportedOperationError );
	case ImageIO::Result::READ_FAILURE:
	case ImageIO::Result::WRITE_FAILURE:
		return BLUE_GET_EXCEPTION( ImageIOError );
	case ImageIO::Result::HEADER_NOT_SUPPORTED:
		return BLUE_GET_EXCEPTION( ImageIOError );
	case ImageIO::Result::INVALID_DATA:
		return BLUE_GET_EXCEPTION( ImageIOError );
	case ImageIO::Result::ERROR_CREATING_BITMAP:
		return BLUE_GET_EXCEPTION( ImageIOError );
	case ImageIO::Result::INVALID_BITMAP:
		return BLUE_GET_EXCEPTION( BlueStdValueError );
	case ImageIO::Result::SAVE_NOT_SUPPORTED:
		return BLUE_GET_EXCEPTION( UnsupportedOperationError );
	case ImageIO::Result::OUT_OF_MEMORY:
		return BLUE_GET_EXCEPTION( BlueStdMemoryError );
	case ImageIO::Result::ERROR_CONVERTING_FORMAT:
		return BLUE_GET_EXCEPTION( BlueStdMemoryError );
	case ImageIO::Result::ERROR_INITIALIZING_EXTERNAL_LIBRARY:
		return BLUE_GET_EXCEPTION( ImageIOError );
	default:
		return BLUE_GET_EXCEPTION( ImageIOError );
	}
BLUE_END_GET_EXCEPTION()

}
