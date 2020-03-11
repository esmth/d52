
/*
 * Z80 Disassembler
 * Copyright (C) 1990-2004 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * dz80pass2.h - Disassembly pass 2
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

//
//	Prototypes
//

extern void	pass2(void);
extern void	invalided(byte k);
extern int	doindex(int i, char index);
extern void	indxld1(int i, char reg, char idx);
extern void	indxld2(int i, char reg, char idx);

//
//	Global variables
//
/* none */

// end of dz80pass2.h

