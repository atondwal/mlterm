/*
 *	$Id$
 */

#include  "kik_pty.h"

#include  <termios.h>		/* tcset|tcget */
#include  <fcntl.h>
#include  <grp.h>
#include  <stdio.h>
#include  <string.h>
#include  <errno.h>
#include  <sys/param.h>
#include  <sys/ioctl.h>
#include  <sys/stat.h>
#include  <unistd.h>

#include  "kik_str.h"		/* strdup */
#include  "kik_debug.h"


/* Disable special character functions */
#ifdef  _POSIX_VDISABLE
#define  VDISABLE _POSIX_VDISABLE
#else
#define  VDISABLE 255
#endif


/* --- static functions --- */

static int
login_tty(
	int fd
	)
{
	setsid() ;
	
	if( ioctl( fd , TIOCSCTTY , (char*)NULL) == -1)
	{
		return 0 ;
	}
	
	dup2( fd , 0) ;
	dup2( fd , 1) ;
	dup2( fd , 2) ;
	
	if( fd > STDERR_FILENO)
	{
		close(fd) ;
	}
	
	return  1 ;
}

static int
open_pty(
	int *  master ,
	int *  slave ,
	char **  slave_name
	)
{
	char  name[] = "/dev/XtyXX" ;
	char *  p1 ;
	char *  p2 ;
	gid_t  ttygid ;
	struct group *  gr ;

	if( ( gr = getgrnam( "tty")) != NULL)
	{
		ttygid = gr->gr_gid ;
	}
	else
	{
		ttygid = (gid_t) -1 ;
	}

	for( p1 = "pqrstuvwxyzPQRST" ; *p1 ; p1++)
	{
		name[8] = *p1 ;
		
		for( p2 = "0123456789abcdef" ; *p2 ; p2++)
		{
			name[5] = 'p';
			name[9] = *p2 ;
			
			if( ( *master = open( name , O_RDWR , 0)) == -1)
			{
				if( errno == ENOENT)
				{
					/* out of ptys */
					
					return  0 ;
				}
			}
			else
			{
				/*
				 * we succeeded to open pty master.
				 * opening pty slave in succession. 
				 */
				 
				name[5] = 't' ;
				
				chown( name, getuid(), ttygid) ;
				chmod( name, S_IRUSR|S_IWUSR|S_IWGRP) ;
				
				if( ( *slave = open( name, O_RDWR, 0)) != -1)
				{
					if( ( *slave_name = strdup( name)) == NULL)
					{
						close( *slave) ;
					}
					else
					{
						return  1 ;
					}
				}

				close( *master);
			}
		}
	}

	return  0 ;
}


/* --- global functions --- */

/*
 * slave_name memory must be freed by a caller.
 */
pid_t
kik_pty_fork(
	int *  master ,
	char **  slave_name
	)
{
	int  slave ;
	pid_t pid ;
	struct termios  tio ;
	int  fd ;

	if( ! open_pty( master , &slave , slave_name))
	{
		return  -1 ;
	}

	pid = fork() ;
	if( pid == -1)
	{
		/* fork failed */

		free( *slave_name) ;
		
		return  -1 ;
	}
	else if( pid == 0)
	{
		/*
		 * child process
		 */

		close( *master) ;
		
		login_tty( slave) ;
		
		return  0 ;
	}
	
	/*
	 * parent process
	 */
	 
	close(slave) ;

	/*
	 * delaying.
	 */
	fcntl( *master , F_SETFL , O_NDELAY) ;

	/*
	 * terminal attributes.
	 */
	 
	tio.c_iflag = BRKINT | IGNPAR | ICRNL | IXON ;
	tio.c_oflag = OPOST | ONLCR ;
	tio.c_cflag = CS8 | CREAD ;
	tio.c_lflag = ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK ;
#ifdef  ECHOKE
	tio.c_lflag |= ECHOKE ;
#endif
#ifdef  ECHOCTL
	tio.c_lflag |= ECHOCTL ;
#endif

	tio.c_cc[VEOF] = CEOF ;
	tio.c_cc[VEOL] = VDISABLE ;
	tio.c_cc[VERASE] = CERASE ;
	tio.c_cc[VINTR] = CINTR ;
	tio.c_cc[VKILL] = CKILL ;
	tio.c_cc[VQUIT] = CQUIT ;
	tio.c_cc[VSTART] = CSTART ;
	tio.c_cc[VSTOP] = CSTOP ;
	tio.c_cc[VSUSP] = CSUSP ;

#ifdef VDSUSP
	tio.c_cc[VDSUSP] = CDSUSP ;
#endif
#ifdef VREPRINT
	tio.c_cc[VREPRINT] = CRPRNT ;
#endif
#ifdef VRPRNT
	tio.c_cc[VRPRNT] = CRPRNT ;
#endif
#ifdef VDISCARD
	tio.c_cc[VDISCARD] = CFLUSH ;
#endif
#ifdef VFLUSHO
	tio.c_cc[VFLUSHO] = CFLUSH ;
#endif
#ifdef VWERASE
	tio.c_cc[VWERASE] = CWERASE ;
#endif
#ifdef VLNEXT
	tio.c_cc[VLNEXT] = CLNEXT ;
#endif
#ifdef VEOL2
	tio.c_cc[VEOL2] = VDISABLE ;
#endif
#ifdef VSWTC
	tio.c_cc[VSWTC] = VDISABLE ;
#endif
#ifdef VSWTCH
	tio.c_cc[VSWTCH] = VDISABLE ;
#endif
/*
 * VMIN == VEOF and VTIME == VEOL on old System V.
 */
#if VMIN != VEOF
	tio.c_cc[VMIN] = 1 ;
#endif
#if VTIME != VEOL
	tio.c_cc[VTIME] = 0 ;
#endif

	/*
	 * inheriting tty(0,1,2) settings ...
	 */
	
	for( fd = 0 ; fd <= 2 ; fd ++)
	{
		struct termios  def_tio ;
		
		if( tcgetattr( fd , &def_tio) == 0)
		{
			tio.c_cc[VEOF] = def_tio.c_cc[VEOF] ;
			tio.c_cc[VEOL] = def_tio.c_cc[VEOL] ;
			tio.c_cc[VERASE] = def_tio.c_cc[VERASE] ;
			tio.c_cc[VINTR] = def_tio.c_cc[VINTR] ;
			tio.c_cc[VKILL] = def_tio.c_cc[VKILL] ;
			tio.c_cc[VQUIT] = def_tio.c_cc[VQUIT] ;
			tio.c_cc[VSTART] = def_tio.c_cc[VSTART] ;
			tio.c_cc[VSTOP] = def_tio.c_cc[VSTOP] ;
			tio.c_cc[VSUSP] = def_tio.c_cc[VSUSP] ;

			break ;
		}
	}

	cfsetispeed( &tio , B9600) ;
	cfsetospeed( &tio , B9600) ;

	if( tcsetattr( *master , TCSANOW , &tio) < 0)
	{
	#ifdef  DEBUG
		kik_warn_printf( KIK_DEBUG_TAG " tcsetattr() failed.\n") ;
	#endif
	}
	
	return  pid ;
}
