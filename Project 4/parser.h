#include <string>
#include "lexer.h"
#include "compiler.h"

class parser
{
public:
  struct AssignmentStatement *parse_program();
  void parse_var_section();
  void parse_id_list();
  struct StatementNode *parse_body();
  struct StatementNode *parse_stmt_list();
  struct StatementNode *parse_stmt();
  struct AssignmentStatement *parse_assign_stmt();
  Token parse_primary();
  struct AssignmentStatement *parse_expr(struct AssignmentStatement *stmt);
  int parse_op();
  struct PrintStatement *parse_print_stmt();
  struct GotoStatement *parse_while_stmt();
  struct IfStatement *parse_if_stmt();
  struct GotoStatement *parse_switch_stmt();
  struct GotoStatement *parse_for_stmt();

private:
  LexicalAnalyzer lexer;

  int check_var_value(string name);
  struct ValueNode *parser::find_valuenode(string name);
  bool is_integer(string name);
  void syntax_error();
  Token expect(TokenType expected_type);
  Token peek();
};