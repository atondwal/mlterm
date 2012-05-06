/*
 *	$Id$
 */

#ifndef  __ML_PTY_H__
#define  __ML_PTY_H__


#include  <kiklib/kik_def.h>	/* USE_WIN32API */
#include  <kiklib/kik_types.h>	/* u_int/u_char */

#ifdef  USE_LIBSSH2
#include  "ml_char_encoding.h"
#endif


typedef struct  ml_pty_event_listener
{
	void *  self ;

	/* Called when ml_pty_delete. */
  	void  (*closed)( void *) ;
#ifdef  USE_WIN32API
  	/* Called when ml_read_pty() is ready. */
  	void  (*read_ready)( void *) ;
#endif

} ml_pty_event_listener_t ;

typedef struct  ml_pty *  ml_pty_ptr_t ;


ml_pty_ptr_t  ml_pty_new( const char *  cmd_path , char **  cmd_argv , char **  env ,
	const char *  host , const char *  pass , const char *  pubkey , const char *  privkey ,
	u_int  cols , u_int  rows) ;

ml_pty_ptr_t  ml_pty_new_with( int  master , int  slave , pid_t  child_pid ,
	u_int  cols , u_int  rows) ;

int  ml_pty_delete( ml_pty_ptr_t  pty) ;

int  ml_pty_set_listener( ml_pty_ptr_t  pty,  ml_pty_event_listener_t *  pty_listener) ;

int  ml_set_pty_winsize( ml_pty_ptr_t  pty , u_int  cols , u_int  rows) ;

size_t  ml_write_to_pty( ml_pty_ptr_t  pty , u_char *  buf , size_t  len) ;

size_t  ml_read_pty( ml_pty_ptr_t  pty , u_char *  buf , size_t  left) ;

pid_t  ml_pty_get_pid( ml_pty_ptr_t  pty) ;

int  ml_pty_get_master_fd( ml_pty_ptr_t  pty) ;

int  ml_pty_get_slave_fd( ml_pty_ptr_t  pty) ;

char *  ml_pty_get_slave_name( ml_pty_ptr_t  pty) ;

#ifdef  USE_LIBSSH2
void *  ml_search_ssh_session( const char *  host , const char *  port , const char *  user) ;

int  ml_pty_use_loopback( ml_pty_ptr_t  pty) ;

int  ml_pty_unuse_loopback( ml_pty_ptr_t  pty) ;

int  ml_pty_ssh_scp( ml_pty_ptr_t  pty , ml_char_encoding_t  pty_encoding ,
	ml_char_encoding_t  path_encoding , char *  dst_path , char *  src_path) ;

void  ml_pty_ssh_set_cipher_list( const char *  list) ;
#endif


#endif
