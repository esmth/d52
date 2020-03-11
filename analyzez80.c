
/*
 * DZ80 Z80 Disassembler
 * Copyright (C) 1995-2004 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * analyzez80.c
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

//////// WARNING! This code is still under development. Use at your own risk.

#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<malloc.h>
#include	<time.h>

#include	"dz80.h"
#include	"common.h"
#include	"analyze.h"
#include	"dz80table.h"

// Defined Constants

#define	STACK_DEPTH			128
#define	MIN_STR_LEN			5		// minimum number of characters to ID a string
#define	TRACE_CHECK_LEN	4		// number of code bytes to check for valid code

#define	ANALYZE_NONE		0x00
#define	ANALYZE_TRACED		0x01
#define	ANALYZE_CODE		0x02
#define	ANALYZE_VECTOR		0x04
#define	ANALYZE_BINARY		0x08
#define	ANALYZE_ASCII		0x10
#define	ANALYZE_TAGGED		(ANALYZE_TRACED | ANALYZE_CODE)
#define	ANALYZE_END			0x80

#define	OPCODE_JP			0xc3
#define	OPCODE_JPC			0xda
#define	OPCODE_JPNC			0xd2
#define	OPCODE_JPZ			0xca
#define	OPCODE_JPNZ			0xc2
#define	OPCODE_JPPE			0xea
#define	OPCODE_JPPO			0xe2
#define	OPCODE_JPM			0xfa
#define	OPCODE_JPP			0xf2

#define	OPCODE_JPHL			0xe9
#define	OPCODE_JPIX			0xdde9
#define	OPCODE_JPIY			0xfde9

#define	OPCODE_JR			0x18
#define	OPCODE_JRC			0x38
#define	OPCODE_JRNC			0x30
#define	OPCODE_JRZ			0x28
#define	OPCODE_JRNZ			0x20

#define	OPCODE_DJNZ			0x10

#define	OPCODE_CALL			0xcd
#define	OPCODE_CALLC		0xdc
#define	OPCODE_CALLNC		0xd4
#define	OPCODE_CALLZ		0xcc
#define	OPCODE_CALLNZ		0xc4
#define	OPCODE_CALLPE		0xec
#define	OPCODE_CALLPO		0xe4
#define	OPCODE_CALLM		0xfc
#define	OPCODE_CALLP		0xf4

#define	OPCODE_RST00		0xc7
#define	OPCODE_RST08		0xcf
#define	OPCODE_RST10		0xd7
#define	OPCODE_RST18		0xdf
#define	OPCODE_RST20		0xe7
#define	OPCODE_RST28		0xef
#define	OPCODE_RST30		0xf7
#define	OPCODE_RST38		0xff

#define	OPCODE_RET			0xc9
#define	OPCODE_RETC			0xd8
#define	OPCODE_RETNC		0xd0
#define	OPCODE_RETZ			0xc8
#define	OPCODE_RETNZ		0xc0
#define	OPCODE_RETPE		0xe8
#define	OPCODE_RETPO		0xe0
#define	OPCODE_RETM			0xf8
#define	OPCODE_RETP			0xf0

#define	OPCODE_RETI			0xed4d
#define	OPCODE_RETN			0xed45

#define	OPCODE_PUSHAF		0xf5
#define	OPCODE_PUSHBC		0xc5
#define	OPCODE_PUSHDE		0xd5
#define	OPCODE_PUSHHL		0xe5
#define	OPCODE_PUSHIX		0xdde5
#define	OPCODE_PUSHIY		0xfde5

#define	OPCODE_POPAF		0xf1
#define	OPCODE_POPBC		0xc1
#define	OPCODE_POPDE		0xd1
#define	OPCODE_POPHL		0xe1
#define	OPCODE_POPIX		0xdde1
#define	OPCODE_POPIY		0xfde1

typedef struct strlist {
	char				*str;
	struct strlist	*prev;
	struct strlist	*next;
} STRLIST;

// Global variables

byte	*analysisFlags = NULL;

int	tpc;					// trace pc
int	pushLevel = 0;
int	astackPtr = 0;
int	astack[STACK_DEPTH];		// analysis stack, for returns and branches
int	vstackPtr = 0;
int	vstack[STACK_DEPTH];		// possible vector references stack
char	alertMessage[128];

// for test only
int pushLevelMax = 0;
int astackMax = 0;
int vstackMax = 0;

FILE	*ctlfp;
int	listCount = 0;
char	fileName[256];
char	fileExt[128];
char	tempString[128];
char	dateString[64];

STRLIST	*ctlLineList;

// Local prototypes

bool	analyze(void);
bool	aPass1(void);
bool	aPass2(void);
bool	trace(int pc);					// trace a single thread of code
bool	isString(int pc, int stop);
int	getEndOfString(int pc, int stop);
bool	isTraceableCode(int pc);	// determine if data is executable code
void	genAnalysisList(void);		// generate list to be written to config file
int	writeCtlFile(void);
void	deleteLineList(void);
int	createLineList(void);
bool	addListEntry(char *str);
void	analysisError(char *msg);

// Code

/*	User has asked to analyze the program.

	Trace code from reset address (0) and all interrupt vector addresses,
	and flag all traced data as code [analyze()].

	Then attempt to identify the data type (code, binary, ascii)
	of any non-traced code, if it appears to be code, then trace from
	the code address [aPass1()].

	Now attempt to identify the type of any remaining data and
	flag what it appears to be in analysisFlags array [aPass2()].

	Finally, generate the output for the configuration file based on
	the data types identified and stored in the analysisFlags array
	[genAnalysisList()], and write it to the ctl file[writeCtlFile()].
*/

bool analyzeCode(void)
{
	int	i;

	if (!analysisFlags)
		analysisFlags = (byte *) malloc(PMEMSIZE);

	if (!analysisFlags)
	{
		analysisError("No memory for analysis flags\n");
		return FALSE;
	}

	for (i=0; i<PMEMSIZE; i++)
		analysisFlags[i] = ANALYZE_NONE;

	if (strlen(src))			// have to load a file before analyzing
	{
		if (createLineList() < 1)		// set up header for output list
		{
			printf("\nCan't init analysis data structures!\n");
			return FALSE;
		}

		if (analyze())			// trace code from reset and interrupt vectors
			return FALSE;

		if (aPass1())			// attempt to identify data types
			return FALSE;

//		if (aPass2())			// aPass1() might have identified vectors, so check data again
//			return FALSE;

		genAnalysisList();	// generate control data
		writeCtlFile();		// write control data to ctl file
		deleteLineList();
	}
	else							// no source file selected
	{
		printf("\nNo file to analyze!\n");
		return FALSE;
	}

//printf("push level %d, max %d\n", pushLevel, pushLevelMax);
//printf("astack %d, max %d\n", astackPtr, astackMax);
//printf("vstack %d, max %d\n", vstackPtr, vstackMax);

	return TRUE;
}

// Trace execution from reset address and interrupt vectors.
// Return TRUE if error, else return FALSE

bool analyze(void)
{
	int	i;
	int	dptr;
	int	vector, adrs;

	vstackPtr = 0;

	if (trace(offset))		// trace from reset address
		return TRUE;

	if (offset == 0)
	{
		if (trace(0x08))			// trace reset vectors
			return TRUE;
		if (trace(0x10))
			return TRUE;
		if (trace(0x18))
			return TRUE;
		if (trace(0x20))
			return TRUE;
		if (trace(0x28))
			return TRUE;
		if (trace(0x30))
			return TRUE;
		if (trace(0x38))
			return TRUE;
		if (trace(0x66))			// trace NMI vector
			return TRUE;
	}

//printf("Initial trace done\n");

// Check the vector stack for possible code referenced as vectors.

	if (vstackPtr)
	{
		for (i=0; i<vstackPtr; i++)
		{
			dptr = vstack[i];

			if (dptr < himark)
			{
				while (1)
				{
					if (analysisFlags[dptr] != ANALYZE_NONE)
						break;

					adrs = dptr;
					vector = ((int) pgmmem[dptr++] << 8);
					vector |= ((int) pgmmem[dptr++] & 0xff);
					vector &= WORD_MASK;

					if (vector < himark)
					{
						analysisFlags[adrs] = ANALYZE_VECTOR | ANALYZE_TRACED;
						analysisFlags[adrs + 1] = ANALYZE_VECTOR | ANALYZE_TRACED;

//printf("tracing vector at 0x%04x\n", vector);

						if (trace(vector))
							return TRUE;
					}
					else
						break;

					if (dptr >= himark)
						break;
				}
			}
		}
	}

	return FALSE;
}

// Attempt to determine the type of non-code data.

bool aPass1(void)
{
	int	i, j;
	int	start, stop, pc;
	int	adrs, begin;
	byte	aflag, lastflag, data, lastdata;
	char	code;

	start = stop = begin = 0;
	lastflag = analysisFlags[0];

	for (pc=0, i=0; i<himark; i++)
	{
		aflag = analysisFlags[i];

		if (aflag != lastflag)		// we've got a type change...
		{
			stop = i - 1;
			code = 0;						// no output type yet

			switch (lastflag)
			{
				case ANALYZE_NONE:		// space not flagged, what kind of data is it?
					begin = start;
					pc = start;
					code = 'b';				// assume it's binary data

					while (pc <= stop)			// guess at data type for this block
					{
						if (analysisFlags[pc] != ANALYZE_NONE)	// already tagged,
						{								// end loop and tag previous
							pc = stop;				// block as last identified type
							break;
						}

						if (isString(pc, stop))	// check if it might be ascii data
						{								// if so, find end of the string
							code = 't';
							pc = getEndOfString(pc, stop);
						}

						data = pgmmem[pc];

						if (!data && code == 't')	// add terminator if it's text
							pc++;

						if (start < pc - 1)			// dump what we've accumulated
						{
							for (adrs=start; adrs<pc; adrs++)
							{							// flag all data in current block
								switch (code)
								{
									case 'b':
										analysisFlags[adrs] = ANALYZE_BINARY | ANALYZE_TRACED;
										break;

									case 't':
										analysisFlags[adrs] = ANALYZE_ASCII | ANALYZE_TRACED;
										break;

									case 'a':
										analysisFlags[adrs] = ANALYZE_VECTOR | ANALYZE_TRACED;
										break;
								}
							}
						}

						while (data == pgmmem[pc] && (pc < stop))
							pc++;				// skip over redundant data

						start = pc;
						data = pgmmem[pc];
						code = 'b';			// assume next data is binary

						if (isprint(data) && data != '"')
						{						// check for ascii text following
							for (j=1; j<MIN_STR_LEN; j++)
							{
								data = pgmmem[pc + j];

								if (!isprint(data) || data == '"')
									break;
							}

							if (j >= MIN_STR_LEN)	// if at min ascii chars in
								code = 't';				// a row, tag the block as text
										
							data = pgmmem[pc];
						}

						if (analysisFlags[start + 1] == ANALYZE_NONE)
						{							// check for possible vector in data
							adrs = (int) pgmmem[start] << 8;
							adrs |= ((int) pgmmem[start + 1] & 0xff);
							adrs &= WORD_MASK;

							if (adrs < himark)
							{						// if it might be a vector...
								for (j=0; j<vstackPtr; j++)
								{						// check against saved dptr values
									if (vstack[j] == begin)
									{					// seems to be a valid vector
										analysisFlags[start] = ANALYZE_VECTOR | ANALYZE_TRACED;
										analysisFlags[start + 1] = ANALYZE_VECTOR | ANALYZE_TRACED;

//printf("tracing vector2 at 0x%04x\n", adrs);

										if (trace(adrs))		// trace the code
											return TRUE;

										code = 0;
										pc = start + 1;
										data = pgmmem[pc];
										start = pc + 1;
									}
								}
							}
							else if (start == begin)	// apparently not a vector
							{
								for (j=0; j<vstackPtr; j++)
								{										// see if it might be a table pointer
									if (vstack[j] == start)		// references some kind of table...
									{
										pc = start;
										lastdata = ANALYZE_NONE;

										while (pc <= stop && lastdata == ANALYZE_NONE)
										{								// check the data in the table
											adrs = (int) pgmmem[pc] << 8;	// might be a vector in
											adrs |= ((int) pgmmem[pc + 1] & 0xff);	// the table
											adrs &= WORD_MASK;

											if (adrs < himark)		// looks like a vector
											{

												analysisFlags[pc] = ANALYZE_VECTOR | ANALYZE_TRACED;
												pc++;
												lastdata |= analysisFlags[pc];
												analysisFlags[pc] = ANALYZE_VECTOR | ANALYZE_TRACED;

												if (analysisFlags[adrs] == ANALYZE_NONE)
												{

//printf("tracing vector3 at 0x%04x\n", adrs);

													if (trace(adrs))
														return TRUE;
												}
											}
											else							// not a vector, must be binary
											{
												analysisFlags[pc] = ANALYZE_BINARY | ANALYZE_TRACED;
											}

											pc++;
											lastdata |= analysisFlags[pc];
											lastdata |= analysisFlags[pc + 1];
										}

										if (analysisFlags[pc] == ANALYZE_NONE)
											analysisFlags[pc] = ANALYZE_BINARY | ANALYZE_TRACED;
									}
								}
							}
						}

						if (isprint(data) && data != '"')	// see if it might be a text string
						{
							if (code != 't')
							{
								for (j=1; j<MIN_STR_LEN; j++)
								{
									data = pgmmem[pc + j];

									if (!isprint(data) || data == '"')
										break;
								}

								if (j >= MIN_STR_LEN)		// yup, looks like a text string
								{
									code = 't';

									for (adrs=start+1; adrs<pc; adrs++)		// tag previous as binary
									{
										if (analysisFlags[adrs] == ANALYZE_NONE)
											analysisFlags[adrs] = ANALYZE_BINARY | ANALYZE_TRACED;
									}

									start = pc;
								}
							}
						}
						else		// not text, must be binary data
						{
							if (code != 'b')
							{
								code = 'b';

								if (start >= pc)
									analysisFlags[start] = ANALYZE_BINARY | ANALYZE_TRACED;	// start might be >= pc
								else
								{
									for (adrs=start+1; adrs<pc; adrs++)		// tag previous as ascii
									{
										if (analysisFlags[adrs] == ANALYZE_NONE)
											analysisFlags[adrs] = ANALYZE_ASCII | ANALYZE_TRACED;
									}
								}

								start = pc;
							}
						}

						pc++;
					}				// while (pc <= stop)

					i = pc;
					code = 0;
					break;

				case (ANALYZE_CODE + ANALYZE_TRACED):
					code = 'c';
					break;

				case (ANALYZE_VECTOR + ANALYZE_TRACED):
					code = 'a';
					break;
			}

			lastflag = aflag;
			start = i;
		}
	}				// for (i=0; i<himark; i++)

	return FALSE;
}

// Check anything not yet identified.

bool aPass2(void)
{
	int	i, j;
	bool	tagged;
	int	start, begin, adrs;
	byte	data, lastdata;

	start = begin = 0;
	j = TRUE;
	analysisFlags[himark - 1] = ANALYZE_END;

	for (i=0; i<himark; i++)
	{
		if (analysisFlags[i] == ANALYZE_NONE)	// not yet identified data
		{
			if (j)				// if previous block identified...
			{
				start = i;		// save start adrs of this block
				j = FALSE;		// flag not identified
			}
		}
		else						// data identified as something...
		{
			if (!j)				// if previous block not identified...
			{
				begin = start;	// then set beginning adrs
				lastdata = 0;

				while (begin < i)
				{
					data = pgmmem[begin];

					if ((data == 0 || data == 0xff) && (lastdata == 0 || lastdata == 0xff))
					{								// two 00 or ff in a row
						start = begin;
						lastdata = data;		// skip over invalid data

						while ((data == 0 || data == 0xff) && start < i)
						{
							start++;
							data = pgmmem[start];
						}

						if (begin < start - 1)
							begin = start;
					}
					else							// data not 00 or ff
					{
						lastdata = 1;			// find end of non 00 or ff data

						while ((data != 0 || data != 0xff || lastdata != 0 || lastdata != 0xff) && begin < i)
						{
							lastdata = data;
							data = pgmmem[begin];
							begin++;

							if (analysisFlags[begin] != ANALYZE_NONE)
								break;

							if ((lastdata == 0 && data == 0) || (lastdata == 0xff && data == 0xff))
								break;
						}

						while (start < begin)
						{
							tagged = FALSE;

							if (analysisFlags[start] == ANALYZE_NONE && analysisFlags[start + 1] == ANALYZE_NONE)
							{
								adrs = (int) pgmmem[start] << 8;	// might be a vector
								adrs |= ((int) pgmmem[start + 1] & 0xff);
								adrs &= WORD_MASK;

								if (adrs && adrs < himark)		// looks like a vector
								{
									if (analysisFlags[adrs] == ANALYZE_NONE || analysisFlags[adrs] == ANALYZE_TAGGED)
									{
										analysisFlags[start] = ANALYZE_VECTOR | ANALYZE_TRACED;
										analysisFlags[start + 1] = ANALYZE_VECTOR | ANALYZE_TRACED;

										if (analysisFlags[adrs] == ANALYZE_NONE)

//printf("tracing vector4 at 0x%04x\n", adrs);

											trace(adrs);

										start++;
										tagged = TRUE;
									}
								}

								if (!tagged)
									analysisFlags[start] = ANALYZE_BINARY | ANALYZE_TRACED;
							}
							else
								analysisFlags[start] = ANALYZE_BINARY | ANALYZE_TRACED;

							start++;
						}

						start = begin;
					}

					begin++;
				}

				start = i;
				j = TRUE;
			}
		}
	}

	return FALSE;
}

// Check if data at 'pc' appears to be an ascii string.
// Return TRUE if so, else return FALSE.

bool isString(int pc, int stop)
{
	int	i;
	bool	retval = FALSE;
	byte	data;

	data = pgmmem[pc];

	if (isprint(data) && data != '"')
	{
		for (i=0; i<MIN_STR_LEN && pc<stop; i++)
		{
			data = pgmmem[pc++];

			if (!isprint(data) || data == '"')
				break;
		}

		if (i >= MIN_STR_LEN)	// if at least min ascii chars in
			retval = TRUE;			// a row, then might be ascii text
	}

	return retval;
}

// Return address of end of string at 'pc'.
// Assumes that pc does point to a string.

int getEndOfString(int pc, int stop)
{
	byte	data;

	data = pgmmem[pc];

	while (isprint(data) && data != '"' && pc < stop)
	{
		pc++;
		data = pgmmem[pc];
	}

	return pc;
}

// Check if code at 'pc' appears to be traceable.
// Return TRUE if so, else return FALSE.
// We assume it's not valid code if it's a series
// of TRACE_CHECK_LEN bytes of 0x00 or 0xff in a row.

bool isTraceableCode(int pc)
{
	int	i;
	bool	retval = FALSE;
	byte	code;

	for (i=0; i<TRACE_CHECK_LEN; i++)
	{
		code = pgmmem[pc++];

		if (code != 0x00 && code != 0xff)
		{
			retval = TRUE;
			break;
		}
	}

	return retval;
}

// Trace a single thread of code starting at address 'pc'.
// Return TRUE if error, else return FALSE.

bool trace(int pc)
{
	bool	done;
	byte	code, flag;
	int	adrs, dest, dptr;
	int	pushLevelSave = 0;

//printf("Start tracing at 0x%04x\n", pc);

	if (!isTraceableCode(pc))		// does not appear to be executable code,
		return FALSE;					// but this is not an error

	tpc = pc;
	astackPtr = 0;
	pushLevel = 0;
	dptr = 0;
	done = FALSE;

	while (!done)
	{
		flag = analysisFlags[tpc];

		while (flag & ANALYZE_TAGGED)		// if we've already been here,
		{
			if (astackPtr)						// check for addresses on stack
			{
				--astackPtr;
				tpc = astack[astackPtr];

//printf("trace: pull 0x%04x off astack\n", tpc);

				flag = analysisFlags[tpc];
			}
			else									// if stack is empty, we're done
			{
				done = TRUE;
				break;
			}
		}

		if (done)
			return FALSE;

		code = pgmmem[tpc];
		analysisFlags[tpc] = ANALYZE_TAGGED;

		switch (code)
		{
/*
			case OPCODE_LJMP:
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				adrs = pgmmem[tpc++] << 8;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				adrs |= (pgmmem[tpc] & 0xff);
				tpc = adrs;
				break;

			case OPCODE_JBC:
			case OPCODE_JB:
			case OPCODE_JNB:
				adrs = tpc + 3;
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				dest = (int) pgmmem[tpc++] & 0xff;

				if (dest & 0x80)
					dest |= 0xff00;

				adrs += dest;
				astack[astackPtr++] = adrs;

				if (astackPtr >= STACK_DEPTH)
				{
					analysisError("trace stack overflow!");
					return TRUE;
				}
				break;
*/
			case OPCODE_CALL:
			case OPCODE_CALLC:
			case OPCODE_CALLNC:
			case OPCODE_CALLZ:
			case OPCODE_CALLNZ:
			case OPCODE_CALLPE:
			case OPCODE_CALLPO:
			case OPCODE_CALLM:
			case OPCODE_CALLP:

//printf("call at 0x%04x\n", tpc);

				pushLevelSave = pushLevel;
				pushLevel = 0;
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				adrs = pgmmem[tpc++] & 0xff;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				adrs |= ((pgmmem[tpc++] & 0xff) << 8);
				adrs &= WORD_MASK;
				astack[astackPtr++] = tpc;

if (astackPtr > astackMax)
	astackMax = astackPtr;

				if (astackPtr >= STACK_DEPTH)
				{
					analysisError("trace stack overflow!");
					return TRUE;
				}

				tpc = adrs;
				break;

			case OPCODE_RET:

//printf("return at 0x%04x\n", tpc);

				if (pushLevel > 0)
				{
					pushLevel = pushLevelSave;
					vstack[vstackPtr++] = dptr;

if (vstackPtr > vstackMax)
	vstackMax = vstackPtr;

					if (vstackPtr >= STACK_DEPTH)
					{
						analysisError("vector stack overflow");
						return TRUE;
					}
				}

			case OPCODE_RETI:
			case OPCODE_JPHL:		// jp (hl) - we don't know where this will go
			case OPCODE_JPIX:
			case OPCODE_JPIY:

//printf("reti or JP() at 0x%04x\n", tpc);

				if (astackPtr)
				{
					--astackPtr;
					tpc = astack[astackPtr];
				}
				else
					done = TRUE;
				break;

			case OPCODE_JR:
				adrs = tpc + 2;
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				dest = (int) pgmmem[tpc++] & 0xff;

				if (dest & 0x80)
					dest |= 0xff00;

				adrs += dest;
				adrs &= WORD_MASK;
				tpc = adrs;
				break;

			case OPCODE_JRC:
			case OPCODE_JRNC:
			case OPCODE_JRZ:
			case OPCODE_JRNZ:
				adrs = tpc + 2;
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				dest = (int) pgmmem[tpc++] & 0xff;

				if (dest & 0x80)
					dest |= 0xff00;

				adrs += dest;
				adrs &= WORD_MASK;
				astack[astackPtr++] = adrs;

if (astackPtr > astackMax)
	astackMax = astackPtr;

				if (astackPtr >= STACK_DEPTH)
				{
					analysisError("trace stack overflow!");
					return TRUE;
				}
				break;

			case OPCODE_PUSHAF:
			case OPCODE_PUSHBC:
			case OPCODE_PUSHDE:
			case OPCODE_PUSHHL:
			case OPCODE_PUSHIX:
			case OPCODE_PUSHIY:
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				tpc++;
				pushLevel++;

if (pushLevel > pushLevelMax)
	pushLevelMax = pushLevel;

				break;

			case OPCODE_POPAF:
			case OPCODE_POPBC:
			case OPCODE_POPDE:
			case OPCODE_POPHL:
			case OPCODE_POPIX:
			case OPCODE_POPIY:
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				tpc++;

				if (pushLevel > 0)
					--pushLevel;

				break;
/*
			case OPCODE_MOVDPTR:
				tpc++;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				dptr = pgmmem[tpc++] << 8;
				analysisFlags[tpc] = ANALYZE_TAGGED;
				dptr |= (pgmmem[tpc++] & 0xff);
				dstack[dstackPtr++] = dptr;

				if (dstackPtr >= STACK_DEPTH)
				{
					analysisError("dptr stack overflow!");
					return TRUE;
				}
				break;
*/
			default:
/*
				if ((code & OPCODE_AJMP_MASK) == OPCODE_AJMP)
				{
					tpc++;
					analysisFlags[tpc] = ANALYZE_TAGGED;
					adrs = tpc + 1;
					dest = ((int) code & 0xe0) << 3;
					dest += ((int) pgmmem[tpc++] & 0xff);
					adrs = dest;
					tpc = adrs;
				}
				else if ((code & OPCODE_ACALL_MASK) == OPCODE_ACALL)
				{
					pushLevelSave = pushLevel;
					pushLevel = 0;
					tpc++;
					analysisFlags[tpc] = ANALYZE_TAGGED;
					adrs = tpc + 1;
					dest = ((int) code & 0xe0) << 3;
					dest |= ((int) pgmmem[tpc++] & 0xff);
					adrs = dest;
					astack[astackPtr++] = tpc;

					if (astackPtr >= STACK_DEPTH)
					{
						analysisError("trace stack overflow!");
						return TRUE;
					}

					tpc = adrs;
				}
				else if ((code == OPCODE_DJNZ1) || (code >= OPCODE_CJNE1 && code <= OPCODE_CJNE2))
				{
					tpc++;
					analysisFlags[tpc] = ANALYZE_TAGGED;
					tpc++;
					analysisFlags[tpc] = ANALYZE_TAGGED;
					adrs = tpc + 1;
					dest = (int) pgmmem[tpc++] & 0xff;

					if (dest & 0x80)
						dest |= 0xff00;

					adrs += dest;
					astack[astackPtr++] = tpc;

					if (astackPtr >= STACK_DEPTH)
					{
						analysisError("trace stack overflow!");
						return TRUE;
					}

					tpc = adrs;
				}
				else*/ if (code == OPCODE_DJNZ)
				{
					tpc++;
					analysisFlags[tpc] = ANALYZE_TAGGED;
					adrs = tpc + 1;
					dest = (int) pgmmem[tpc++] & 0xff;

					if (dest & 0x80)
						dest |= 0xff00;

					adrs += dest;
					adrs &= WORD_MASK;
					astack[astackPtr++] = tpc;

if (astackPtr > astackMax)
	astackMax = astackPtr;

					if (astackPtr >= STACK_DEPTH)
					{
						analysisError("trace stack overflow!");
						return TRUE;
					}

					tpc = adrs;
				}
				else
				{
					if (opttbl[code] & OPT_2)
					{
						tpc++;
						analysisFlags[tpc] = ANALYZE_TAGGED;
						tpc++;
					}
					else if (opttbl[code] & OPT_3)
					{
						tpc++;
						analysisFlags[tpc] = ANALYZE_TAGGED;
						tpc++;
						analysisFlags[tpc] = ANALYZE_TAGGED;
						tpc++;
					}
					else
						tpc++;
				}
				break;
		}
	}
	return FALSE;
}

// Generate list to be written to control file.
// Scan through the program and add list entries
// for each data type (code, ascii, etc).

void genAnalysisList(void)
{
	int	i;
	int	start, stop;
	byte	aflag, lastflag;
	char	code, ostr[64], datatype[32];

	aflag = lastflag = analysisFlags[0];
	start = offset;

	if (offset > 1)
	{
		sprintf(ostr, "i 0000-%04x\t; Invalid data", offset - 1);
		addListEntry(ostr);
	}

	for (i=offset; i<himark; i++)
	{
		aflag = analysisFlags[i];

		if (aflag != lastflag)
		{
			stop = i - 1;
			datatype[0] = '\0';

			switch (lastflag & ~ANALYZE_TRACED)
			{
				case ANALYZE_NONE:
//					code = 'i';
//					sprintf(datatype, "\t; Invalid data");
					code = 'c';
					sprintf(datatype, "\t; Unknown - assumed code space");
					break;

				case ANALYZE_CODE:
					code = 'c';
					sprintf(datatype, "\t; Code space");
					break;

				case ANALYZE_VECTOR:
					code = 'a';
					sprintf(datatype, "\t; pointers");
					break;

				case ANALYZE_BINARY:
					code = 'b';
					sprintf(datatype, "\t; 8-bit data");
					break;

				case ANALYZE_ASCII:
					code = 't';
					sprintf(datatype, "\t; ASCII text");
					break;

				default:
					code = '?';
					break;
			}

			sprintf(ostr, "%c %04x-%04x", code, start, stop);
			strcat(ostr, datatype);
			addListEntry(ostr);

			start = i;
			lastflag = aflag;
		}
	}
}

int createLineList(void)
{
	int		year;
	time_t	tp;

	if (ctlLineList)
		deleteLineList();

	ctlLineList = malloc(sizeof(STRLIST));

	if (!ctlLineList)
	{
		printf("No memory for line list.\n");

		if (ctlfp)
		{
			fclose(ctlfp);
			ctlfp = NULL;
		}
		return -1;
	}

	ctlLineList->str = NULL;
	ctlLineList->prev = NULL;
	ctlLineList->next = NULL;
	addListEntry(";");

	strcpy(tempString, "; DZ80 configuration file for ");
	strcat(tempString, src);
	addListEntry(tempString);

	sprintf(tempString, "; Generated by DZ80 V%d.%d.%d on ", VERSION, MAJORREV, MINORREV);

	time(&tp);									// get current time
	date_time = localtime(&tp);			// convert to hr/min/day etc
	year = date_time->tm_year;

	while (year > 100)
		year -= 100;

	sprintf(dateString, "%02d/%02d/%02d %d:%02d", date_time->tm_mon + 1,
		date_time->tm_mday, year,
		date_time->tm_hour, date_time->tm_min);

	strcat(tempString, dateString);
	addListEntry(tempString);
	addListEntry(";");
	return listCount;
}

void deleteLineList(void)
{
	STRLIST	*list, *prev;

	list = ctlLineList;

	while (list)
	{
		prev = list;
		list = list->next;
		free(prev->str);
		free(prev);
	}

	ctlLineList = NULL;
	listCount = 0;
}

int writeCtlFile(void)
{
	STRLIST	*list;
	char		*str = NULL;

	list = ctlLineList;

	if (!list)
	{
		if (ctlfp)
		{
			fclose(ctlfp);
			ctlfp = NULL;
		}
		return -1;
	}

	if (ctlfp)
	{
		fclose(ctlfp);
		strcpy(fileName, ctl);
		strcat(fileName, ".backup");
		rename(ctl, fileName);
	}

	ctlfp = fopen(ctl, "w");		// reopen ctl file for writing

	if (!ctlfp)
		return -1;

	while (list)
	{
		str = list->str;

		if (str && *str)
			fprintf(ctlfp, "%s\n", str);

		list = list->next;
	}

	fflush(ctlfp);
	fclose(ctlfp);
	ctlfp = NULL;
	return 0;
}

bool addListEntry(char *str)
{
	STRLIST	*list = ctlLineList;

	if (!list)
		return FALSE;

	while (list->next)
		list = list->next;

	list->next = malloc(sizeof(STRLIST));

	if (!list->next)
		return FALSE;

	list->next->prev = list;
	list = list->next;
	list->next = NULL;
	list->str = malloc(strlen(str) + 2);

	if (!list->str)
		return FALSE;

	strcpy(list->str, str);
	listCount++;

	return TRUE;
}

void analysisError(char *msg)
{
	sprintf(alertMessage, "\nAnalysis Error - ");
	strcat(alertMessage, msg);
	printf("%s\n", alertMessage);
}

// end of analyzez80.c

