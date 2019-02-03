/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

struct stmt{
  int stmt_type;
  int LHS;
  int operand;
  int op1;
  int op2;
  struct stmt* next;
  struct stmt* proc_code; // Pointer to the statement object (beginning of the procedure)

  // TODO: When you construct procedures, we will need to create another hashmap. 
  // Key: string (ID or NUM) 
  // Value: Pointer to statement (a.k.a the address) of the statement to execute. 


}; 

typedef enum { INVALID = 0,
    ASSIGN_STMT, OUTPUT_STMT, INPUT_STMT, PROC_INVOCATION_STMT, DO_STMT
    } StatementType;

class Parser {
  public:
    struct stmt* parse_input();

  private:
    LexicalAnalyzer lexer;
    struct stmt* parse_program();

    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();
    struct stmt* parse_input_statement();
    void parse_proc_decl_section();
    void parse_proc_decl();
    std::string parse_procedure_name();
    struct stmt* parse_procedure_body();
    struct stmt* parse_statement();
    struct stmt* parse_statement_list();
    struct stmt* parse_output_statement();
    struct stmt* parse_procedure_invocation();
    struct stmt* parse_assign_statement();
    void parse_expr(struct stmt* st);
    int parse_operator();
    int parse_primary();
    struct stmt* parse_main();
    void parse_inputs();
    struct stmt* parse_do_statement();
};

#endif

