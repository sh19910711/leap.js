%{

#include <string.h>
#include "algebra.h"
#include "y.tab.h"

	/* other includes */

int lineno = 1;
char linebuf[500];

void yyerror(char *s);
%}

%%

\n	lineno++;
		

#.*\n	; /* comment */ 

SELECT		{ return SELECT; }
UNION		{ return UNION; }
JOIN		{ return JOIN; }
INTERSECT	{ return INTERSECT; }
DIFFERENCE	{ return DIFFERENCE; }
MINUS		{ return DIFFERENCE; }
DIVIDE		{ return DIVIDE; }
PROJECT		{ return PROJECT; }
PRODUCT		{ return PRODUCT; }
DISPLAY		{ return DISPLAY; }
NOT		{ return NOT; }
OR		{ return OR; }
AND		{ return AND; }

LIST		{ return LIST; }
ADD		{ return ADD; }

EXIT	{ return EXIT; }



[-+*/:(),.]	{ return yytext[0]; }

	/* names */
[A-Za-z][A-Za-z0-9_]*	{ yylval.strval=strdup(yytext); return NAME; }

	/* numbers */
[0-9]+	|
[0-9]+"."[0-9]* |
"."[0-9]*	{ yylval.strval=strdup(yytext); return INTNUM; }

	/* strings */

\'[^\'\n]*\' {
	/* int c=input(); 
	
	unput(c);
	printf("woohoo (c=[%c]!",c);
	if (c!='\''){*/
		yylval.strval=strdup(yytext); 
		printf("Got: [%s]",yytext);
		return STRING;
	/* } else
		yymore();*/
}

\'[^\'\n]*$	{ yyerror("Unterminated string"); }

"="	|
"<>"	|
"<"	|
">"	|
"<="	|
">="	{ yylval.strval=strdup(yytext); return COMPARISON; }


[ \t\r]+	; 

; 	{ return TERMINATOR; }


.	yyerror("invalid character"); 


%%

void yyerror(char *s) {

	printf("%d: %s at %s",lineno, s, yytext);

}

int init_parse() {
	parse_tree ptree;
	
	
	if(!yyparse()) {
		BEGIN INITIAL;
		return(1);
	} else {
		BEGIN INITIAL;
		return(0);
	}
}

int yywrap() {
	return(1);
}

int done_parse() {

	BEGIN INITIAL;

}
