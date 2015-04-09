/* 
 * Name: test.c
 * Description: basic test harness
 * Version: testutil.c,v 1.201.2.1 2004/02/09 20:05:20 rleyton Exp
 *
 *   LEAP RDBMS - An extensible and free RDBMS
 *   Copyright (C) 1995-2004 Richard Leyton
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   See the file COPYING for more information.
 *
 *   Richard Leyton
 *   leap@leyton.org
 *   http://leap.sourceforge.net
 *   http://www.leyton.org
 *
 */


#include "util.h"


void main () {

	char test[180],post[180],p2[180];


	strcpy(test,"(subject,STRING,10)");

	printf("pre: >%s<\n",test);

	cut_to_right_bracket(test,1,FALSE,post);
	printf("post: >%s<\ntest: >%s<\n",post,test);
	cut_to_right_bracket(post,1,FALSE,p2);
	printf("post: >%s<\np2: >%s<\n",post,p2);

}
