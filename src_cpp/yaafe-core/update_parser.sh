#!/bin/sh
flex -Pdf_parser_ -o DataFlowParser.l.c DataFlowParser.l
bison -d -p df_parser_ DataFlowParser.y -o DataFlowParser.y.cpp
