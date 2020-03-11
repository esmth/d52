
/*
 * Disassembler common routines
 * Copyright (C) 1995-2005 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * common.c - Support routines
 *
 * Version 3.3.6 - 01/18/05
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

#include	"d52.h"
#include	"common.h"

// Global variables

char	src[FN_LEN], dst[FN_LEN];	// file name buffers
char	baseFileName[FN_LEN];		// source file name without extension
char	ctl[FN_LEN];					// control file name
char	linebuffer[MAX_LINE];		// input line buffer
FILE	*fp;								// dos file struct
int	hexflag;							// append hex flag
int	fileflag;						// file type flag
int	upperflag;						// upper case output flag
int	kcnt;								// output char counter
int	pc;								// current program counter
int	himark;							// highest data adrs
int	offset;							// program counter offset
byte	*pgmmem;							// program data pointer
int	*pgmflags;						// pointer to program flags

#ifdef	EXTENDED_MEM
byte	*epgmmem[EXT_PGM_SEGS];		// extended program memory pointers
int	*epgmflags[EXT_PGM_SEGS];	// extended program flags pointers
#endif

char	string[ASCLIMIT];				// ascii data for defb
int	asc_cnt;							// count for string data
byte	byte_data[BYTELIMIT];		// binary data for defb
int	byte_cnt;						// count for binary data
int	word_data[WORDLIMIT];		// binary data for defw
int	word_cnt;						// count for word data
byte	dump;								// dump just done flag
byte	ascii_flag;						// use ascii string flag
char	defbstr[8];						// string for defined bytes
char	defwstr[8];						// string for defined words
char	ascistr[8];						// string for defined ascii
char	orgstr[] = "org";				// org pseudo-op string
char	equstr[] = "equ";				// equ pseudo-op string

struct sym	*sym_tab;					// symbol table pointer
struct sym	*lab_tab;					// label table pointer
struct sym	*name_tab;					// operand names pointer
struct sym	*sym_tab_last;				// last symbol table pointer
struct sym	*lab_tab_last;				// lastlabel table pointer
struct sym	*name_tab_last;			// last name table pointer
int			symbol_count;				// number of symbols
int			label_count;				// number of labels
int			name_count;					// number of operand names

SYM_PTR		*sym_val_index;			// array of pointers
SYM_PTR		*lab_val_index;			//  for binary search
SYM_PTR		*name_val_index;
struct sym	*tail_ptr, *head_ptr;	// sort pointers

struct comment	*comment_list;			// header comment list
struct comment	*icomment_list;		// inline comment list
struct comment	*patch_list;			// patch list

int			newline;						// just output newline flag
struct tm	*date_time;					// disassembly time

//
// Code
//

bool init_memory(void)
{
	int	count;

#ifdef EXTENDED_MEM
	for (count=0; count <EXT_PGM_SEGS; count++)
	{
		epgmmem[count] = NULL;
		epgmflags[count] = NULL;
	}
#endif

	if ((pgmmem = (byte *) malloc(PMEMSIZE)) == NULL)
	{
		printf("INTERNAL ERROR! - Can't allocate program space!\n");
		return FALSE;
	}

	if ((pgmflags = (int *) malloc(PMEMSIZE * sizeof(int))) == NULL)
	{
		printf("INTERNAL ERROR! - Can't allocate flag space!\n");
		return FALSE;
	}

	printf("Initializing program spaces...");

	for (count=WORD_MASK; count; count--)	// fill code space with
	{
		pgmmem[count] = NO_DATA;				// invalidate data
		pgmflags[count] = PF_INIT;				// invalidate flags
	}

	pgmmem[0] = NO_DATA;							// include first location
	pgmflags[0] = PF_INIT;
	return TRUE;
}

#ifdef	EXTENDED_MEM

// Get extended program or flag memory.
// Width = 1 for program memory or 2 for flag memory.

byte *get_extended_mem(int width)
{
	byte	*ptr;

	ptr = (byte *) malloc(PMEMSIZE * width);

	if (!ptr)
		printf("\nCan't allocate extended memory!\n");

	return ptr;
}
#endif

char * makeupper(char *str)
{
	char	*ret = str;

	while (*str)
	{
		*str = toupper(*str);
		str++;
	}

	return ret;
}

// Parse file name. Put file name without extension in
// baseFileName, and return:
//		EITHERFILE		if no extension provided
//		BINFILE			if extension is '.bin'
//		HEXFILE			if extension is '.hex'
//		CPMFILE			if extension is '.com'

int parseFileName(char *str, char *ext)
{
	int	i, type;
	char	*cptr;

	type = EITHERFILE;
	strcpy(baseFileName, str);
	strcpy(src, str);
	cptr = strrchr(str, '.');

	if (!cptr)						// if '.' not found, then use eitherfile
	{
		strcat(src, ".hex");
	}
	else
	{
		i = (int) (cptr - str);	// get offset of '.' character

		if (!strncasecmp(cptr, ".hex", 4))
		{
			baseFileName[i] = '\0';
			type =  HEXFILE;
		}
		else if (!strncasecmp(cptr, ".bin", 4))
		{
			baseFileName[i] = '\0';
			type = BINFILE;
		}
		else if (!strncasecmp(cptr, ".com", 4) && !strcmp(ext, ".z80"))
		{
			baseFileName[i] = '\0';
			offset = 0x100;
			type = CPMFILE;
		}
		else
			strcat(src, ".hex");
	}

	strcpy(dst, baseFileName);
	strcat(dst, ext);
	strcpy(ctl, baseFileName);
	strcat(ctl, ".ctl");

	return type;
}

//
// Read bin, hex, or com file into program memory.
//

int readfile(char *filename)
{
	int	i, j, rectype, page, line, readsize;

// open source file

	switch (fileflag)
	{
		case EITHERFILE:				// if no type specified...
			fp = fopen(src, "r");	// search for hex file first

			if (fp == NULL)			// if not found, search for bin file
			{
				fileflag = BINFILE;
				strcpy(src, baseFileName);
				strcat(src, ".bin");
				fp = fopen(src, "rb");

				if (fp == NULL)
				{
					printf("\n* Can't open either '%s.hex' nor '%s.bin' *\n\n",
							baseFileName, baseFileName);
					exit(FILE_ERROR);
				}
				else
					fileflag = BINFILE;
			}
			break;

		case HEXFILE:					// force hex file
			fp = fopen(src, "r");
			break;

		case BINFILE:					// force bin file
		case CPMFILE:					// CP/M .com file
			fp = fopen(src, "rb");
			break;
	}

	if (fp == NULL)					// if file not found...
	{
		printf("\n* Can't open file '%s' *\n\n", src);
		exit(FILE_ERROR);
	}

// read input file and set up data array

	himark = 0;
	line = 0;
	pc = offset;
	printf("\nreading %s\n", src);

	if (fileflag == BINFILE || fileflag == CPMFILE)	// if binary file...
	{
		while (!feof(fp))								// until end of file...
		{
			if (pc >= PMEMSIZE)						// exceeded 64K limit
			{
				printf("\nInput file too large!\n\n");
				break;
			}

			if ((pc + MAX_LINE) >= PMEMSIZE)		// if next read would exceed limit
				readsize = PMEMSIZE - pc;			// reduce read byte count
			else
				readsize = MAX_LINE;

			i = fread(linebuffer, 1, readsize, fp);	// read a block of data

			for (j=0; j<i; j++)
			{
				pgmmem[pc] = linebuffer[j];		// copy to program space
				pgmflags[pc] = PF_DATA;
				pc++;

				if ((pc & 0xff) == 0)
					printf("\r%04x", pc);			// show progress
			}

			if (pc & WORD_MASK)
				himark = pc;
			else
				himark = WORD_MASK;					// flag highest location
		}
	}

	else													// else hex file...
	{
		page = 0;
		rectype = 0;

		while (!feof(fp))								// until end of file...
		{
			*linebuffer = '\0';						// clear previous line
			fgets(linebuffer, MAX_LINE - 1, fp);// read one line
			line++;

			if (sscanf(linebuffer, "%*c%2x%4x%2x", &i, &pc, &rectype) != EOF)
			{												// get count and address
				pc += offset;							// add offset to address
				pc += page;								// add segment to address

				if ((pc + i) >= PMEMSIZE)
				{
					printf("\nInput file too large!\n\n");
//					exit(FILE_ERROR);
					break;
				}

				if (rectype == 1)
					break;								// done if end of hex record

				if (rectype == 2)						// extended segment address record
				{
					sscanf((char *) &linebuffer[9], "%4x", &page);
					page <<= 4;
				}
				else if (rectype == 0)				// data record
				{
					if (i > 64)							// oops! line too long
					{
						printf("invalid count (%d) in line %d:\n", i, line);
						printf("%s", linebuffer);
						exit(FILE_ERROR);
					}

					for (j=0; j<i ; j++)				// now move data to program area
					{
						getcode(linebuffer + 9 + j * 2, &pgmmem[pc]);	// code to program space
						pgmflags[pc] = PF_DATA;		// flag valid data
						pc++;

						if ((pc & 0xff) == 0)		// show progress
							printf("\r%04x", pc);
					}

					if (pc > himark)
						himark = pc;
				}
			}
		}
	}

	fclose(fp);										// done reading input file
	printf("\rHighest location = ");			// show last location

	if (himark == WORD_MASK)
		printf("%04x\n", himark);
	else
		printf("%04x\n", himark - 1);

	return 0;
}

//
//	Put ascii hex data into binary array
//

void getcode(char *from, byte *loc)
{
	byte	c, i;

	c = *from++ - 0x30;
	c = (c > 10) ? c - 7 : c;
	i = c << 4;
	c = *from++ - 0x30;
	c = (c > 10) ? c - 7 : c;
	*loc = i | c;
}

//
// Get hexadecimal number from line in control file.
// Return updated character pointer.
//

char *get_adrs(char *text, int *val)
{
	int	result, start;
	char	c;

	result = start = 0;
	c = toupper(*text);

	while (c)
	{
		if (c == ';')			// beginning of comment, ignore all else
			break;

		if (c == '\n')			// necessary because isspace() includes \n
			break;

		if (isspace(c))		// skip leading whitespace
		{
			text++;
			if (start)			// if result already begun...
				break;
		}
		else if (!isxdigit(c))		// done if not hexadecimal character
			break;
		else
		{
			start = 1;			// flag beginning of result conversion
			c = (c > '9') ? c - 0x37 : c - 0x30;
			result <<= 4;
			result |= ((int) c & 0xf);
			text++;
		}

		c = toupper(*text);
	}

	*val = result;				// pass number back to caller
	return(text);				// and return updated text pointer
}

void error(char *str1, char *str2)			// fatal error trap
{
	printf("\n%s%s", str1, str2);
	exit(FILE_ERROR);
}

//			Sort label or symbol table
//	First sort by name so that we can check for duplicates,
//	then sort by value, check for duplicates, and set up
//	pointer array for binary search.
//

struct sym *sort(struct sym *list, SYM_PTR *array, int count)
{
	int	i;
	struct sym	*sptr, *temp;

	sptr = sort_by_name(list);

	if (list == name_tab)
		chk_dup_op_name(sptr, count);
	else
		chk_dup_name(sptr, count);

	sptr = sort_by_value(sptr);
	chk_dup_value(sptr, count);
	temp = sptr;

	for (i=0; i<count; i++)		// set up array of pointers sorted by value
	{
		array[i] = temp;
		temp = temp->next;
	}

	return(sptr);
}

//
// In-place non-recursive merge sort using label text as key
//

struct sym *sort_by_name(struct sym *list)
{
	int			i, n;
	struct sym	*a, *b, *todo, *t;

	head_ptr = (struct sym *) malloc(sizeof(struct sym));
	head_ptr->next = list;
	a = tail_ptr;

	for (n=1; a != head_ptr->next; n = n + n)
	{
		todo = head_ptr->next;
		list = head_ptr;

		while (todo != tail_ptr)
		{
			t = todo;
			a = t;

			for (i=1; i<n; i++)
				t = t->next;

			b = t->next;
			t->next = tail_ptr;
			t = b;

			for (i=1; i<n; i++)
				t = t->next;

			todo = t->next;
			t->next = tail_ptr;
			list->next = merge_by_name(a, b);

			for (i=1; i<=n+n; i++)
				list = list->next;
		}
	}

	return(head_ptr->next);
}

//
// In-place non-recursive merge sort using value as key
//

struct sym *sort_by_value(struct sym *list)
{
	int			i, n;
	struct sym	*a, *b, *todo, *t;

	head_ptr = (struct sym *) malloc(sizeof(struct sym));
	head_ptr->next = list;
	a = tail_ptr;

	for (n=1; a != head_ptr->next; n = n + n)
	{
		todo = head_ptr->next;
		list = head_ptr;

		while (todo != tail_ptr)
		{
			t = todo;
			a = t;

			for (i=1; i<n; i++)
				t = t->next;

			b = t->next;
			t->next = tail_ptr;
			t = b;

			for (i=1; i<n; i++)
				t = t->next;

			todo = t->next;
			t->next = tail_ptr;
			list->next = merge_by_value(a, b);

			for (i=1; i<=n+n; i++)
				list = list->next;
		}
	}

	return(head_ptr->next);
}

//
// Merge sub-lists by text field
//

struct sym *merge_by_name(struct sym *a, struct sym *b)
{
	int			i;
	struct sym	*c;

	c = tail_ptr;

	do
	{
		i = strcasecmp(a->name, b->name);

		if (i <= 0)
		{
			c->next = a;
			c = a;
			a = a->next;
		}
		else
		{
			c->next = b;
			c = b;
			b = b->next;
		}
	} while (c != tail_ptr);

	c = tail_ptr->next;
	tail_ptr->next = tail_ptr;
	return(c);
}

//
// Merge sub-lists by value field
//

struct sym *merge_by_value(struct sym *a, struct sym *b)
{
	struct sym	*c;

	c = tail_ptr;

	do
	{
		if (a->val < b->val)
		{
			c->next = a;
			c = a;
			a = a->next;
		}
		else
		{
			c->next = b;
			c = b;
			b = b->next;
		}
	} while (c != tail_ptr);

	c = tail_ptr->next;
	tail_ptr->next = tail_ptr;
	return(c);
}

//
// Check for redefinitions of label/symbol names
//

void chk_dup_name(struct sym *list, int count)
{
	int	i;

	for (i=0; i<count; i++)
	{
		if (!strcasecmp(list->name, list->next->name))
		{
			printf("\nAttempted redefinition of '%s', value 0x%x,"
					 " as value 0x%x\n",
					list->name, list->val, list->next->val);
			exit(USER_ERROR);
		}

		list = list->next;
	}
}

//
// Check for redefinitions of operand names
//

void chk_dup_op_name(struct sym *list, int count)
{
	int	i, adrs, val, next_adrs, next_val;

	for (i=0; i<count; i++)
	{
		if (!strcasecmp(list->name, list->next->name))
		{
			adrs = list->val;
			val = pgmmem[adrs];

			if (pgmflags[adrs] & PF_WORD)
			{
				val <<= 8;
				val |= pgmmem[adrs + 1];
			}

			next_adrs = list->next->val;
			next_val = pgmmem[next_adrs];

			if (pgmflags[next_adrs] & PF_WORD)
			{
				next_val <<= 8;
				next_val |= pgmmem[next_adrs + 1];
			}

			if (val != next_val)
			{
				printf("\nAttempted redefinition of '%s', value 0x%x,"
						 " as value 0x%x\n",
						list->name, val, next_val);
				exit(USER_ERROR);
			}
		}

		list = list->next;
	}
}

//
// Check for redefinitions of values
//

void chk_dup_value(struct sym *list, int count)
{
	int	i;

	for (i=0; i<count; i++)
	{
		if (list->val == list->next->val)
		{
			printf("\nAttempted redefinition of value 0x%x, '%s', as '%s'\n",
					list->val, list->name, list->next->name);
			exit(USER_ERROR);
		}

		list = list->next;
	}
}

//		Find symbol or label by value
//
// Binary search using table of pointers to list nodes.
// This search is based on the fact that:
//			list[first-1] <= key < list[last+1]
// is an invariant. This allows the while loop to involve
// only one boolean expression, rather than two. The 'if'
// statement also involves only one boolean expression.
// The test for equality (has it been found?) is not done
// until the loop is complete. This significantly speeds
// up the search compared to a standard binary search.
//

char *find_entry(int val, int count, SYM_PTR *table)
{
	struct sym	*ptr;
	int			first, mid, last;
	char			*ret;

	first = 1;
	last = count;

	while (first <= last)				// while more to search...
	{
		mid = (first + last) >> 1;		// begin in middle of remaining array
		ptr = table[mid - 1];			// get pointer to node

		if (ptr->val > val)				// determine which way to go
			last = mid - 1;				// search before midpoint
		else
			first = mid + 1;				// search after midpoint
	}

	ret = NULL;								// assume not found

	if (last > 0)							// are we still within the array?
	{
		ptr = table[last - 1];			// if so, get last pointer

		if (val == ptr->val)				// is it what we're searching for?
		{
			ptr->used = TRUE;				// label/symbol has been used
			ret = ptr->name;				// return pointer to caller
		}
	}

	return(ret);
}

//
// Find symbol or label by name
// Table must be sorted by name
//

SYM_PTR find_name(char *name, int count, SYM_PTR *table)
{
	int		i;
	SYM_PTR	ptr;
	int		first, mid, last;

	first = 1;
	last = count;

	while (first <= last)				// while more to search...
	{
		mid = (first + last) >> 1;		// begin in middle of remaining array
		ptr = table[mid - 1];			// get pointer to node

		i = strcasecmp(ptr->name, name);

		if (i > 0)
			last = mid - 1;				// search before midpoint
		else
			first = mid + 1;				// search after midpoint
	}

	ptr = NULL;								// assume not found

	if (last > 0)							// are we still within the array?
	{
		ptr = table[last - 1];			// if so, get last pointer

		if (strcasecmp(name, ptr->name))
			ptr = NULL;
	}

	return(ptr);
}

//
// Allocate new entry in symbol table or label table
//

struct sym *get_smem(int type, int req_size)
{
	struct sym	*ptr;

	ptr = (struct sym*) malloc(sizeof(struct sym) + req_size + 1);
																// get memory from OS
	if (ptr == NULL)										// what? out of memory?...
	{
		printf("\nINTERNAL ERROR! - No memory for ");

		switch (type)
		{
			case LABEL_TYPE:
				printf("label");
				break;

			case SYMBOL_TYPE:
				printf("symbol");
				break;

			case NAME_TYPE:
				printf("name");
				break;

			default:
				printf("\nUnknown table type: %d\n", type);
				exit(PROGRAM_ERROR);
				break;
		}

		printf(" table!\n");
		exit(MEM_ERROR);
	}

	ptr->next = NULL;
	return(ptr);						// give caller the address
}

//
// Add symbol or label to table
//

void add_entry(int val, char *symbol, int type)
{
	struct sym	*nptr, *tbl_ptr;
	char			*cptr;
	char			tbl_name[8];

	cptr = symbol;				// ensure that input string is null terminated

	while (*cptr)
	{
		if (!isgraph(*cptr))
			break;
		cptr++;
	}

	*cptr = '\0';

	if (upperflag)
		makeupper(symbol);

	switch (type)
	{
		case LABEL_TYPE:
			tbl_ptr = lab_tab;				// get pointer to correct table
			strcpy(tbl_name, "label");
			label_count++;
			break;

		case SYMBOL_TYPE:
			tbl_ptr = sym_tab;
			strcpy(tbl_name, "symbol");
			symbol_count++;
			break;

		case NAME_TYPE:
			tbl_ptr = name_tab;
			strcpy(tbl_name, "name");
			name_count++;
			break;

		default:
			printf("\nUnknown table type: %d\n", type);
			exit(PROGRAM_ERROR);
			break;
	}

	nptr = get_smem(type, strlen(symbol));

	if (tbl_ptr == NULL)					// if first symbol or label...
	{
		switch (type)
		{
			case LABEL_TYPE:
				lab_tab = nptr;
				break;

			case SYMBOL_TYPE:
				sym_tab = nptr;
				break;

			case NAME_TYPE:
				name_tab = nptr;
				break;

			default:
				printf("\nUnknown table type: %d\n", type);
				exit(PROGRAM_ERROR);
				break;
		}
	}
	else
	{
		switch (type)
		{
			case LABEL_TYPE:
				lab_tab_last->next = nptr;
				break;

			case SYMBOL_TYPE:
				sym_tab_last->next = nptr;
				break;

			case NAME_TYPE:
				name_tab_last->next = nptr;
				break;

			default:
				printf("\nUnknown table type: %d\n", type);
				exit(PROGRAM_ERROR);
				break;
		}
	}

	switch (type)
	{
		case LABEL_TYPE:
			lab_tab_last = nptr;
			break;

		case SYMBOL_TYPE:
			sym_tab_last = nptr;
			break;

		case NAME_TYPE:
			name_tab_last = nptr;
			break;

		default:
			printf("\nUnknown table type: %d\n", type);
			exit(PROGRAM_ERROR);
			break;
	}

	nptr->used = FALSE;
	nptr->val = val;						// set value in new entry
	nptr->name = malloc(strlen(symbol) + 1);

	if (!nptr->name)
	{
		printf("\nCan't allocate memory for ");

		switch (type)
		{
			case LABEL_TYPE:
				printf("label: %s\n", symbol);
				break;

			case SYMBOL_TYPE:
				printf("symbol: %s\n", symbol);
				break;

			case NAME_TYPE:
				printf("name: %s\n", symbol);
				break;

			default:
				printf("Unknown table type: %d: %s\n", type, symbol);
				break;
		}

		exit(MEM_ERROR);
	}

	strcpy(nptr->name, symbol);		// and copy text to entry
}

//
// Output header comment(s) associated with 'adrs'
// Expand newlines
//

void output_comment(int adrs)
{
	COMMENT_PTR	cmt;
	char			*str;

	cmt = comment_list;

	while (cmt)										// search through list for all
	{													// entries that match adrs
		if (cmt->adrs == adrs)
		{
			str = cmt->str;
			fprintf(fp, "\n;");

			if (strlen(str) > 3 && str[2] == '\\' && str[3] == 'n')
			{
				fprintf(fp, "\n; ");
				str += 4;
			}
			else
				str++;

			while (*str)
			{
				if (*str == '\\' && *(str + 1) && (*(str + 1) == 'n'))
				{
					str++;
					fprintf(fp, "\n;");

					if (*(str + 1))
						fputc(' ', fp);
				}
				else
					fputc(*str, fp);

				str++;
			}
		}

		cmt = cmt->next;
	}
}

//
// Output inline comment associated with 'adrs'
//

void output_icomment(int adrs)
{
	COMMENT_PTR	cmt;

	cmt = icomment_list;

	while (cmt)										// search through list for all
	{													// entries that match adrs
		if (cmt->adrs == adrs)
			fprintf(fp, "\t%s", cmt->str);

		cmt = cmt->next;
	}
}

//
// Output patch string(s) associated with 'adrs'
// Expand newlines
//

void output_patch(int adrs)
{
	COMMENT_PTR	patch;
	char			*str;

	patch = patch_list;

	while (patch)									// search through list for all
	{													// entries that match adrs
		if (patch->adrs == adrs)
		{
			str = patch->str;
			fprintf(fp, "\n");

			while (*str)
			{
				if (*str == '\\' && *(str + 1) && (*(str + 1) == 'n'))
				{
					str++;
					fprintf(fp, "\n");
				}
				else
					fputc(*str, fp);

				str++;
			}
		}

		patch = patch->next;
	}
}

//
// Add comment string to linked list in memory
//

void add_comment(int adrs, char *str)
{
	int			len;
	char			*ctext;
	COMMENT_PTR	cmt_ptr;

	len = strlen(str) - 1;

	if (len >= 0 && (str[len] == '\n' || str[len] == '\r'))
		str[len] = '\0';										// get rid of newline

	if (!comment_list)										// first comment
	{
		comment_list = malloc(sizeof(struct comment));

		if (comment_list == NULL)
		{
			printf("\nNo memory for comment struct");
			exit(MEM_ERROR);
		}

		cmt_ptr = comment_list;
	}
	else															// add comment to list
	{
		cmt_ptr = comment_list;

		while (cmt_ptr->next)								// find end of list
			cmt_ptr = cmt_ptr->next;

		cmt_ptr->next = malloc(sizeof(struct comment));

		if (cmt_ptr->next == NULL)
		{
			printf("\nNo memory for comment struct");
			exit(MEM_ERROR);
		}

		cmt_ptr = cmt_ptr->next;
	}

	cmt_ptr->adrs = adrs;
	ctext = malloc(strlen(str) + 3);

	if (ctext == NULL)
	{
		printf("\nNo memory for comment string");
		exit(MEM_ERROR);
	}

	cmt_ptr->str = ctext;

	if (len >= 0)
	{
		strcpy(ctext, "; ");
		strcat(ctext, str);
	}
	else
		strcpy(ctext, ";");

	cmt_ptr->next = NULL;
}

void add_icomment(int adrs, char *str)
{
	int			len;
	char			*ctext;
	COMMENT_PTR	cmt_ptr;

	len = strlen(str) - 1;

	if (str[len] == '\n' || str[len] == '\r')
		str[len] = '\0';										// get rid of newline

	if (!icomment_list)										// first comment
	{
		icomment_list = malloc(sizeof(struct comment));

		if (icomment_list == NULL)
		{
			printf("\nNo memory for comment struct");
			exit(MEM_ERROR);
		}

		cmt_ptr = icomment_list;
	}
	else															// add comment to list
	{
		cmt_ptr = icomment_list;

		while (cmt_ptr->next)								// find end of list
			cmt_ptr = cmt_ptr->next;

		cmt_ptr->next = malloc(sizeof(struct comment));

		if (cmt_ptr->next == NULL)
		{
			printf("\nNo memory for comment struct");
			exit(MEM_ERROR);
		}

		cmt_ptr = cmt_ptr->next;
	}

	cmt_ptr->adrs = adrs;
	ctext = malloc(strlen(str) + 3);

	if (ctext == NULL)
	{
		printf("\nNo memory for comment string");
		exit(MEM_ERROR);
	}

	cmt_ptr->str = ctext;
	strcpy(ctext, "; ");
	strcat(ctext, str);
	cmt_ptr->next = NULL;
}

//
// Add patch string to linked list in memory
//

void add_patch(int adrs, char *str)
{
	int			len;
	char			*ctext;
	COMMENT_PTR	patch_ptr;

	len = strlen(str) - 1;

	if (len >= 0 && (str[len] == '\n' || str[len] == '\r'))
		str[len] = '\0';									// get rid of newline

	if (!patch_list)										// first patch
	{
		patch_list = malloc(sizeof(struct comment));

		if (patch_list == NULL)
		{
			printf("\nNo memory for patch struct");
			exit(MEM_ERROR);
		}

		patch_ptr = patch_list;
	}
	else														// add patch to list
	{
		patch_ptr = patch_list;

		while (patch_ptr->next)							// find end of list
			patch_ptr = patch_ptr->next;

		patch_ptr->next = malloc(sizeof(struct comment));

		if (patch_ptr->next == NULL)
		{
			printf("\nNo memory for patch struct");
			exit(MEM_ERROR);
		}

		patch_ptr = patch_ptr->next;
	}

	patch_ptr->adrs = adrs;
	ctext = malloc(strlen(str) + 3);

	if (ctext == NULL)
	{
		printf("\nNo memory for patch string");
		exit(MEM_ERROR);
	}

	patch_ptr->str = ctext;

	if (len >= 0)
		strcpy(ctext, str);
	else
		strcpy(ctext, ";");

	patch_ptr->next = NULL;
}

//
//		Output hexadecimal operand
//

void puthex(int j)
{
	j &= WORD_MASK;

	if (j < 10)
	{
		if (upperflag)
			kcnt += fprintf(fp, "%X", j);
		else
			kcnt += fprintf(fp, "%x", j);
	}
	else if (j < 16)
	{
		if (upperflag)
			kcnt += fprintf(fp, "0%XH", j);
		else
			kcnt += fprintf(fp, "0%xh", j);
	}
	else if (j < 0xa0)
	{
		if (upperflag)
			kcnt += fprintf(fp, "%XH", j);
		else
			kcnt += fprintf(fp, "%xh", j);
	}
	else if (j < 0x100)
	{
		if (upperflag)
			kcnt += fprintf(fp, "0%XH", j);
		else
			kcnt += fprintf(fp, "0%xh", j);
	}
	else if (j < 0xa00)
	{
		if (upperflag)
			kcnt += fprintf(fp, "%XH", j);
		else
			kcnt += fprintf(fp, "%xh", j);
	}
	else if (j < 0x1000)
	{
		if (upperflag)
			kcnt += fprintf(fp, "0%XH", j);
		else
			kcnt += fprintf(fp, "0%xh", j);
	}
	else if (j < 0xa000)
	{
		if (upperflag)
			kcnt += fprintf(fp, "%XH", j);
		else
			kcnt += fprintf(fp, "%xh", j);
	}
	else
	{
		if (upperflag)
			kcnt += fprintf(fp, "0%XH", j);
		else
			kcnt += fprintf(fp, "0%xh", j);
	}
}


//
//		Convert code to printable ascii
//

int ascii(int i)
{
	i = i & 0x7f;

	if (i == 0x7f)
		return ('.');
	else if (i < 0x20)
		return ('.');
	else
		return (i);
}

//
//		Check if data is printable ascii other than the string delimiter
//

int is_ascii(byte data)
{
	if (data < ' ' || data > 0x7e || data == '\'')
		return(0);
	return(1);
}

//
//	Convert ascii hex to hexadecimal for X option
//

int atox(char *str)
{
	char	c;
	int	i;

	i = 0;
	c = (char) toupper(*str++);

	while (c)
	{
		if (isxdigit((int) c))
		{
			c = (c > '9') ? c - 0x37 : c & 0xf;
			i = (i << 4) | c;
		}
		else
			break;

		c = (char) toupper(*str++);
	}
	return(i);
}

//
// Check for reference to address in middle of
// code and flag split reference if true
//

void splitcheck(int i)
{
	if (!(pgmflags[i] & PF_CLREF))		// ignore if not referenced
		pgmflags[i] |= PF_SPLIT;			// else flag split ref
}

//
//	Add label to output line if current location marked as referenced
//

void chk_ref(int i)
{
	int	cnt;
	char	*cptr;

	if ((pgmflags[i] & (PF_REF | PF_CLREF)) == PF_REF)
	{
		pgmflags[i] |= PF_LABGEN;
		cptr = find_entry(i, label_count, lab_val_index);			// see if label exists

		if (cptr == NULL)				// if not, output hex value
		{
			if (upperflag)
				cnt = fprintf(fp, "\nX%04X:", i);
			else
				cnt = fprintf(fp, "\nX%04x:", i);
		}
		else
			cnt = fprintf(fp, "\n%s:", cptr);	// else output label text

		if (cnt > 8)
			fprintf(fp, "\n\t");
		else
			fprintf(fp, "\t");
	}
	else
		fprintf(fp, "\n\t");
}

//
//	Add label to output line if current location marked as referenced
// For PF_WORD and PF_ADRS data
//

void chk_label(int i)
{
	char	*cptr;

	if ((pgmflags[i] & (PF_REF | PF_CLREF)) == PF_REF)
	{
		pgmflags[i] |= PF_LABGEN;
		cptr = find_entry(i, label_count, lab_val_index);			// see if label exists

		if (cptr == NULL)							// if not, output hex value
		{
			if (upperflag)
				fprintf(fp, "\nX%04X:", i);
			else
				fprintf(fp, "\nX%04x:", i);
		}
		else
			fprintf(fp, "\n%s:", cptr);		// else output label text

		fprintf(fp, "\t");
	}
	else
		fprintf(fp, "\n\t");
}

//
// Output opcode for current code
//

void doopcode(char *mnem)
{
	char	c;

	c = *mnem++;

	if (upperflag)
		c = toupper(c);

	while (c)						// output text from opcode table
	{
		if (c == ' ')				// convert spaces to tabs
		{
			putc('\t', fp);
			kcnt = (kcnt + 8) & 0x78;
		}
		else
		{
			putc(c, fp);
			kcnt++;
		}

		c = *mnem++;

		if (upperflag)
			c = toupper(c);
	}
}

//
// Output ascii data accumulated in buffer
//

void dump_ascii(int adrs)
{
	int	padrs, off, cnt;
	char	*cptr;

	padrs = adrs - asc_cnt;			// print address for comment field
	adrs = padrs;						// address in program array
	cnt = off = 0;						// cnt = char count, off = buffer offset

	while (asc_cnt)					// while data in ascii buffer...
	{
		if (pgmflags[adrs] & PF_REF)	// if addresss is referenced...
		{
			if (cnt)
			{
				putc('\'', fp);			// terminate line
				kcnt++;

				if (hexflag)				// if comment field requested...
				{
					do							// show hex address
					{
						putc('\t', fp);	// but tab out to field first
						kcnt = (kcnt + 8) & 0x78;
					} while (kcnt < XSTOP);

					fprintf(fp, "; %04x", padrs);
				}

				padrs += cnt;				// update print address
				cnt = 0;						// clear char count for this line
			}

			pgmflags[adrs] |= PF_LABGEN;
			cptr = find_entry(adrs, label_count, lab_val_index);
												// see if label exists for this adrs
			if (cptr == NULL)				// if not, show address in hex
				fprintf(fp, "\nX%04x:\t%s\t'", adrs, ascistr);
			else								// else show label name
				fprintf(fp, "\n%s:\t%s\t'", cptr, ascistr);

			kcnt = 17;
		}
		else if (!cnt)
		{
			fprintf(fp, "\n\t%s\t'", ascistr);
			kcnt = 17;
		}

		putc(string[off], fp);			// output data in ascii
		kcnt++;								// character position in this line
		cnt++;								// increment char in this line
		off++;								// increment offset into asci buffer
		adrs++;								// offset into program memory

		if (cnt >= ASCLINE)				// if max characters per line...
		{
			putc('\'', fp);				// terminate line
			kcnt++;

			if (hexflag)					// if comment field requested
			{
				do								// show hex address
				{
					putc('\t', fp);
					kcnt = (kcnt + 8) & 0x78;
				} while (kcnt < XSTOP);

				fprintf(fp, "; %04x", padrs);
			}

			padrs += cnt;					// update print address
			cnt = 0;
		}

		--asc_cnt;
	}

	putc('\'', fp);						// terminate line
	kcnt++;

	if (hexflag && cnt)					// if comment field requested...
	{
		do										// show address
		{
			putc('\t', fp);
			kcnt = (kcnt + 8) & 0x78;
		} while (kcnt < XSTOP);

		fprintf(fp, "; %04x", padrs);
	}

	dump = 1;
}

//
// Output binary data accumulated in buffer
//

void dump_bytes(int adrs)
{
	int	padrs, bcnt, off, k;
	char	*cptr, chr;

	padrs = adrs - byte_cnt;			// compute adrs to print in ascii part
	adrs = padrs;
	bcnt = off = 0;						// no bytes output yet

	while (byte_cnt)						// while data in binary buffer...
	{
		if (pgmflags[adrs] & PF_REF)	// if data adrs is referenced, or header comment...
		{
			if (off && hexflag)			// dump any remaining ascii first
			{
				do
				{
					putc('\t', fp);
					kcnt = (kcnt + 8) & 0x78;
				} while (kcnt < XSTOP);

				fprintf(fp, "; %04x ", padrs);

				for (k=0; k<off; k++)
					putc(ascii(pgmmem[padrs + k]), fp);

				padrs += k;					// update print address
				off = 0;
			}

			if (pgmflags[adrs] & PF_REF)
			{
				pgmflags[adrs] |= PF_LABGEN;
				cptr = find_entry(adrs, label_count, lab_val_index);	// then do a label

				if (cptr == NULL)
				{
					if (upperflag)
						fprintf(fp, "\nX%04X:\t%s\t", adrs, defbstr);
					else
						fprintf(fp, "\nX%04x:\t%s\t", adrs, defbstr);
				}
				else
					fprintf(fp, "\n%s:\t%s\t", cptr, defbstr);
			}

			kcnt = 16;
			bcnt = 0;
		}
		else if (!bcnt)						// else if first byte...
		{
			kcnt = 16;
			fprintf(fp, "\n\t%s\t", defbstr);
		}
		else
		{
			putc(',', fp);						// else separate bytes
			kcnt++;
		}

		if ((pgmflags[adrs] & (PF_LABEL | PF_SYMBOL)) == (PF_LABEL | PF_SYMBOL))
			cptr = NULL;
		else
			cptr = find_entry(pgmmem[adrs], symbol_count, sym_val_index);

		if (cptr)
			kcnt += fprintf(fp, "%s", cptr);
		else
		{
			if (!(pgmflags[adrs] & PF_ASCII))
				puthex(pgmmem[adrs] & 0xff);
			else									// user defined this as ascii text
			{										// even though it's not; let's try
				chr = pgmmem[adrs];			// to give him what he wants.

				if (chr & 0x80)				// if flagged binary byte because
				{									// high bit is set...
					chr &= 0x7f;

					if (chr >= ' ' && chr <= 'z')		// would it be ascii
					{											// without bit 7 set?
						kcnt += fprintf(fp, "'%c'+80h", chr);	// yes
						bcnt += 3;
					}
					else							// else do as binary and remove
					{								// ascii flag
						puthex(pgmmem[adrs] & 0xff);
						pgmflags[adrs] &= ~PF_ASCII;
					}
				}
				else								// high bit not set, so is
				{									// really binary, not ascii
					puthex(pgmmem[adrs] & 0xff);
					pgmflags[adrs] &= ~PF_ASCII;
				}
			}
		}

		bcnt++;

		if (bcnt >= BYTELINE || pgmflags[adrs] & PF_ICMT)				// if max bytes per line...
		{
			bcnt = 0;

			if (hexflag)						// do ascii dump of previous bytes
			{
				do
				{
					putc('\t', fp);
					kcnt = (kcnt + 8) & 0x78;
				} while (kcnt < XSTOP);

				fprintf(fp, "; %04x ", padrs);

				for (k=0; k<=off; k++)
					putc(ascii(pgmmem[padrs + k]), fp);

				padrs += k;
				off = 0;
			}
		}
		else
			off++;

		if (pgmflags[adrs] & PF_ICMT)
			output_icomment(adrs);

		--byte_cnt;
		adrs++;
	}

	if (off && hexflag)						// generate comment line
	{
		do
		{
			putc('\t', fp);
			kcnt = (kcnt + 8) & 0x78;
		} while (kcnt < XSTOP);

		fprintf(fp, "; %04x ", padrs);	// show address and ascii for data

		for (k=0; k<off; k++)
			putc(ascii(pgmmem[padrs + k]), fp);
	}

	dump = 1;
}

// end of common.c
