#include <string>
#include "lexer.h"
#include "compiler.h"

class parser
{
public:
  struct AssignmentStatement parse_program();
  void parse_var_section();
  void parse_id_list();
  struct AssignmentStatement parse_body();
  struct AssignmentStatement parse_stmt_list();
  struct AssignmentStatement parse_stmt();
  struct AssignmentStatement parse_assign_stmt();
  struct AssignmentStatement parse_print_stmt();
  struct AssignmentStatement parse_while_stmt();
  struct AssignmentStatement parse_if_stmt();
  struct AssignmentStatement parse_switch_stmt();
  struct AssignmentStatement parse_for_stmt();

private:
  LexicalAnalyzer lexer;

  int check_var_value(string name);
  struct ValueNode* parser::find_valuenode(string name);
  void syntax_error();
  Token expect(TokenType expected_type);
  Token peek();
};