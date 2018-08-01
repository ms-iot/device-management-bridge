/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include <exception>
#include "Logger.h"

class DMBridgeExceptionWithErrorCode : public std::exception
{
	long _errorCode;
public:
	DMBridgeExceptionWithErrorCode(long errorCode) :
		std::exception(""), _errorCode(errorCode)
	{
		TRACEP("Exception error code: ", errorCode);
	}

	DMBridgeExceptionWithErrorCode(const char* message, long errorCode) :
		std::exception(message), _errorCode(errorCode)
	{
		TRACEP("Exception error code: ", errorCode);
	}

	long ErrorCode() const
	{
		return _errorCode;
	}
};
