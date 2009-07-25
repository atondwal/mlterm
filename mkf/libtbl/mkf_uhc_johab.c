/*
 *	$Id$
 */

#include  "../lib/mkf_ko_kr_map.h"

#include  "table/mkf_johab_to_uhc.table"
#include  "table/mkf_uhc_to_johab.table"


/* --- global functions --- */

int
mkf_map_johab_to_uhc(
	mkf_char_t *  uhc ,
	mkf_char_t *  johab
	)
{
	u_int16_t  johab_code ;
	u_int16_t  c ;

	johab_code = mkf_char_to_int( johab) ;
	
	if( ( c = CONV_JOHAB_TO_UHC(johab_code)))
	{
		mkf_int_to_bytes( uhc->ch , 2 , c) ;
		uhc->size = 2 ;
		uhc->cs = UHC ;
		
		return  1 ;
	}
	
	return  0 ;
}

int
mkf_map_uhc_to_johab(
	mkf_char_t *  johab ,
	mkf_char_t *  uhc
	)
{
	u_int16_t  uhc_code ;
	u_int16_t  c ;

	uhc_code = mkf_char_to_int( uhc) ;

	if( ( c = CONV_UHC_TO_JOHAB(uhc_code)))
	{
		mkf_int_to_bytes( johab->ch , 2 , c) ;
		johab->size = 2 ;
		johab->cs = JOHAB ;

		return  1 ;
	}
	
	return  0 ;
}