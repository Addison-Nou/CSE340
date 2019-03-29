#include <string>
#include "lexer.h"
#include <vector>
#include <deque>
#include <map>

struct variable{
  std::string type;
  int value;
};

struct scope{
    //Map containing the currently declared variables within the scope
    std::map<std::string, int> variables;

    //Link to previous scope
    scope* previousScope;

    //Counter keeping track of the next available memory slot for the map in the scope
    int nextAvailableMemory = 0;
};

struct stmt{
    //struct scope;             //scope of the program
    std::string stmt_type;      //the type the statement is
    int LHS;
    int operator_symbol;        //operator symbol
    int op1;                    //first variable
    int op2;                    //second variable
    struct stmt* next;          //next statement
};

class project3 {
  public:
    struct stmt* parse_program();
    struct stmt* parse_scope();
    struct stmt* parse_scope_list();
    struct stmt* parse_var_decl();
    struct stmt* parse_id_list();
    struct stmt* parse_type_name();
    struct stmt* parse_stmt_list();
    struct stmt* parse_stmt();
    struct stmt* parse_assign_stmt();
    struct stmt* parse_while_stmt();
    struct stmt* parse_expr();
    struct stmt* parse_arithmetic_operator();
    struct stmt* parse_binary_boolean_operator();
    struct stmt* parse_relational_operator();
    struct stmt* parse_primary();
    struct stmt* parse_bool_const();
    struct stmt* parse_condition();

  private:
    LexicalAnalyzer lexer;

    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();

};