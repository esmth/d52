
/*
 * D48 8048/8041 Disassembler
 * Copyright (C) 1995-2004 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * d48table.h - 8048 disassembler tables
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
// Global variables
//

extern byte					optbl[256];
extern byte					bctbl[256];
extern struct mnementry	mnemtbl[256];
extern struct sfrentry	rbname[];

// end of d48table.h
