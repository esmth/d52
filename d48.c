
/*
 * D48 8048/8041 Disassembler
 * Copyright (C) 1995-2005 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * d48.c - Main File
 *
 * Version 3.3.6 - 01/02/05
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

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	<malloc.h>

#include	"d48.h"
#include	"common.h"
#include	"d48pass.h"
#include	"d48table.h"

//
// Global variables
//

byte	flag41;							// 8041 flag

#include	"dispass0.c"
#include	"dispass3.c"

void usage(void)
{
	printf("\nUsage: d48 [options] <filename>\n"
		"Options may be entered Unix style (-d) or DOS style (/b)\n"
		"\t-1 disassemble 8041 code.\n"
		"\t-a use ascii macro instead of db/defb for text.\n"
		"\t-b force .bin extension on input file.\n"
		"\t-d include address and data in comment field.\n"
		"\t-h force .hex extension on input file.\n"
		"\t   If neither 'b' nor 'h' is specified, D48 will first search\n"
		"\t   for a .hex file, and if not found, then a .bin file.\n"
		"\t-s change 'db' and 'dw' to 'defb' and 'defw'.\n"
		"\t-u output labels, symbols, and mnemonics in upper case.\n"
		"\nOptions may be entered in a freeform fashion as long "
		"as a dash (-) or\n"
		"a slash (/) preceeds any option that preceeds the filename."
		"\nExamples:\n"
		"\td48 filename bd\n"
		"\td48 -d filename b\n"
		"\td48 /b -d filename\n\n");
	exit(GOOD_EXIT);
}

//
// The Main Program
//

int main(int argc, char *argv[])
{
	char	c, *inp;
	int	count;
	int	line;

	printf("\nD48 8048/8041 Disassembler V %d.%d.%d"
			 "\nCopyright (C) 1996-2005 by J. L. Post"
			 "\nReleased under the GNU General Public License\n",
		VERSION, MAJORREV, MINORREV);

	if (argc < 2)
		usage();

/*****************************************
	allocate memory for program and flags
 *****************************************/

	if (!init_memory())
		exit(MEM_ERROR);

	strcpy(defbstr, "db");				// init define byte and word strings
	strcpy(defwstr, "dw");
	strcpy(ascistr, "db");				// init define ascii string

	sym_tab = NULL;						// no symbols or labels yet
	lab_tab = NULL;
	name_tab = NULL;
	fileflag = EITHERFILE;				// assume search for either file type
	hexflag = FALSE;
	upperflag = FALSE;
	flag41 = FALSE;
	dump = FALSE;
	ascii_flag = FALSE;

/*******************************
	find filename in command line
 *******************************/

	for (line=1; line<argc; line++)	// find first non option string
	{
		inp = argv[line];
		c = *inp;

		if (c == '?')						// '?' without preceeding '-' or '/'
			usage();

		if (c == '-' || c == '/')
		{
			if (*(++inp) == '?')			// '?' following '-' or '/'
				usage();
		}
		else
		{										// assume first found is file name
			fileflag = parseFileName(argv[line], ".d48");
			break;
		}
	}

/******************************
	process command line options
 ******************************/

	if (argc > 2)							// test for options
	{
		for (count=1; count<argc; count++)
		{
			inp = argv[count];			// to avoid modifying pointer in argv
			c = (char) toupper(*inp++);

			while (c)
			{
				if (c == '-' || c == '/')
					c = toupper(*inp++);		// skip over option specifier

				if (c == '?')
					usage();

				if (count == line)			// skip if already identified
					break;						// as the file name

				if (c == 'D')					// add data in comment field
					hexflag = 1;
				else if (c == 'U')
					upperflag = 1;
				else if (c == 'B' && fileflag == EITHERFILE)			// binary instead of hex file
				{
					fileflag = BINFILE;
					strcpy(src, baseFileName);
					strcat(src, ".bin");
				}
				else if (c == 'H' && fileflag == EITHERFILE)			// force search for hex file
				{
					fileflag = HEXFILE;
					strcpy(src, baseFileName);
					strcat(src, ".hex");
				}
				else if (c == 'S')			// change db/dw strings
				{
					strcpy(defbstr, "defb");
					strcpy(defwstr, "defw");
					if (!ascii_flag)
						strcpy(ascistr, "defb");
				}
				else if (c == 'A')			// use ascii macro
				{
					strcpy(ascistr, "ascii");
					ascii_flag = TRUE;
				}
				else if (c == '1')			// modify opcode table
				{
					flag41 = 1;
					strcpy(mnemtbl[1].mnem, defbstr);
					optbl[1] = OPT_INVAL;
					strcpy(mnemtbl[2].mnem, "out dbb,a");
					strcpy(mnemtbl[8].mnem, defbstr);
					optbl[8] = OPT_INVAL;
					strcpy(mnemtbl[34].mnem, "in a,dbb");
					optbl[34] = OPT_NONE;
					strcpy(mnemtbl[117].mnem, defbstr);
					optbl[117] = OPT_INVAL;
					strcpy(mnemtbl[128].mnem, defbstr);
					optbl[128] = OPT_INVAL;
					strcpy(mnemtbl[129].mnem, defbstr);
					optbl[129] = OPT_INVAL;
					strcpy(mnemtbl[134].mnem, "jobf ");
					strcpy(mnemtbl[136].mnem, defbstr);
					optbl[136] = OPT_INVAL;
					strcpy(mnemtbl[144].mnem, "mov sts,a");
					strcpy(mnemtbl[145].mnem, defbstr);
					optbl[145] = OPT_INVAL;
					strcpy(mnemtbl[152].mnem, defbstr);
					optbl[152] = OPT_INVAL;
					strcpy(mnemtbl[214].mnem, "jnibf ");
					bctbl[214] = 2;
					optbl[214] = OPT_PAGE;
					strcpy(mnemtbl[229].mnem, "en dma");
					strcpy(mnemtbl[245].mnem, "en flags");
					printf("8041 disassembly\n");
				}

				c = (char) toupper(*inp++);
			}
		}
	}

	readfile(src);			// read disassembly file

// Got the program in data array, now let's go to work...

	symbol_count = 0;
	label_count = 0;
	name_count = 0;
	comment_list = NULL;
	icomment_list = NULL;
	patch_list = NULL;

	pass0();								// read control file
	pass1();								// find internal references
	pass2();								// disassemble to output file
	pass3();								// generate equ's for references
	printf("\nDone\n\n");			// go bye-bye
	return(GOOD_EXIT);
}								//  End of Main

//  end of d48.c
