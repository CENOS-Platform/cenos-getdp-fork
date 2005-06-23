/*
 * Copyright (C) 1997-2005 P. Dular, C. Geuzaine
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to <getdp@geuz.org>.
 *
 * Contributor(s):
 *   Ruth Sabariego
 */

/* FMM truncation number for 2D & 3D Laplace Expansions */

/* 1e-3 precision */

int t2D1e_3[49] = { 3, 4, 4, 5, 5, 6, 7, 
		    3, 4, 4, 5, 6, 6, 7,
		    4, 4, 4, 5, 6, 6, 7,
		    4, 4, 5, 5, 6, 7, 7,
		    4, 5, 5, 5, 6, 7, 8,
		    5, 5, 5, 6, 7, 7, 8,
		    6, 6, 6, 7, 7, 8, 9};

int t3D1e_3[49] = { 2, 3, 3, 4, 5, 5, 6, 
		    3, 3, 3, 4, 5, 6, 7, 
		    3, 3, 4, 4, 5, 6, 7, 
		    4, 4, 4, 5, 6, 7, 8,
		    5, 5, 5, 6, 6, 7, 9,
		    5, 6, 6, 7, 7, 8,10,
		    6, 7, 7, 8, 9,10,12 };


/* 1e-4 precision */
int t2D1e_4[49] = { 4, 5, 6, 6, 7, 8, 9, 
		    4, 5, 6, 6, 7, 8, 9, 
		    5, 5, 6, 6, 7, 8, 9,
		    5, 5, 6, 7, 8, 9, 10,
		    6, 6, 6, 7, 8, 9, 10,
		    7, 7, 7, 8, 9, 10,11,
		    8, 8, 8, 9,10, 11,13};


int t3D1e_4[49] = { 3, 4, 4, 5, 6, 7, 9, 
		    4, 4, 5, 6, 7, 8, 9,
		    4, 5, 5, 6, 6, 8,10,
		    5, 6, 6, 7, 8, 9,11,
		    6, 7, 7, 8, 9,10,12,
		    7, 8, 8, 9,10,11,13,
		    8, 9,10,11,12,13,15 };


/* 1e-5 precision */
int t2D1e_5[49] = {5, 6, 7, 8, 9,10,11,
		   5, 6, 7, 8, 9,10,11,
		   6, 6, 7, 8, 9,10,12,
		   7, 7, 7, 8, 9,11,12,
		   8, 8, 8, 9,10,11,13,
		   9, 9, 9,10,11,12,14,
		   10,10,11,11,12,14,16};


int t3D1e_5[49] = {4, 5, 6, 7, 8, 9,11, 
		   5, 5, 6, 7, 8,10,12, 
		   6, 6, 7, 8, 9,11,12, 
		   7, 7, 8, 8,10,11,13,
		   8, 8, 9,10,11,12,15,
		   9,10,11,11,12,14,16,
		   11,12,12,13,15,16,19};


/* 1e-6 precision */
int t2D1e_6[49] = {5, 7, 8, 9,10,11,13,
		   6, 7, 8, 9,10,12,13,
		   7, 7, 8, 9,11,12,14,
		   8, 8, 9,10,11,13,15,
		   9, 9,10,10,12,14,16,
		   10,11,11,12,13,15,17,
		   12,12,13,14,15,17,20};

int t3D1e_6[49] = {4, 6, 7, 8,10,11,13,
		   6, 6, 7, 9,10,12,14,
		   7, 7, 8, 9,11,13,15,
		   8, 9, 9,10,11,14,16,
		   9,10,11,11,13,15,18,
		   11,12,13,14,15,17,20,
		   13,14,15,16,18,20,22};


/* 1e-7 precision */
int t2D1e_7[49] = {6, 8, 9,10,12,13,15,
		   7, 8, 9,11,12,14,16,
		   8, 8, 9,11,12,14,17,
                   9,10,10,11,13,15,17,
		   10,11,11,12,14,16,19,
		   12,13,13,14,15,17,20,
		   14,15,16,16,18,20,23 };

int t3D1e_7[49] = {5, 7, 8,10,12,13,16, 
		   6, 7, 9,10,12,14,17, 
		   8, 9, 9,11,13,15,18,
		   10,10,11,12,13,16,19,
		   11,12,12,13,15,17,21,
		   13,14,15,16,17,19,22,
		   14,14,16,17,18,20,23};

/* 1e-8 precision */
int t2D1e_8[49] = {7, 9,10,12,13,15,17,
		   8, 9,10,12,14,16,18,
		   9, 9,11,12,14,16,19,
		   11,11,11,13,15,17,20,
		   12,13,13,14,16,18,22,
		   14,15,15,16,18,20,24,
                   16,17,18,19,21,23,27};

int t3D1e_8[49] = {6, 8, 9,11,13,15,18, 
		   8, 8,10,12,14,16,19,
		   9,10,11,12,15,17,20,
		   11,12,12,13,15,18,22,
		   11,13,13,14,16,19,23,
		   13,14,15,16,17,21,24,
		   14,14,16,17,18,22,24};


/* 1e-9 precision */
int t2D1e_9[49] = {8,10,11,13,15,17,19,
		   9,10,12,13,15,18,20,
		   10,11,12,14,16,19,21,
		   12,12,13,14,17,19,23,
		   14,14,15,16,18,21,24,
		   16,17,18,18,20,22,27,
		   18,19,20,22,23,26,30};

int t3D1e_9[49] = {7, 9,11,13,15,17,20, 
		   8, 9,11,13,16,18,21,
		   10,11,12,14,16,19,22,
		   12,13,14,15,17,20,23,
		   14,15,16,17,19,21,24,
		   17,18,19,20,22,24,27,
		   19,20,21,22,25,27,31};
