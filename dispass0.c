
/*
 * Pass 0 for Disassemblers
 * Copyright (C) 1995-2005 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * Version 3.3.6 - 01/23/05
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
// Read control file, if it exists, and flag areas as code, text,
// or whatever. Also handle labels, symbols, etc.
//
// Some strange things happen here with oring and anding on the
// flag bits; this is so the user can define something as code,
// data, or whatever, and assign a label for the same location.
// We should handle the result intelligently regardless of the
// order in which such things are found in the control file.
// Defining a location as code sets the PF_NOINIT bit, which
// might seem strange at first, but this is to prevent pass 2 from
// skipping over multiple NOPs if the user specifically defines
// them as code.
//

#include	"common.h"

void pass0(void)
{
	int			i;
	char			*text, func, c, *ltext;
	int			start, stop, code, temp;
	FILE			*fpc;

	if (upperflag)
	{
		makeupper(defbstr);
		makeupper(defwstr);
		makeupper(ascistr);
		makeupper(orgstr);
		makeupper(equstr);
	}

	fpc = fopen(ctl, "r");

	if (!fpc)
		return;

	if (fpc != NULL)			// if control file exists...
	{
		printf("\nReading control file   ");

		while (!feof(fpc))					// until end of file...
		{
			start = stop = 0;
			*linebuffer = '\0';				// clear previous line
			text = fgets(linebuffer, MAX_LINE - 1, fpc);	// read one line
			i = 0;

			if (!text)
				break;

			while (linebuffer[i] && linebuffer[i] != '\n')
				i++;

			linebuffer[i] = '\0';
			text = &linebuffer[1];

			while (isgraph(*text))			// skip remaining chars in first word
				text++;

			text = get_adrs(text, &start);

			while (1)
			{
				c = *text++;
				if (c != ' ' && c != '\t')	// skip whitespace
					break;
			}

			if (c == '\n' || c == ';')		// if only one numeric...
				--text;							// back up to newline

			func = c;							// save operator
			ltext = text;
			--ltext;
			text = get_adrs(text, &stop);

			if (func == '+')					// check for valid operator
				stop += (start - 1);
			else if (func == '-' && !stop)
				stop = start;

			switch (toupper(linebuffer[0]))
			{
				case 'A':								// address
					if (start < PMEMSIZE && stop < PMEMSIZE)
					{
						do
						{						// get address to reference
#ifdef	CPU_BIG_ENDIAN
							code = (pgmmem[start]) << 8;
							temp = pgmmem[start + 1] & 0xff;
#else
							code = pgmmem[start] & 0xff;
							temp = (pgmmem[start + 1] & 0xff) << 8;
#endif
							code |= temp;

							if (code < PMEMSIZE)
							{
								pgmflags[code] |= PF_REF;		// flag referenced address
								pgmflags[code] &= ~(PF_CLREF | PF_SPLIT);
								pgmflags[start] |= PF_ADRS;	// set flags to adrs code
								pgmflags[start++] &=
									~(PF_NOINIT | PF_WORD | PF_BYTE | PF_ASCII | PF_SPLIT);
								pgmflags[start] |= PF_ADRS;
								pgmflags[start++] &=
									~(PF_NOINIT | PF_WORD | PF_BYTE | PF_ASCII | PF_SPLIT);
							}
							else
							{
								pgmflags[start++] |= PF_WORD;
								pgmflags[start++] |= PF_WORD;
							}
						} while (start < stop);
					}
					else
						printf("\rinvalid address specified: %x, %x\n",
									start, stop);
					break;

				case 'B':								// byte binary
					if (start < PMEMSIZE && stop < PMEMSIZE)
					{
						do
						{
							pgmflags[start] |= PF_BYTE;
							pgmflags[start++] &=
								~(PF_NOINIT | PF_ADRS | PF_WORD | PF_ASCII | PF_SPLIT);
						} while (start <= stop);
					}
					else
						printf("\rinvalid address specified: %x, %x\n",
									start, stop);
					break;

				case 'C':								// code
					if (start < PMEMSIZE && stop < PMEMSIZE)
					{
						do
						{
							pgmflags[start] &=
#ifdef _D48_H_
								~(PF_NOINIT | PF_ADRS | PF_WORD | PF_BYTE | PF_ASCII | PF_SPLIT | PF_MB0 | PF_MB1);
#else
								~(PF_ADRS | PF_WORD | PF_BYTE | PF_ASCII | PF_SPLIT);
#endif
							pgmflags[start++] |= PF_FORCE;
						} while (start <= stop);
					}
					else
						printf("\rinvalid address specified: %x, %x\n",
									start, stop);
					break;

				case 'D':								// define data type
					switch (toupper(*ltext))
					{
						case '0':
						case 'L':
							pgmflags[start] |= PF_LABEL;		// 0 or (l)abel = search label table only
							break;

						case '1':
						case 'S':
							pgmflags[start] |= PF_SYMBOL;		// 1 or (s)ymbol = search symbol table only
							break;

						case '2':
						case 'N':
							pgmflags[start] |= (PF_LABEL | PF_SYMBOL);	// 2 or (n)one = don't search either table
							break;

						default:
							printf("\nInvalid data type flag: %s\n", linebuffer);
							break;
					}

					break;

#ifdef	EXTENDED_MEM
#ifndef _D48_H_

				case 'E':								// extended memory specification
					// incomplete
					break;
#endif
#endif

#ifdef _D52_H_

				case 'F':								// modify SFR name
					if (start < 0x80 || start > 0xff)
					{
						printf("\rInvalid SFR address: 0x%x in '%s'\n",
									start, (char *) &linebuffer[2]);
					}
					else
					{
						start &= 0x7f;

						for (stop=0; stop<15; stop++)	// transfer new name
						{
							func = *ltext++;

							if (isgraph(func))
								sfr[start].dent[stop] = func;
							else
							{
								sfr[start].dent[stop] = '\0';	// terminate name
								break;
							}
						}

						sfrflags[start] = 1;

						if (stop >= 15)
							sfr[start].dent[15] = '\0';
					}
					break;

#endif

				case 'I':								// ignore
					if (start < PMEMSIZE && stop < PMEMSIZE)
					{
						do
						{
							pgmflags[start++] = PF_INIT;
						} while (start <= stop);
					}
					else
						printf("\rinvalid address specified: %x, %x\n",
									start, stop);
					break;

#ifdef _D52_H_

				case 'K':								// modify SFR bit name
					if (start < 0x80 || start > 0xff)
					{
						printf("\rInvalid SFR bit address: 0x%x in '%s'\n",
									start, (char *) &linebuffer[2]);
					}
					else
					{
						start &= 0x7f;

						for (stop=0; stop<15; stop++)	// transfer name
						{
							func = *ltext++;

							if (isgraph(func))
								sfrbits[start].dent[stop] = func;
							else
							{
								sfrbits[start].dent[stop] = '\0';
								break;
							}
						}

						sbflags[start] = 1;

						if (stop >= 15)
							sfrbits[start].dent[15] = '\0';
					}
					break;

#endif

				case 'L':								// label
					if (start < PMEMSIZE)
					{
						pgmflags[start] |= PF_REF;			// flag reference
						pgmflags[start] &= ~(PF_CLREF | PF_SPLIT);

						if (isgraph(*ltext))
							add_entry(start, ltext, LABEL_TYPE);
					}
					else
						printf("\rinvalid address specified: %x, %x\n",
									start, stop);
					break;

#ifdef _D48_H_

				case 'M':								// force memory bank selection
					get_adrs(ltext, &stop);

					if (start)
						pgmflags[stop] |= PF_MB1;
					else
						pgmflags[stop] |= PF_MB0;
					break;

#endif

#ifdef _D52_H_

				case 'M':								// modify memory bit name
					if (start > 0x7f)
					{
						printf("\rInvalid memory bit address: 0x%x in '%s'\n",
									start, (char *) &linebuffer[2]);
					}
					else
					{
						for (stop=0; stop<15; stop++)	// transfer name
						{
							func = *ltext++;

							if (isgraph(func))
							{
								if (!keilflag)
									membits[start].dent[stop] = func;
								else
									keilmembits[start].dent[stop] = func;
							}
							else
							{
								if (!keilflag)
									membits[start].dent[stop] = '\0';
								else
									keilmembits[start].dent[stop] = '\0';
								break;
							}
						}

						mbflags[start] = 1;

						if (stop >= 15)
						{
							if (!keilflag)
								membits[start].dent[15] = '\0';
							else
								keilmembits[start].dent[15] = '\0';
						}
					}
					break;

#endif

				case 'P':								// patch code (add inline code)
					pgmflags[start] |= PF_PATCH;		// flag address
					text = get_adrs((char *) &linebuffer[1], &start);
					add_patch(start, text);
					break;

#ifdef _D48_H_

				case 'R':								// register name
					if (start > 15)
					{
						printf("\rInvalid register address: 0x%x in '%s'\n",
						linebuffer[0], linebuffer);
					}
					else
					{
						for (stop=0; stop<7; stop++)	// transfer register name
						{
							func = *ltext++;

							if (upperflag)
								func = toupper(func);

							if (isalnum(func))
								rbname[start].dent[stop] = func;
							else
							{
								rbname[start].dent[stop] = '\0';
								break;
							}
						}
					}

					if (stop >= 7)
						rbname[start].dent[7] = '\0';	// null terminate reg name
					break;

#endif

#ifdef _D52_H_

				case 'R':								// modify register name
					if (start > 0x7f)
					{
						printf("\rInvalid register/memory address: 0x%x in '%s'\n",
									start, (char *) &linebuffer[2]);
					}
					else
					{
						dirregs[start] |= 2;

						for (stop=0; stop<15; stop++)	// transfer register name
						{
							func = *ltext++;

							if (upperflag)
								func = toupper(func);

							if (isgraph(func))
								rbname[start].dent[stop] = func;
							else
							{
								rbname[start].dent[stop] = '\0';
								break;
							}
						}

						if (stop >= 15)
							rbname[start].dent[15] = '\0';
					}
					break;

#endif

				case 'S':								// symbol
					add_entry(start, ltext, SYMBOL_TYPE);
					break;

				case 'T':								// ascii text
					do
					{
						pgmflags[start] |= PF_ASCII;
						pgmflags[start++] &=
							~(PF_NOINIT | PF_ADRS | PF_WORD | PF_BYTE | PF_SPLIT);
					} while (start <= stop);
					break;

#ifdef	EXTENDED_MEM
#ifndef _D48_H_

				case 'U':								// use extended memory translation
					// incomplete
					break;
#endif
#endif

				case 'W':								// word binary
					if (start < PMEMSIZE && stop < PMEMSIZE)
					{
						do
						{
							pgmflags[start] |= PF_WORD;
							pgmflags[start++] &=
								~(PF_NOINIT | PF_ADRS | PF_BYTE | PF_ASCII | PF_SPLIT);
						} while (start <= stop);
					}
					else
						printf("\rinvalid address specified: %x, %x\n",
									start, stop);
					break;

				case 'X':								// operand name
					pgmflags[start] |= PF_NAME;
					add_entry(start, ltext, NAME_TYPE);
					break;

				case 0x00:								// ignore empty lines
				case '\n':
				case '\r':
				case '\t':
				case ' ':
				case ';':								// ignore commented out lines
					break;

				case '#':								// comment string
					pgmflags[start] |= PF_CMT;		// flag address
					text = get_adrs((char *) &linebuffer[1], &start);
					add_comment(start, text);
					break;

				case '!':								// inline comment string
					pgmflags[start] |= PF_ICMT;
					text = get_adrs((char *) &linebuffer[1], &start);
					add_icomment(start, text);
					break;

				default:						// somebody didn't read the docs...
					printf("\nUnknown control code: 0x%02x in '%s'\n",
						linebuffer[0], linebuffer);
					break;
			}
		}

		if (label_count || symbol_count || name_count)	// set up tail node for sort
		{
			tail_ptr = (struct sym *) malloc(sizeof(struct sym));

			if (tail_ptr == NULL)
			{
				printf("\nNo memory for symbol pointers!\n");
				exit(MEM_ERROR);
			}

			tail_ptr->next = tail_ptr;
			tail_ptr->name = malloc(4);
			tail_ptr->name[0] = 0xfe;		// set max values for sort
			tail_ptr->name[1] = 0;
			tail_ptr->val = 0xfffff;		// set an invalid value to mark end of list
		}

		if (label_count)						// if labels encountered...
		{
			lab_tab_last->next = tail_ptr;	// set up pointer array for sort
			lab_val_index = malloc(sizeof(SYM_PTR) * label_count);

			if (lab_val_index == NULL)
			{
				printf("\nNo memory for label pointers");
				exit(MEM_ERROR);
			}

			lab_tab = sort(lab_tab, lab_val_index, label_count);

			if (!lab_tab)
				exit(USER_ERROR);
		}

		if (symbol_count)						// if symbols encountered...
		{
			sym_tab_last->next = tail_ptr;
			sym_val_index = malloc(sizeof(SYM_PTR) * symbol_count);

			if (sym_val_index == NULL)
			{
				printf("\nNo memory for symbol pointers");
				exit(MEM_ERROR);
			}

			sym_tab = sort(sym_tab, sym_val_index, symbol_count);

			if (!sym_tab)
				exit(USER_ERROR);
		}

		if (name_count)						// if operand symbols encountered...
		{
			name_tab_last->next = tail_ptr;
			name_val_index = malloc(sizeof(SYM_PTR) * name_count);

			if (name_val_index == NULL)
			{
				printf("\nNo memory for operand name pointers!\n");
				exit(MEM_ERROR);
			}

			name_tab = sort(name_tab, name_val_index, name_count);

			if (!name_tab)
				exit(USER_ERROR);
		}

		fclose(fpc);
	}
	else
		printf("No control file found\n\n");
}

// end of pass0.c
