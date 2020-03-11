
/*
 * Z80 Disassembler
 * Copyright (C) 1990-2004 by Jeffery L. Post
 * theposts <AT> pacbell <DOT> net
 *
 * dz80table.c - Mnemonic Tables
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

#include	<stdio.h>

#include	"dz80.h"

struct mnementry mnemtbl[] = {
    {"nop"},        {"ld bc,"},     {"ld (bc),a"},  {"inc bc"},    // 00 - 03
    {"inc b"},      {"dec b"},      {"ld b,"},      {"rlca"},      // 04 - 07
    {"ex af,af'"},  {"add hl,bc"},  {"ld a,(bc)"},  {"dec bc"},    // 08 - 0b
    {"inc c"},      {"dec c"},      {"ld c,"},      {"rrca"},      // 0c - 0f
    {"djnz "},      {"ld de,"},     {"ld (de),a"},  {"inc de"},    // 10 - 13
    {"inc d"},      {"dec d"},      {"ld d,"},      {"rla"},       // 14 - 17
    {"jr "},        {"add hl,de"},  {"ld a,(de)"},  {"dec de"},    // 18 - 1b
    {"inc e"},      {"dec e"},      {"ld e,"},      {"rra"},       // 1c - 1f
    {"jr nz,"},     {"ld hl,"},     {"ld "},        {"inc hl"},    // 20 - 23
    {"inc h"},      {"dec h"},      {"ld h,"},      {"daa"},       // 24 - 27
    {"jr z,"},      {"add hl,hl"},  {"ld hl,"},     {"dec hl"},    // 28 - 2b
    {"inc l"},      {"dec l"},      {"ld l,"},      {"cpl"},       // 2c - 2f
    {"jr nc,"},     {"ld sp,"},     {"ld "},        {"inc sp"},    // 30 - 33
    {"inc (hl)"},   {"dec (hl)"},   {"ld (hl),"},   {"scf"},       // 34 - 37
    {"jr c,"},      {"add hl,sp"},  {"ld a,"},      {"dec sp"},    // 38 - 3b
    {"inc a"},      {"dec a"},      {"ld a,"},      {"ccf"},       // 3c - 3f
    {"ld b,b"},     {"ld b,c"},     {"ld b,d"},     {"ld b,e"},    // 40 - 43
    {"ld b,h"},     {"ld b,l"},     {"ld b,(hl)"},  {"ld b,a"},    // 44 - 47
    {"ld c,b"},     {"ld c,c"},     {"ld c,d"},     {"ld c,e"},    // 48 - 4b
    {"ld c,h"},     {"ld c,l"},     {"ld c,(hl)"},  {"ld c,a"},    // 4c - 4f
    {"ld d,b"},     {"ld d,c"},     {"ld d,d"},     {"ld d,e"},    // 50 - 53
    {"ld d,h"},     {"ld d,l"},     {"ld d,(hl)"},  {"ld d,a"},    // 54 - 57
    {"ld e,b"},     {"ld e,c"},     {"ld e,d"},     {"ld e,e"},    // 58 - 5b
    {"ld e,h"},     {"ld e,l"},     {"ld e,(hl)"},  {"ld e,a"},    // 5c - 5f
    {"ld h,b"},     {"ld h,c"},     {"ld h,d"},     {"ld h,e"},    // 60 - 63
    {"ld h,h"},     {"ld h,l"},     {"ld h,(hl)"},  {"ld h,a"},    // 64 - 67
    {"ld l,b"},     {"ld l,c"},     {"ld l,d"},     {"ld l,e"},    // 68 - 6b
    {"ld l,h"},     {"ld l,l"},     {"ld l,(hl)"},  {"ld l,a"},    // 6c - 6f
    {"ld (hl),b"},  {"ld (hl),c"},  {"ld (hl),d"},  {"ld (hl),e"}, // 70 - 73
    {"ld (hl),h"},  {"ld (hl),l"},  {"halt"},       {"ld (hl),a"}, // 74 - 77
    {"ld a,b"},     {"ld a,c"},     {"ld a,d"},     {"ld a,e"},    // 78 - 7b
    {"ld a,h"},     {"ld a,l"},     {"ld a,(hl)"},  {"ld a,a"},    // 7c - 7f
    {"add a,b"},    {"add a,c"},    {"add a,d"},    {"add a,e"},   // 80 - 83
    {"add a,h"},    {"add a,l"},    {"add a,(hl)"}, {"add a,a"},   // 84 - 87
    {"adc a,b"},    {"adc a,c"},    {"adc a,d"},    {"adc a,e"},   // 88 - 8b
    {"adc a,h"},    {"adc a,l"},    {"adc a,(hl)"}, {"adc a,a"},   // 8c - 8f
    {"sub b"},      {"sub c"},      {"sub d"},      {"sub e"},     // 90 - 93
    {"sub h"},      {"sub l"},      {"sub (hl)"},   {"sub a"},     // 94 - 97
    {"sbc a,b"},    {"sbc a,c"},    {"sbc a,d"},    {"sbc a,e"},   // 98 - 9b
    {"sbc a,h"},    {"sbc a,l"},    {"sbc a,(hl)"}, {"sbc a,a"},   // 9c - 9f
    {"and b"},      {"and c"},      {"and d"},      {"and e"},     // a0 - a3
    {"and h"},      {"and l"},      {"and (hl)"},   {"and a"},     // a4 - a7
    {"xor b"},      {"xor c"},      {"xor d"},      {"xor e"},     // a8 - ab
    {"xor h"},      {"xor l"},      {"xor (hl)"},   {"xor a"},     // ac - af
    {"or b"},       {"or c"},       {"or d"},       {"or e"},      // a0 - a3
    {"or h"},       {"or l"},       {"or (hl)"},    {"or a"},      // a4 - a7
    {"cp b"},       {"cp c"},       {"cp d"},       {"cp e"},      // a8 - ab
    {"cp h"},       {"cp l"},       {"cp (hl)"},    {"cp a"},      // ac - af
    {"ret nz"},     {"pop bc"},     {"jp nz,"},     {"jp "},       // c0 - c3
    {"call nz,"},   {"push bc"},    {"add a,"},     {"rst 0"},     // c4 - c7
    {"ret z"},      {"ret"},        {"jp z,"},      {""},          // c8 - cb
    {"call z,"},    {"call "},      {"adc a,"},     {"rst 8"},     // cc - cf
    {"ret nc"},     {"pop de"},     {"jp nc,"},     {"out "},      // d0 - d3
    {"call nc,"},   {"push de"},    {"sub "},       {"rst 10h"},   // d4 - d7
    {"ret c"},      {"exx"},        {"jp c,"},      {"in a,"},     // d8 - db
    {"call c,"},    {""},           {"sbc a,"},     {"rst 18h"},   // dc - df
    {"ret po"},     {"pop hl"},     {"jp po,"},     {"ex (sp),hl"},// e0 - e3
    {"call po,"},   {"push hl"},    {"and "},       {"rst 20h"},   // e4 - e7
    {"ret pe"},     {"jp (hl)"},    {"jp pe,"},     {"ex de,hl"},  // e8 - eb
    {"call pe,"},   {""},           {"xor "},       {"rst 28h"},   // ec - ef
    {"ret p"},      {"pop af"},     {"jp p,"},      {"di"},        // f0 - f3
    {"call p,"},    {"push af"},    {"or "},        {"rst 30h"},   // f4 - f7
    {"ret m"},      {"ld sp,hl"},   {"jp m,"},      {"ei"},        // f8 - fb
    {"call m,"},    {""},           {"cp "},        {"rst 38h"}    // fc - ff
} ;

//  Indexed codes (dd, fd ..)

struct mnementry cbtbl[] = {
    {"rlc "},   {"rrc "},   {"rl "},    {"rr "},        // 00 - 1f
    {"sla "},   {"sra "},   {""},       {"srl "},       // 20 - 3f
    {"bit 0,"}, {"bit 1,"}, {"bit 2,"}, {"bit 3,"},     // 40 - 5f
    {"bit 4,"}, {"bit 5,"}, {"bit 6,"}, {"bit 7,"},     // 60 - 7f
    {"res 0,"}, {"res 1,"}, {"res 2,"}, {"res 3,"},     // 80 - 9f
    {"res 4,"}, {"res 5,"}, {"res 6,"}, {"res 7,"},     // a0 - bf
    {"set 0,"}, {"set 1,"}, {"set 2,"}, {"set 3,"},     // c0 - df
    {"set 4,"}, {"set 5,"}, {"set 6,"}, {"set 7,"}      // e0 - ff
};

struct mnementry regtbl[] = {
    {"b"},      {"c"},      {"d"},      {"e"},
    {"h"},      {"l"},      {"(hl)"},   {"a"},
};

struct mnementry ddcbtbl[] = {
    {"rlc "},   {"rrc "},   {"rl "},    {"rr "},
    {"sla "},   {"sra "},   {"??? "},   {"srl "},
    {"bit "},   {"res "},   {"set "}
};

struct mnementry dd1tbl[] = {
    {"add "},   {"add "},   {"inc "},   {"add "},
    {"dec "},   {"add "},   {"pop "},   {"ex (sp),"},
    {"push "},  {"jp ("},   {"ld sp,"}
};

struct mnementry dd2tbl[] = {
    {"add a,(i"},   {"adc a,(i"},   {"sub (i"},
    {"sbc a,(i"},   {"and (i"},     {"xor (i"},
    {"or (i"},      {"cp (i"}
};

//  Miscellaneous codes (ed ..)

struct mnementry edtbl[] = {
    {"in b,(c)"},   {"out (c),b"},  {"sbc hl,bc"},  {"ld "},        // 40-43
    {"neg"},        {"retn"},       {"im 0"},       {"ld i,a"},     // 44-47
    {"in c,(c)"},   {"out (c),c"},  {"adc hl,bc"},  {"ld bc,"},     // 48-4b
    {"defb "},      {"reti"},       {"defb "},      {"ld r,a"},     // 4c-4f
    {"in d,(c)"},   {"out (c),d"},  {"sbc hl,de"},  {"ld "},        // 50-53
    {"defb "},      {"defb "},      {"im 1"},       {"ld a,i"},     // 54-57
    {"in e,(c)"},   {"out (c),e"},  {"adc hl,de"},  {"ld de,"},     // 58-5b
    {"defb "},      {"defb "},      {"im 2"},       {"ld a,r"},     // 5c-5f
    {"in h,(c)"},   {"out (c),h"},  {"sbc hl,hl"},  {"defb "},      // 60-63
    {"defb "},      {"defb "},      {"defb "},      {"rrd"},        // 64-67
    {"in l,(c)"},   {"out (c),l"},  {"adc hl,hl"},  {"defb "},      // 68-6b
    {"defb "},      {"defb "},      {"defb "},      {"rld"},        // 6c-6f
    {"defb "},      {"defb "},      {"sbc hl,sp"},  {"ld "},        // 70-73
    {"defb "},      {"defb "},      {"defb "},      {"defb "},      // 74-77
    {"in a,(c)"},   {"out (c),a"},  {"adc hl,sp"},  {"ld sp,"},     // 78-7b

    {"ldi"},        {"cpi"},        {"ini"},        {"outi"},       // a0-a3
    {"defb "},      {"defb "},      {"defb "},      {"defb "},      // a4-a7
    {"ldd"},        {"cpd"},        {"ind"},        {"outd"},       // a8-ab
    {"defb "},      {"defb "},      {"defb "},      {"defb "},      // ac-af
    {"ldir"},       {"cpir"},       {"inir"},       {"otir"},       // b0-b3
    {"defb "},      {"defb "},      {"defb "},      {"defb "},      // b4-b7
    {"lddr"},       {"cpdr"},       {"indr"},       {"otdr"}        // b8-bb
};

/* OPTTBL (option table) entries:

   bit 7 = unconditional transfer instruction
   bit 6 = special processing (cb, dd, ed, fd opcodes)
   bit 5 = relative addressing (jr & djnz instructions)
   bit 4 = direct reference (jp or call)
   bit 3 = parenthesized address
   bit 2 = immediate data
   bit 1 = 3 byte instruction
   bit 0 = 2 byte instruction
      if entry = 0, instruction is single byte, no options
*/

unsigned char opttbl[] = {
   0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,    // 00 - 07
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,    // 08 - 0f
   0x21, 0x16, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,    // 10 - 17
   0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,    // 18 - 1f
   0x21, 0x16, 0x1e, 0x00, 0x00, 0x00, 0x05, 0x00,    // 20 - 27
   0x21, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x05, 0x00,    // 28 - 2f
   0x21, 0x16, 0x1e, 0x00, 0x00, 0x00, 0x05, 0x00,    // 30 - 37
   0x21, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x05, 0x00,    // 38 - 3f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 40 - 47
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 48 - 4f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 50 - 57
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 58 - 5f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 60 - 67
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 68 - 6f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,    // 70 - 77
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 78 - 7f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 80 - 87
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 88 - 8f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 90 - 97
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 98 - 9f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // a0 - a7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // a8 - af
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // b0 - b7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // b8 - bf
   0x00, 0x00, 0x16, 0x96, 0x16, 0x00, 0x05, 0x00,    // c0 - c7
   0x00, 0x80, 0x16, 0x41, 0x16, 0x16, 0x05, 0x00,    // c8 - cf
   0x00, 0x00, 0x16, 0x09, 0x16, 0x00, 0x05, 0x00,    // d0 - d7
   0x00, 0x00, 0x16, 0x09, 0x16, 0x40, 0x05, 0x00,    // d8 - df
   0x00, 0x00, 0x16, 0x00, 0x16, 0x00, 0x05, 0x00,    // e0 - e7
   0x00, 0x80, 0x16, 0x00, 0x16, 0x40, 0x05, 0x00,    // e8 - ef
   0x00, 0x00, 0x16, 0x00, 0x16, 0x00, 0x05, 0x00,    // f0 - f7
   0x00, 0x00, 0x16, 0x00, 0x16, 0x40, 0x05, 0x00     // f8 - ff
} ;

/*  Flags for ED codes:
    00 - invalid code
    01 - 2 byte, entry in edtbl
    03 - xx,(nn) where xx = bc, de, or sp
    13 - (nn),bc
    23 - (nn),de
    33 - (nn),sp
    81 - retn or reti
*/
     
unsigned char edcode[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 00 - 07
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 08 - 0f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 10 - 17
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 18 - 1f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 20 - 27
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 28 - 2f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 30 - 37
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 38 - 3f
   0x01, 0x01, 0x01, 0x13, 0x01, 0x81, 0x01, 0x01,    // 40 - 47
   0x01, 0x01, 0x01, 0x03, 0x00, 0x81, 0x00, 0x01,    // 48 - 4f
   0x01, 0x01, 0x01, 0x23, 0x00, 0x00, 0x01, 0x01,    // 50 - 57
   0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x01, 0x01,    // 58 - 5f
   0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,    // 60 - 67
   0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,    // 68 - 6f
   0x00, 0x00, 0x01, 0x33, 0x00, 0x00, 0x00, 0x00,    // 70 - 77
   0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00,    // 78 - 7f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 80 - 87
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 88 - 8f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 90 - 97
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 98 - 9f
   0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,    // a0 - a7
   0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,    // a8 - af
   0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,    // b0 - b7
   0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,    // b8 - bf
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // c0 - c7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // c8 - cf
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // d0 - d7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // d8 - df
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // e0 - e7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // e8 - ef
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // f0 - f7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00     // f8 - ff
} ;

/*  Flags for DD and FD codes:
    00 - invalid code
    x1 - 2 byte entry in dd1tbl
    x2 - 3 byte load codes
    x3 - 4 byte direct address codes
    x6 - 3 byte arithmetic codes
    x7 - 4 byte dd/fd cb codes
*/
unsigned char ddcode[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 00 - 07
   0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 08 - 0f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 10 - 17
   0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 18 - 1f
   0x00, 0x03, 0x13, 0x21, 0x00, 0x00, 0x00, 0x00,    // 20 - 27
   0x00, 0x31, 0x23, 0x41, 0x00, 0x00, 0x00, 0x00,    // 28 - 2f
   0x00, 0x00, 0x00, 0x00, 0x02, 0x12, 0x33, 0x00,    // 30 - 37
   0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // 38 - 3f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x00,    // 40 - 47
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00,    // 48 - 4f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00,    // 50 - 57
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x00,    // 58 - 5f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x00,    // 60 - 67
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x00,    // 68 - 6f
   0x82, 0x92, 0xa2, 0xb2, 0xc2, 0xd2, 0x00, 0xe2,    // 70 - 77
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf2, 0x00,    // 78 - 7f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,    // 80 - 87
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00,    // 88 - 8f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00,    // 90 - 97
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00,    // 98 - 9f
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00,    // a0 - a7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x00,    // a8 - af
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x00,    // b0 - b7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00,    // b8 - bf
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // c0 - c7
   0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,    // c8 - cf
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // d0 - d7
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // d8 - df
   0x00, 0x61, 0x00, 0x71, 0x00, 0x81, 0x00, 0x00,    // e0 - e7
   0x00, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // e8 - ef
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // f0 - f7
   0x00, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00     // f8 - ff
} ;

//  end of dz80table.c

