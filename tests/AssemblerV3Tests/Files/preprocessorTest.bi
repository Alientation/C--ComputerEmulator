;#include "include.binc"
;
;.org $FFFF
; conditional
	.db "M_PI is 3", 0
;M_PI * 2
;*
#ifdef M_PI
	.db "M_PI is defined", 0
	
	
	#ifndef M_FOUR
		.db "M_FOUR is NOT defined", 0
	#elsendef M_TWO
		.db "M_TWO is NOT defined", 0
	#else
		.db "M_FOUR is defined and M_TWO is defined", 0
	#endif
#elsedef M_FOUR
	.db "M_PI is not defined, but M_FOUR is defined", 0
#else
	.db "M_PI and M_FOUR are not defined", 0
#endif*;
;*#macro add (a: DBYTE, b: DBYTE) : DBYTE
	.equ _c a + b
	#macret _c
#macend

#macro multiply (a: DBYTE, b: DBYTE) : DBYTE
	.equ _c a * b
	#macret _c
#macend

#macro math (a: DBYTE, b: DBYTE, c: DBYTE) : DBYTE
	.equ _d ((a * b) + c) / b
	#macret _d
#macend

#invoke multiply(300, 300) output
*;
