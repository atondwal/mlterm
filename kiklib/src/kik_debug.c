/*
 *	$Id$
 */

#include  "kik_debug.h"

#include  <stdio.h>
#include  <stdarg.h>
#include  <string.h>    /* strlen */
#include  <unistd.h>	/* getpid */

#include  "kik_mem.h"	/* alloca */
#include  "kik_util.h"	/* DIGIT_STR_LEN */
#include  "kik_conf_io.h"	/* kik_get_user_rc_path */


#if  0
#define  __DEBUG
#endif


/* --- static variables --- */

static char *  log_file_path ;


/* --- static functions --- */

static FILE *
open_msg_file(void)
{
	FILE *  fp ;

	if( log_file_path && ( fp = fopen( log_file_path , "a")))
	{
		return  fp ;
	}

	return  stderr ;
}

static void
close_msg_file(
	FILE *  fp
	)
{
	if( fp != stderr)
	{
		fclose( fp) ;
	}
	else
	{
	#ifdef  USE_WIN32API
		fflush( fp) ;
	#endif
	}
}

static int
debug_printf(
	const char *  prefix ,
	const char *  format ,
	va_list  arg_list
	)
{
	char *  new_format = NULL ;
	int  ret ;
	FILE *  fp ;

	if( ( new_format = alloca( strlen( prefix) + strlen( format) + 1)) == NULL)
	{
		/* error */

		return  0 ;
	}

	sprintf( new_format , "%s%s" , prefix , format) ;
	fp = open_msg_file() ;
	ret = vfprintf( fp , new_format , arg_list) ;
	close_msg_file( fp) ;

	return  ret ;
}


/* --- global functions --- */

/*
 * this is usually used between #ifdef __DEBUG ... #endif
 */
int
kik_debug_printf(
	const char *  format ,
	...
	)
{
	va_list  arg_list ;

	va_start( arg_list , format) ;

	return  debug_printf( "DEBUG: " , format , arg_list) ;
}

/*
 * this is usually used between #ifdef DEBUG ... #endif
 */
int
kik_warn_printf(
	const char *  format ,
	...
	)
{
	va_list  arg_list ;

	va_start( arg_list , format) ;

	return  debug_printf( "WARN: " , format , arg_list) ;
}

/*
 * this is usually used without #ifdef ... #endif
 */
int
kik_error_printf(
	const char *  format ,
	...
	)
{
	va_list  arg_list ;

	va_start( arg_list , format) ;

	return  debug_printf( "*** ERROR HAPPEND *** " , format , arg_list) ;
}

/*
 * for noticing message.
 */
int
kik_msg_printf(
	const char *  format ,
	...
	)
{
	va_list  arg_list ;

	va_start( arg_list , format) ;

	return  debug_printf( "" , format , arg_list) ;
}

int
kik_set_msg_log_file_name(
	const char *  name
	)
{
	char *  p ;

	free( log_file_path) ;
	
	if( name && *name && ( p = alloca( strlen( name) + DIGIT_STR_LEN(pid_t) + 5)))
	{
		pid_t  pid ;

		pid = getpid() ;

		sprintf( p , "%s%d.log" , name , pid) ;
		log_file_path = kik_get_user_rc_path( p) ;
	}
	else
	{
		log_file_path = NULL ;
	}

	return  1 ;
}
