// Error.h:
//
// Errors and exceptions are mutually exclusive.  Errors are problems that
// the user can't do anything about, for instance, bad parameters passed to
// a function or an object getting in to an invalid state.
//
// Errors are negative numbers.  These constants are intended for functions
// that return signed short/int/long.  The non-negative numbers communicate
// some kind of success, or at least the lack of error.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERROR_H__0C25DC05_61D1_4FD0_AFB7_944A12259688__INCLUDED_)
#define AFX_ERROR_H__0C25DC05_61D1_4FD0_AFB7_944A12259688__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


extern short HRR_OK;
extern short HRR_INVALID_PARAMETER;
extern short HRR_INVALID_STATE;
extern short HRR_CREATE_THREAD_FAILURE;
extern short HRR_SUSPEND_THREAD_FAILURE;
extern short HRR_RESUME_THREAD_FAILURE;
extern short HRR_NO_CONTENT_LENGTH;
extern short HRR_NO_THREAD;
extern short HRR_INVALID_RESPONSE;

extern bool IsHrrError( long n );
extern bool IsHrrGood( long n );

#endif // !defined(AFX_ERROR_H__0C25DC05_61D1_4FD0_AFB7_944A12259688__INCLUDED_)
