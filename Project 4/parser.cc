#include <iostream>
#include <string>
#include <vector>
#include "parser.h"

using namespace std;

//Vector of value nodes
std::vector<struct ValueNode*> memory;

//Start of parser

//Loops through the memory and checks variable's value; returns -1 if it does not
int parser::check_var_value(string name){

    for (std::vector<struct ValueNode*>::iterator it = memory.begin(); it != memory.end(); it++){
        if ((*it)->name == name)
            return (*it)->value;
    }

    return -1;
}

//Loops through memory to find the ValueNode that holds the given name
struct ValueNode* parser::find_valuenode(string name){

    for (int i = 0; i < memory.size(); i++)
        if (memory.at(i)->name == name) return memory.at(i);
}

//Get/unget the next token
Token parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

//Throws syntax error
void parser::syntax_error()
{
    cout << "Syntax Error\n";
    exit(1);
}

//Checks the expected type; throws syntax error otherwise
Token parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

struct AssignmentStatement parser::parse_program(){

    // var_section -> id_list -> ID COMMA id_list | ID
    parse_var_section();
    parse_body();
}

void parser::parse_var_section(){
    parse_id_list();
    expect(SEMICOLON);
}

void parser::parse_id_list(){
    Token t = expect(ID);

    // If the variable does not exist, then create it
    if (check_var_value(t.lexeme) == -1){
        ValueNode newValue;
        newValue.name = t.lexeme;
        newValue.value = 0;
    }

    // Peek for the next token, if the id list continues then repeat
    t = peek();

    if (t.token_type == ID) parse_id_list();

}

struct AssignmentStatement parser::parse_body(){
    expect(LBRACE);
    parse_stmt_list();
    expect(RBRACE);
}

struct AssignmentStatement parser::parse_stmt_list(){
    parse_stmt();

    Token t = peek();

    if (t.token_type == ID && t.token_type == PRINT && t.token_type == WHILE && t.token_type == IF && 
        t.token_type == SWITCH && t.token_type == FOR) return parse_stmt_list();

    else expect(RBRACE);
}

struct AssignmentStatement parser::parse_stmt(){
    Token t = peek();

    if (t.token_type == ID) return parse_assign_stmt();
    else if (t.token_type == PRINT) return parse_print_stmt();
    else if (t.token_type == WHILE) return parse_while_stmt();
    else if (t.token_type == IF) return parse_if_stmt();
    else if (t.token_type == SWITCH) return parse_switch_stmt();
    else if (t.token_type == FOR) return parse_for_stmt();
}

struct AssignmentStatement parser::parse_assign_stmt(){

    struct AssignmentStatement *stmt = new AssignmentStatement;

    Token t = expect(ID);

    stmt->left_hand_side = find_valuenode(t.lexeme);



}

struct AssignmentStatement parser::parse_print_stmt(){
    Token t = expect(PRINT);

}

struct AssignmentStatement parser::parse_while_stmt(){
    Token t = expect(WHILE);
}

struct AssignmentStatement parser::parse_if_stmt(){
    Token t = expect(IF);
}

struct AssignmentStatement parser::parse_switch_stmt(){
    Token t = expect(SWITCH);
}

struct AssignmentStatement parser::parse_for_stmt(){
    Token t = expect(FOR);
}