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
#include "yaafe-core/DataFlowParserContext.h"
#include <iostream>
#include <string.h>

#define STR(tok) std::string(tok.str,tok.len)

extern "C"
{
        int yylex(void);  
        int yywrap()
        {
                return 1;
        }

}

int df_parser_error(YAAFE::DataFlowContext& context, char* errstr);
char* prepare_identifier_with_port(char* str);
void free_identifier_with_port(char* id, char* port);

%}

%parse-param { class YAAFE::DataFlowContext& context }

%token USELIB
%token IDENTIFIER
%token IDENTIFIER_WITH_PORT
%token VALUE
%token ASSIGN
%token LINK
%token COMMENT

%%
lines:	lines uselib_expr '\n'
    |   lines node_expr '\n'
	|	lines link_expr '\n'
	|	lines '\n'
	|	/* empty */
	|   lines COMMENT '\n'		
	|	error '\n' 		{ yyerrok; }
	;

uselib_expr: USELIB VALUE {
				// printf("use library %s\n",$2);
				context.m_dataflow->useComponentLibrary($2);
				free($2);
			}
	;

node_expr:	IDENTIFIER ASSIGN IDENTIFIER param_expr	{
				// printf("process node_expr %s to %s\n",$1,$3);
				YAAFE::Graph<YAAFE::NodeDesc>::Node* n = context.m_dataflow->createNode(YAAFE::NodeDesc($3,context.m_params));
				if ((strcmp($3,"Input")==0) || (strcmp($3,"Output")==0)) {
					context.m_dataflow->setNodeName(n,$1);
				}
				context.m_identifiers[$1] = n; 
				context.m_params.clear();
				free($1); free($3)
			}
	;

param_expr:  param_expr IDENTIFIER '=' VALUE	{
				context.m_params[$2] = $4;
				free($2); free($4);
			}
	| param_expr IDENTIFIER '=' IDENTIFIER	{
				context.m_params[$2] = $4;
				free($2); free($4);
			}
	|	/* empty */
	;

link_expr:	IDENTIFIER LINK IDENTIFIER {
				// printf("process link_expr %s to %s\n",$1,$3);
				context.m_dataflow->link(context.m_identifiers[$1],"",context.m_identifiers[$3],"");
				free($1); free($3);
			}
	| IDENTIFIER_WITH_PORT LINK IDENTIFIER {
				char* port = prepare_identifier_with_port($1);
				context.m_dataflow->link(context.m_identifiers[$1],port,context.m_identifiers[$3],"");
				free_identifier_with_port($1,port);
				free($3);
			}
	| IDENTIFIER LINK IDENTIFIER_WITH_PORT {
				char* port = prepare_identifier_with_port($3);
				context.m_dataflow->link(context.m_identifiers[$1],"",context.m_identifiers[$3],port);
				free($1);
				free_identifier_with_port($3,port);
			}
	| IDENTIFIER_WITH_PORT LINK IDENTIFIER_WITH_PORT {
				char* port1 = prepare_identifier_with_port($1);
				char* port3 = prepare_identifier_with_port($3);
				context.m_dataflow->link(context.m_identifiers[$1],port1,context.m_identifiers[$3],port3);
				free_identifier_with_port($1,port1);
				free_identifier_with_port($3,port3);
			}
	;
	
%%

#include <ctype.h>
#include <stdio.h>

int df_parser_error(YAAFE::DataFlowContext& context, char* errstr) {
	printf("Dataflow parser error: %s\n", errstr);
	return -1;
}

char* prepare_identifier_with_port(char* str)
{
	// printf("call prepare_identifier_with_port on %s\n",str);
	char* port = str;
	while (*port!=':') port++;
	*port = 0;
	port++;
	return port;
}

void free_identifier_with_port(char* id, char* port)
{
	port--;
	*port = ':';
	free(id);
}

