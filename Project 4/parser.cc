#include <iostream>
#include <string>
#include <vector>
#include "parser.h"

using namespace std;

//Vector of value nodes
std::vector<struct ValueNode*> memory;

//Start of parser

//Loops through the memory and checks variable's value; returns -1 if it does not exist
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
    
    //cout << "Could not find ValueNode: " << name << endl;

    // if (is_integer(name)){
    //     cout << "Generating ValueNode: " << name << endl;
    //     ValueNode newValue;
    //     newValue.name = name;
    //     newValue.value = 0;
    // }
}

bool parser::is_integer(string name){
    return name.find_first_not_of("0123456789") == string::npos;
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

struct AssignmentStatement *parser::parse_program(){

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
        ValueNode *newValue;
        newValue->name = t.lexeme;
        newValue->value = 0;
        memory.push_back(newValue);
    }

    // Peek for the next token, if the id list continues then repeat
    t = peek();

    if (t.token_type == ID) parse_id_list();

}

struct StatementNode *parser::parse_body(){
    expect(LBRACE);
    parse_stmt_list();
    expect(RBRACE);
}

struct StatementNode *parser::parse_stmt_list(){
    parse_stmt();

    Token t = peek();

    if (t.token_type == ID && t.token_type == PRINT && t.token_type == WHILE && t.token_type == IF && 
        t.token_type == SWITCH && t.token_type == FOR) return parse_stmt_list();

    else expect(RBRACE);
}

struct StatementNode *parser::parse_stmt(){
    Token t = peek();

    struct StatementNode *stmt = new StatementNode;
    

    if (t.token_type == ID){
        stmt->type = ASSIGN_STMT;
        stmt->assign_stmt = parse_assign_stmt();
    }
    else if (t.token_type == PRINT){
        stmt->type = PRINT_STMT;
        stmt->print_stmt = parse_print_stmt();
    }
    else if (t.token_type == WHILE){
        stmt->type = GOTO_STMT;
        stmt->if_stmt = parse_while_stmt(stmt);
    }
    else if (t.token_type == IF){
        stmt->type = IF_STMT;
        stmt = parse_if_stmt(stmt);
    }
    else if (t.token_type == SWITCH){
        stmt->type = IF_STMT;
        stmt = parse_switch_stmt(stmt);
    }
    else if (t.token_type == FOR){
        stmt->type = IF_STMT;
        stmt = parse_for_stmt(stmt);
    }
}

struct AssignmentStatement *parser::parse_assign_stmt(){

    struct AssignmentStatement *stmt = new AssignmentStatement;

    Token t = expect(ID);

    stmt->left_hand_side = find_valuenode(t.lexeme);

    expect(EQUAL);

    // ID
    t = lexer.GetToken;

    // SEMICOLON or op
    Token t2 = peek();

    t = lexer.UngetToken;

    // if t2 is SEMICOLON:
    // assign_stmt -> ID EQUAL primary SEMICOLON
    if (t2.token_type == SEMICOLON){
        stmt->operand1 = find_valuenode(parse_primary().lexeme);    //parse_primary() consumes primary
        expect(SEMICOLON);
        return stmt;
    }

    // if t2 is op:
    // assign_stmt -> ID EQUAL expr SEMICOLON
    else {
        parse_expr(stmt);
        expect(SEMICOLON);
        return stmt;
    }

}

Token parser::parse_primary(){
    Token t = lexer.GetToken();
    if (t.token_type == ID){
        //If the toke type is a variable, then check to see if it exists yet. If it does not, then return syntax error.
        if (check_var_value(t.lexeme) == -1){
            syntax_error();
        } else return t;

    } else if (t.token_type == NUM){
        // If the token type is a number, then check to see if the number exists yet. If it does not, then create it.
        if (check_var_value(t.lexeme) == -1){
            ValueNode *newValue;
            newValue->name = t.lexeme;
            newValue->value = 0;
            memory.push_back(newValue);
        } else return t;
    }
}

struct AssignmentStatement *parser::parse_expr(struct AssignmentStatement *stmt){

    //Set operand1 to the first primary; parse_primary() does checking to see if the primary is an ID or a NUM.
    stmt->operand1 = find_valuenode(parse_primary().lexeme);

    switch (parse_op()){
        case 10: stmt->op = OPERATOR_PLUS;
        case 11: stmt->op = OPERATOR_MINUS;
        case 12: stmt->op = OPERATOR_DIV;
        case 13: stmt->op = OPERATOR_MULT;
        default: OPERATOR_NONE;
    }

    //Set operand2 to the first primary.
    stmt->operand2 = find_valuenode(parse_primary().lexeme);
    return stmt;
}

int parser::parse_op(){
    Token t = lexer.GetToken();
    return t.token_type;
}

struct PrintStatement *parser::parse_print_stmt(){
    expect(PRINT);
    struct PrintStatement *print = new PrintStatement;
    Token t = expect(ID);
    print->id = find_valuenode(t.lexeme);
    expect(SEMICOLON);
}

struct IfStatement *parser::parse_while_stmt(struct StatementNode *stmt){
    // Consume WHILE
    expect(WHILE);

    // Create new IfStatement for while loop
    struct IfStatement *whileStmt = new IfStatement;

    // Parse the condition
    parse_condition(whileStmt);
    
    // While Statement's true_branch = whatever the body afterward is
    whileStmt->true_branch = parse_body();

    /***************GOTO NODE***************/

    // Create a new StatementNode to hold a GotoStatement
    struct StatementNode *GotoNode = new StatementNode;

    // Set the type to tell the compiler to run Goto
    GotoNode->type = GOTO_STMT;
    // Set the Goto node's target to the beginning of the loop
    GotoNode->goto_stmt->target = stmt;
    // Set the next statement in the true branch to be the Goto node so it loops back
    whileStmt->true_branch->next = GotoNode;

    /***************GOTO END***************/

    struct StatementNode *noop = whileStmt->true_branch;

    while (noop->next != NULL){
        noop = noop->next;
    }

    whileStmt->false_branch = noop;

    return whileStmt;
}

int parser::parse_relop(){
    Token t = lexer.GetToken();
    return t.token_type;
}

struct IfStatement *parser::parse_condition(struct IfStatement *whileStmt){

    // Get first operand
    whileStmt->condition_operand1 = find_valuenode(parse_primary().lexeme);

    switch(parse_relop()){
        case 25: whileStmt->condition_op = CONDITION_GREATER;// GREATER
        case 26: whileStmt->condition_op = CONDITION_LESS;// LESS
        case 24: whileStmt->condition_op = CONDITION_NOTEQUAL;// NOTEQUAL
        default: syntax_error();
    }

    whileStmt->condition_operand2 = find_valuenode(parse_primary().lexeme);

    return whileStmt;
}

struct IfStatement *parser::parse_if_stmt(struct StatementNode *stmt){
    expect(IF);
    struct IfStatement *ifStmt = new IfStatement;

    parse_condition(ifStmt);

    ifStmt->true_branch = parse_body();
    //ifStmt->false_branch = stmt->next;

    struct StatementNode *noop = ifStmt->true_branch;

    while (noop->next != NULL){
        noop = noop->next;
    }

    ifStmt->false_branch = noop;

    return ifStmt;
}

struct IfStatement *parser::parse_switch_stmt(struct StatementNode *stmt){
    expect(SWITCH);

    struct IfStatement *switchStmt = new IfStatement;

    switchStmt->condition_operand1 = find_valuenode(parse_primary().lexeme);

    return stmt;
}

struct IfStatement *parser::parse_for_stmt(struct StatementNode *stmt){
    Token t = expect(FOR);

    return stmt;
}