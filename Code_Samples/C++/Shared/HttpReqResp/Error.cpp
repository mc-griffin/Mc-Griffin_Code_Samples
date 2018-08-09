// Error.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Error.h"


short HRR_OK = 0;
short HRR_INVALID_PARAMETER = -1;
short HRR_INVALID_STATE = -2;
short HRR_CREATE_THREAD_FAILURE = -3;
short HRR_SUSPEND_THREAD_FAILURE = -4;
short HRR_RESUME_THREAD_FAILURE = -5;
short HRR_NO_CONTENT_LENGTH = -6;
short HRR_NO_THREAD = -7;
short HRR_INVALID_RESPONSE = -8;


bool IsHrrError( long n )
{
	return n < 0;
}


bool IsHrrGood( long n )
{
	return !IsHrrError( n );
}