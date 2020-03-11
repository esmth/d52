
/*
 * Disassembler common definitions
 * Copyright (C) 2005 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * defs.h - Disassmbler constants and structures
 *
 * Version 3.3.6- 01/18/05
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *	GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef _DEFS_H_
#define _DEFS_H_

//		Defined Constants

#define	VERSION	3
#define	MAJORREV	3
#define	MINORREV	6

#ifndef	TRUE
#define	TRUE		1
#endif

#ifndef	FALSE
#define	FALSE		0
#endif

#define	byte		unsigned char
#define	bool		int

#define	WORD_MASK	0xffff

#define	FN_LEN	256
#define	MAX_LINE	256

#define	TSTOP		32			// tab stop
#define	ASTOP		56			// ascii stop
#define	XSTOP		49			// hex stop

#define	PMEMSIZE	65536		// program memory size

#define	EITHERFILE	0
#define	HEXFILE		1
#define	BINFILE		2
#define	CPMFILE		3

#define	GOOD_EXIT		0
#define	MEM_ERROR		1
#define	FILE_ERROR		2
#define	USER_ERROR		3
#define	PROGRAM_ERROR	4

#define	SYMBOL_TYPE		0
#define	LABEL_TYPE		1
#define	NAME_TYPE		2

#define	ASCLINE		32		// max length of ascii defb line
#define	ASCLIMIT		512	// size of ascii buffer
#define	BYTELINE		8		// max length of binary defb line
#define	BYTELIMIT	512	// size of byte binary buffer
#define	WORDLINE		6		// max length of defw line
#define	WORDLIMIT	512	// size of word binary buffer
#define	NO_DATA		0xff	// pgmmem uninitialized value

// pgmflags bits:

#define	PF_INIT		0x000ff	// initial value
#define	PF_DATA		0x00000	// data valid from file

#define	PF_LABGEN	0x40000	// label generated in pass 2
#define	PF_PATCH		0x20000	// patch flag
#define	PF_MB0		0x10000	// force memory bank 0 reference
#define	PF_MB1		0x08000	// force memory bank 1 reference
#define	PF_2BYTE		0x04000	// 2 byte operand for name
#define	PF_FORCE		0x02000	// force code disassembly
#define	PF_ICMT		0x01000	// inline comment
#define	PF_NAME		0x00800	// search data name table
#define	PF_LABEL		0x00400	// search only label table
#define	PF_SYMBOL	0x00200	// search only symbol table
#define	PF_CMT		0x00100	// comment flag
#define	PF_NOINIT	0x00080	// set if uninitialized space
#define	PF_ADRS		0x00040	// address data
#define	PF_WORD		0x00020	// word binary data
#define	PF_BYTE		0x00010	// byte binary data
#define	PF_CLREF		0x00008	// clear if referenced
#define	PF_ASCII		0x00004	// ascii text
#define	PF_SPLIT		0x00002	// set if split opcode
#define	PF_REF		0x00001	// set if referenced

//
//		Structure Definitions
//

// symbol table and label table entries

struct sym {
	struct sym	*next;
	int			val;
	byte			used;
	char			*name;
};

typedef struct sym *	SYM_PTR;

// comment structure

struct comment {
	int	adrs;
	char	*str;
	struct comment	*next;
};

typedef struct comment *	COMMENT_PTR;

// mnemonic table entries

struct mnementry {
   char mnem[16];
};

// special function register name entries

struct sfrentry {
   char dent[16];
};

#endif	// _DEFS_H_

// end of defs.h

