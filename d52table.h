
/*
 * D52 8052 Disassembler
 * Copyright (C) 1995-2004 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * d52table.h - 8052 disassembly tables
 *
 * Version 3.3.5 - 11/27/04
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
 
#ifndef	_D52TABLE_H_
#define	_D52TABLE_H_

//		Defined Constants

/* none */

//		Prototypes

/* none */

//		Global Variables

extern struct mnementry	mnemtbl[256];
extern unsigned char		opttbl[256];
extern struct sfrentry	sfr[128];
extern struct sfrentry	keilsfr[128];
extern struct sfrentry	sfrbits[128];
extern struct sfrentry	keilsfrbits[128];
extern struct sfrentry	membits[128];
extern struct sfrentry	keilmembits[128];
extern struct sfrentry	rbname[128];

#endif	// _D52TABLE_H_

// end of d52table.h

