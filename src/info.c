/* 
 * Name: info.c
 * Description: Information routines.
 * Version: info.c,v 1.206.2.1 2004/02/09 20:05:20 rleyton Exp
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


#ifdef HAVE_CONFIG_H
# include "defines.h"
#endif
#include <stdio.h>
#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "leapio.h"
#include "info.h"
#include "util.h"

void print_header(boolean brief) {
/* print_version
 * Prints the LEAP version header
 */
	
	if (status_quiet!=TRUE) {
	/* First things first, report what we are. */
		if (LEAP_REVISION_VERSION!=0) {
			leap_fprintf(stdout,"LEAP RDBMS %d.%d.%d",LEAP_MAJOR_VERSION,LEAP_MINOR_VERSION,LEAP_REVISION_VERSION);
		} else {
			leap_fprintf(stdout,"LEAP RDBMS %d.%d",LEAP_MAJOR_VERSION,LEAP_MINOR_VERSION);
		}
		if (LEAP_PATCH!=0) {
			leap_fprintf(stdout,".%d",LEAP_PATCH);
		}
		leap_fprintf(stdout," - %s\n",LEAP_VERSION_TEXT);
		leap_fprintf(stdout,"%s\n",LEAP_COPYRIGHT);
		if (brief==FALSE) {
				leap_fprintf(stdout,"\n%s\n",LEAP_DISTRIBUTION_1);
				leap_fprintf(stdout,"%s\n",LEAP_DISTRIBUTION_2);
				leap_fprintf(stdout,"%s\n",LEAP_DISTRIBUTION_3);
				if ((LEAP_MINOR_VERSION % 2)==1) {
					leap_fprintf(stdout,"*****************************\n");
					leap_fprintf(stdout,"* THIS IS A >>BETA<< REVISION\n");
					leap_fprintf(stdout,"*****************************\n");
				}
				leap_fprintf(stdout,"\n");
		}
	}

#ifdef FULL_DEBUG
	fprintf(stderr,"FULL-DEBUG (VERY NOISY)\n\n");
	#define DEBUG
#endif
#ifdef DEBUG
	fprintf(stderr,"DEBUG VERSION!\n");
#endif

}

void do_addresses() {
/* do_addresses
 * displays contact information etc.
 */
    leap_printf("Addresses for comments/feedback etc.\n\n");
    leap_printf("Author: Richard Leyton\n");
    leap_printf(" E-Mail: leap@leyton.org\n");
    leap_printf(" Snail: c/o 3.Pelting Drove, Priddy, WELLS, Somerset, BA5 3BA, UK\n\n");
    leap_printf("Free Software Foundation\n");
    leap_printf(" Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n");
    leap_printf("For a full list see the LEAP home page:\n");
    leap_printf(" http://leap.sourceforge.net\n");
}

void print_info() {
/* print_info
 * displays the warranty and conditions blurb
 */
    leap_printf("This program is free software; you can redistribute it and/or modify\n");
    leap_printf("it under the terms of the GNU General Public License as published by\n");
    leap_printf("the Free Software Foundation; either version 2 of the License, or\n");
    leap_printf("(at your option) any later version.\n");
    leap_printf("\n");
    leap_printf("This program is distributed in the hope that it will be useful,\n");
    leap_printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    leap_printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    leap_printf("GNU General Public License for more details.\n");
    leap_printf("\n");
    leap_printf("You should have received a copy of the GNU General Public License\n");
    leap_printf("along with this program; if not, write to the Free Software\n");
    leap_printf("Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n");
    leap_printf("\n");
    leap_printf("See the file COPYING for full details, or write to the address above.\n");
    leap_printf("Type \"addresses\" for a list of addresses\n");
}

void print_help() {
/* print_help
 * Print command line help and so on
 */
	print_header(TRUE);

	leap_printf("\nleap [options]\n\n");
	leap_printf("--activity-file file - Output activity file to file\n");
	leap_printf("--configure script   - Configure (install) LEAP\n");
	leap_printf("--database db        - Database to open\n");
	leap_printf("--debug [n]          - Enable Debug Mode (with optional level)\n");
	leap_printf("--directory dir      - LEAP directory\n");
	leap_printf("--long-commands      - Long commands enabled.\n");
	leap_printf("--merge-stderr       - Merge stderr into stdout.\n");
	leap_printf("--help               - This help page\n");
	leap_printf("--padding            - Pad relation names to attributes\n");
	leap_printf("--product-join       - Enable product in joins.\n");
	leap_printf("--quiet              - Quiet mode\n");
	leap_printf("--status             - Status messages\n");
	leap_printf("--time-logging       - Disable times in log messages\n");
	leap_printf("--timing             - Timing information\n");
	leap_printf("--tracing            - Tracing information\n");
	leap_printf("--version            - Print version information\n\n");
	leap_printf("--warranty           - Warranty and conditions of use\n");
	leap_printf("\ndir should be base LEAP Directory, containing the database\n");
	leap_printf("directory.\n\n");
	leap_printf("Default directory is ~/leap/ - ../ is checked on failure.\n\n");
}

void print_shutdown() {
/* print_shutdown
 * Print the shutdown message 
 */
	if (status_quiet!=TRUE) {
		leap_printf("\nPlease send all comments, bugs, suggestions etc. to:\n");
		leap_printf("leap@leyton.org\n\n");
		leap_printf("Latest versions are available via anonymous ftp:\n");
		leap_printf("All   - ftp.demon.co.uk/pub/compsci/databases/leap\n");
		leap_printf("Un*x  - sunsite.unc.edu/pub/Linux/apps/database/relational\n\n");
		leap_printf("For up to date information, ftp sites, and developments - see\n");
		leap_printf("the LEAP Web Page at http://leap.sourceforge.net\n\n");
		leap_printf("For announcements, and information, join the LEAP mailing\n");
		leap_printf("list http://lists.sourceforge.net/lists/listinfo/leap-announce");
	}
}

void do_warranty() {
/* do_warranty
 * Print the warrant information of LEAP
 */
print_header(TRUE);
leap_printf("\n                            NO WARRANTY\n");
leap_printf("  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n");
leap_printf("FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n");
leap_printf("OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n");
leap_printf("PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n");
leap_printf("OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n");
leap_printf("MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n");
leap_printf("TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n");
leap_printf("PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n");
leap_printf("REPAIR OR CORRECTION.\n\n");
leap_printf("  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n");
leap_printf("WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n");
leap_printf("REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n");
leap_printf("INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n");
leap_printf("OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n");
leap_printf("TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n");
leap_printf("YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n");
leap_printf("PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n");
leap_printf("POSSIBILITY OF SUCH DAMAGES.\n");


}
