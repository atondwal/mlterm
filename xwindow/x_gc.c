/*
 *	$Id$
 */

#include  "x_gc.h"

#include  <kiklib/kik_mem.h>	/* malloc */

#include  "x_color.h"


/* --- global functions --- */

x_gc_t *
x_gc_new(
	Display *  display
	)
{
	x_gc_t *  gc ;
#ifndef  USE_WIN32GUI
	XGCValues  gc_value ;
#endif

	if( ( gc = malloc( sizeof( x_gc_t))) == NULL)
	{
		return  NULL ;
	}

	gc->display = display ;
	gc->fg_color = RGB_BLACK ;
	gc->bg_color = RGB_WHITE ;
	gc->fid = None ;
	
#ifdef  USE_WIN32GUI
	gc->gc = None ;
	gc->pen = None ;
	gc->brush = None ;
#else
	gc_value.graphics_exposures = 0 ;
	gc->gc = XCreateGC( gc->display, DefaultRootWindow( gc->display),
			GCGraphicsExposures, &gc_value) ;
#endif

	return  gc ;
}

int
x_gc_delete(
	x_gc_t *  gc
	)
{
#ifndef  USE_WIN32GUI
	XFreeGC( gc->display , gc->gc) ;
#endif

	free( gc) ;

	return  1 ;
}

#ifdef  USE_WIN32GUI

int
x_set_gc(
	x_gc_t *  gc,
	GC  _gc
	)
{
	gc->gc = _gc ;

	SetTextAlign( gc->gc, TA_LEFT|TA_BASELINE) ;
	
	gc->fg_color = RGB_BLACK ;
#if  0
	/* RGB_BLACK is default value */
	SetTextColor( gc->gc, RGB_BLACK) ;
#endif

	gc->bg_color = RGB_WHITE ;
#if  0
	/* RGB_WHITE is default value */
	SetBkColor( gc->gc, RGB_WHITE) ;
#endif

	gc->fid = None ;
	gc->pen = None ;
	gc->brush = None ;

	return  1 ;
}

int
x_gc_set_fg_color(
	x_gc_t *  gc ,
	u_long  fg_color
	)
{
	if( fg_color != gc->fg_color)
	{
		SetTextColor( gc->gc, fg_color) ;
		gc->fg_color = fg_color ;
	}

	return  1 ;
}

int
x_gc_set_bg_color(
	x_gc_t *  gc ,
	u_long  bg_color
	)
{
	if( bg_color != gc->bg_color)
	{
		SetBkColor( gc->gc, bg_color) ;
		gc->bg_color = bg_color ;
	}

	return  1 ;
}

int
x_gc_set_fid(
	x_gc_t *  gc,
	Font  fid
	)
{
	if( gc->fid != fid)
	{
		SelectObject( gc->gc, fid) ;
		gc->fid = fid ;
	}

	return  1 ;
}

int
x_gc_set_pen(
	x_gc_t *  gc,
	HPEN  pen
	)
{
	if( gc->pen != pen)
	{
		SelectObject( gc->gc, pen) ;
	}
	
	return  1 ;
}

int
x_gc_set_brush(
	x_gc_t *  gc,
	HBRUSH  brush
	)
{
	if( gc->brush != brush)
	{
		SelectObject( gc->gc, brush) ;
	}
	
	return  1 ;
}

#else

int
x_gc_set_fg_color(
	x_gc_t *  gc ,
	u_long  fg_color
	)
{
	if( fg_color != gc->fg_color)
	{
		XSetForeground( gc->display , gc->gc , fg_color) ;
		gc->fg_color = fg_color ;
	}

	return  1 ;
}

int
x_gc_set_bg_color(
	x_gc_t *  gc ,
	u_long  bg_color
	)
{
	if( bg_color != gc->bg_color)
	{
		XSetBackground( gc->display , gc->gc , bg_color) ;
		gc->bg_color = bg_color ;
	}

	return  1 ;
}

int
x_gc_set_fid(
	x_gc_t *  gc,
	Font  fid
	)
{
	if( gc->fid != fid)
	{
		XSetFont( gc->display , gc->gc , fid) ;
		gc->fid = fid ;
	}

	return  1 ;
}

#endif