/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Telecom - Telecom Paristech
 * Telecom ParisTech / dept. TSI
 *
 * Author : Benoit Mathieu
 *
 * This file is part of Yaafe.
 *
 * Yaafe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Yaafe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

%{

#include "yaafe-core/DataFlowParserCommon.h"

#define yylval df_parser_lval
#include "yaafe-core/DataFlowParser.y.hpp"

%}

%option noyywrap

%%

[ ]			 { /* Skip blanks. */ }
useComponentLibrary   { return USELIB; }
:=                       { return ASSIGN; }
>                        { return LINK; }
^#.*	{ return COMMENT; }
[a-zA-Z][a-zA-Z0-9_]*     { yylval = strdup(yytext); return IDENTIFIER; }
[a-zA-Z][a-zA-Z0-9_]*:[a-zA-Z0-9_]+ { yylval = strdup(yytext); return IDENTIFIER_WITH_PORT; }
[0-9]+\.?[0-9]*			 { yylval = strdup(yytext); return VALUE; }
[a-zA-Z0-9.:_/-]+			 { yylval = strdup(yytext); return VALUE; }
\"[^"]+\"       { yylval = strdup(yytext+1); yylval[yyleng-2] = 0; return VALUE; }
\n|.		 { return yytext[0]; }

%%

