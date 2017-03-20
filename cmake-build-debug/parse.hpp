/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_HOME_LYANG_PROJECTS_REDBASE_CMAKE_BUILD_DEBUG_PARSE_HPP_INCLUDED
# define YY_YY_HOME_LYANG_PROJECTS_REDBASE_CMAKE_BUILD_DEBUG_PARSE_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    RW_CREATE = 258,
    RW_DROP = 259,
    RW_TABLE = 260,
    RW_INDEX = 261,
    RW_LOAD = 262,
    RW_SET = 263,
    RW_HELP = 264,
    RW_PRINT = 265,
    RW_EXIT = 266,
    RW_SELECT = 267,
    RW_FROM = 268,
    RW_WHERE = 269,
    RW_INSERT = 270,
    RW_DELETE = 271,
    RW_UPDATE = 272,
    RW_AND = 273,
    RW_INTO = 274,
    RW_VALUES = 275,
    T_EQ = 276,
    T_LT = 277,
    T_LE = 278,
    T_GT = 279,
    T_GE = 280,
    T_NE = 281,
    T_EOF = 282,
    T_OVERLAP = 283,
    NOTOKEN = 284,
    RW_RESET = 285,
    RW_IO = 286,
    RW_BUFFER = 287,
    RW_RESIZE = 288,
    RW_QUERY_PLAN = 289,
    RW_ON = 290,
    RW_OFF = 291,
    T_INT = 292,
    T_REAL = 293,
    T_STRING = 294,
    T_QSTRING = 295,
    T_SHELL_CMD = 296,
    T_MBR = 297
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 72 "src/parse.y" /* yacc.c:1909  */

    int ival;
    CompOp cval;
    float rval;
    char *sval;
    NODE *n;
	struct MBR mval;

#line 106 "/home/lyang/Projects/redbase/cmake-build-debug/parse.hpp" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_HOME_LYANG_PROJECTS_REDBASE_CMAKE_BUILD_DEBUG_PARSE_HPP_INCLUDED  */
