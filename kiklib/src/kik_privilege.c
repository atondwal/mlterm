/*
 *	$Id$
 */

#include  "kik_config.h"

#include  <sys/types.h>
#include  <unistd.h>		/* getuid/getgid */


/* --- static variables --- */

static int  euid_is_changed ;
static uid_t  saved_euid ;

static int  egid_is_changed ;
static gid_t  saved_egid ;


/* --- global functions --- */

#if defined(HAVE_SETEUID) && defined(HAVE_GETEUID)

int
kik_priv_change_euid(
	uid_t  uid
	)
{
	if( euid_is_changed)
	{
		/* pretending to succeed */
		
		return  1 ;
	}

	saved_euid = geteuid() ;

	if( seteuid( uid) == 0)
	{
		euid_is_changed = 1 ;
		
		return  1 ;
	}
	else
	{
		return  0 ;
	}
}

int
kik_priv_restore_euid(void)
{
	if( ! euid_is_changed)
	{
		/* pretending to succeed */
		
		return  1 ;
	}

	if( seteuid( saved_euid) == 0)
	{
		euid_is_changed = 0 ;
		
		return  1 ;
	}
	else
	{
		return  0 ;
	}
}

#else

int
kik_priv_change_euid(
	uid_t  uid
	)
{
	return  0 ;
}

int
kik_priv_restore_euid(void)
{
	return  0 ;
}

#endif


#if defined(HAVE_SETEUID) && defined(HAVE_SETEGID)

int
kik_priv_change_egid(
	uid_t  uid
	)
{
	if( egid_is_changed)
	{
		/* pretending to succeed */
		
		return  1 ;
	}

	saved_egid = getegid() ;

	if( setegid( uid) == 0)
	{
		egid_is_changed = 1 ;
		
		return  1 ;
	}
	else
	{
		return  0 ;
	}
}

int
kik_priv_restore_egid(void)
{
	if( ! egid_is_changed)
	{
		/* pretending to succeed */
		
		return  1 ;
	}

	if( setegid( saved_egid) == 0)
	{
		egid_is_changed = 0 ;
		
		return  1 ;
	}
	else
	{
		return  0 ;
	}
}

#else

int
kik_priv_change_egid(
	gid_t  gid
	)
{
	return  0 ;
}

int
kik_priv_restore_egid(void)
{
	return  0 ;
}

#endif
