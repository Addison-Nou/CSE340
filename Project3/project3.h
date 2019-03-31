#include <string>
#include "lexer.h"
#include <vector>
#include <deque>
#include <map>

struct variable
{
  TokenType type;
  float value;
  std::string stringValue;
  bool initialized;
  int line_no_declared;
};

struct id_line_pair
{
  std::string id;
  int line_no;
};

struct scope
{
  //Map containing the currently declared variables within the scope
  std::map<std::string, int> variables;

  //Link to previous scope
  scope *previousScope;

  std::vector<int> initialized_variables;
};

struct stmt
{
  //struct scope;             //scope of the program
  std::string stmt_type; //the type the statement is
  int LHS;
  TokenType operator_symbol; //operator symbol
  int op1 = -1;              //first variable
  int op2 = -1;              //second variable
  struct stmt *next;         //next statement
  struct stmt *body;         // body stmt

  // X = + 1 + 2 3
  // parse assign stmt  X = temporary_value_1 (op1)
  // parse expr         temporary_value_1 = 1 + temporary_value_2
  // prase expr         temporary_value_2 = 2 + 3
  //------------------------ REVERSE ORDER
  // prase expr         temporary_value_2 = 2 + 3
  // parse expr         temporary_value_1 = 1 + temporary_value_2
  // parse assign stmt  X = temporary_value_1 (op1)
};

class project3
{
public:
  struct stmt *parse_program();
  struct stmt *parse_scope();
  struct stmt *parse_scope_list();
  struct stmt *parse_var_decl();
  std::vector<id_line_pair> parse_id_list();
  TokenType parse_type_name();
  struct stmt *parse_stmt_list();
  struct stmt *parse_stmt();
  struct stmt *parse_assign_stmt();
  struct stmt *parse_while_stmt();
  struct stmt *parse_expr();
  TokenType parse_arithmetic_operator();
  TokenType parse_binary_boolean_operator();
  TokenType parse_relational_operator();
  int parse_primary();
  int parse_bool_const();
  struct stmt *parse_condition();

private:
  LexicalAnalyzer lexer;

  void syntax_error();
  Token expect(TokenType expected_type);
  Token peek();
  int getVariableLocation(std::string variableName, bool lookLocally);
};