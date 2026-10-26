/************************************************************************
 *   IRC - Internet Relay Chat, common/match_ext.h
 *   Copyright (C) 1997 Alain Nissen
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*  This file contains external definitions for global variables and functions
    defined in match.c.
 */

int ircd_tolower(u_char c);
int ircd_toupper(u_char c);
int ircd_match (char *mask, char *name);
char *ircd_collapse (char *pattern);
int ircd_strcmp (char *s1, char *s2);
int ircd_strncmp (char *str1, char *str2, int n);
