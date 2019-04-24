#include <iostream>
#include <string>
#include <vector>
#include "parser.h"
#include "compiler.h"

using namespace std;

//Vector of value nodes
std::vector<struct ValueNode *> memory;

//Start of parser

//Loops through the memory and checks variable's value; returns -1 if it does not exist
int parser::check_var_value(string name)
{

    for (std::vector<struct ValueNode *>::iterator it = memory.begin(); it != memory.end(); it++)
    {
        if ((*it)->name == name)
            return (*it)->value;
    }

    return -1;
}

//Loops through memory to find the ValueNode that holds the given name
struct ValueNode *parser::find_valuenode(string name)
{

    for (int i = 0; i < memory.size(); i++)
        if (memory.at(i)->name == name)
            return memory.at(i);

    //cout << "Could not find ValueNode: " << name << endl;

    // if (is_integer(name)){
    //     cout << "Generating ValueNode: " << name << endl;
    //     ValueNode newValue;
    //     newValue.name = name;
    //     newValue.value = 0;
    // }
}

bool parser::is_integer(string name)
{
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

struct StatementNode *parse_generate_intermediate_representation()
{
    parser parser;
    struct StatementNode *program = parser.parse_program();
    return program;
}

struct StatementNode *parser::parse_program()
{

    struct StatementNode *program = new StatementNode;

    // Initializes all variables
    parse_var_section();

    program = parse_body();

    return program;
}

void parser::parse_var_section()
{
    parse_id_list();
    expect(SEMICOLON);
}

void parser::parse_id_list()
{
    Token t = expect(ID);

    // If the variable does not exist, then create it
    if (check_var_value(t.lexeme) == -1)
    {
        ValueNode *newValue = new ValueNode;
        newValue->name = t.lexeme;
        newValue->value = 0;
        memory.push_back(newValue);
    }

    // Peek for the next token, if the id list continues then repeat
    t = peek();

    if (t.token_type == COMMA)
    {
        lexer.GetToken();
        parse_id_list();
    }
}

struct StatementNode *parser::parse_body()
{
    expect(LBRACE);
    struct StatementNode *stmt = parse_stmt_list();
    expect(RBRACE);

    return stmt;
}

struct StatementNode *parser::parse_stmt_list()
{

    struct StatementNode *stmt = parse_stmt();
    struct StatementNode *end = stmt;
    while (end->next != NULL)
    {
        end = end->next;
    }
    end->next = NULL;

    Token t = peek();

    if (t.token_type == ID || t.token_type == PRINT || t.token_type == WHILE || t.token_type == IF ||
        t.token_type == SWITCH || t.token_type == FOR)
    {

        end->next = parse_stmt_list();
    }

    return stmt;
}

struct StatementNode *parser::parse_stmt()
{
    Token t = peek();

    struct StatementNode *stmt = new StatementNode;
    stmt->next = NULL;

    if (t.token_type == ID)
    {
        stmt->type = ASSIGN_STMT;
        stmt->assign_stmt = parse_assign_stmt();
    }
    else if (t.token_type == PRINT)
    {
        stmt->type = PRINT_STMT;
        stmt->print_stmt = parse_print_stmt();
    }
    else if (t.token_type == WHILE)
    {
        stmt->type = IF_STMT;
        stmt->if_stmt = parse_while_stmt(stmt);
    }
    else if (t.token_type == IF)
    {
        stmt->type = IF_STMT;
        stmt->if_stmt = parse_if_stmt(stmt);
    }
    else if (t.token_type == SWITCH)
    {
        stmt = parse_switch_stmt();
        stmt->type = IF_STMT;
    }
    else if (t.token_type == FOR)
    {
        stmt = parse_for_stmt();
        // stmt->type = IF_STMT;
    }

    return stmt;
}

struct AssignmentStatement *parser::parse_assign_stmt()
{

    struct AssignmentStatement *stmt = new AssignmentStatement;

    Token t = expect(ID);

    stmt->left_hand_side = find_valuenode(t.lexeme);

    expect(EQUAL);

    // ID or NUM
    t = lexer.GetToken();

    // SEMICOLON or op
    Token t2 = peek();

    lexer.UngetToken(t);

    // if t2 is SEMICOLON:
    // assign_stmt -> ID EQUAL primary SEMICOLON
    if (t2.token_type == SEMICOLON)
    {
        stmt->operand1 = find_valuenode(parse_primary().lexeme); //parse_primary() consumes primary
        stmt->op = OPERATOR_NONE;
        stmt->operand2 = NULL;
        expect(SEMICOLON);
        return stmt;
    }

    // if t2 is op:
    // assign_stmt -> ID EQUAL expr SEMICOLON
    else
    {
        parse_expr(stmt);
        expect(SEMICOLON);
        return stmt;
    }
}

Token parser::parse_primary()
{
    Token t = lexer.GetToken();
    if (t.token_type == ID)
    {
        //If the toke type is a variable, then check to see if it exists yet. If it does not, then return syntax error.
        if (check_var_value(t.lexeme) == -1)
        {
            syntax_error();
        }
        else
            return t;
    }
    else if (t.token_type == NUM)
    {
        // If the token type is a number, then check to see if the number exists yet. If it does not, then create it.
        if (check_var_value(t.lexeme) == -1)
        {
            ValueNode *newValue = new ValueNode;
            newValue->name = t.lexeme;
            newValue->value = atoi(t.lexeme.c_str());
            memory.push_back(newValue);
        }
        else
            return t;
    }
}

struct AssignmentStatement *parser::parse_expr(struct AssignmentStatement *stmt)
{

    //Set operand1 to the first primary; parse_primary() does checking to see if the primary is an ID or a NUM.
    stmt->operand1 = find_valuenode(parse_primary().lexeme);

    switch (parse_op())
    {
    case 10:
        stmt->op = OPERATOR_PLUS;
        break;
    case 11:
        stmt->op = OPERATOR_MINUS;
        break;
    case 12:
        stmt->op = OPERATOR_DIV;
        break;
    case 13:
        stmt->op = OPERATOR_MULT;
        break;
    default:
        stmt->op = OPERATOR_NONE;
    }

    //Set operand2 to the first primary.
    stmt->operand2 = find_valuenode(parse_primary().lexeme);
    return stmt;
}

int parser::parse_op()
{
    Token t = lexer.GetToken();
    return t.token_type;
}

struct PrintStatement *parser::parse_print_stmt()
{
    expect(PRINT);
    struct PrintStatement *print = new PrintStatement;
    Token t = expect(ID);
    print->id = find_valuenode(t.lexeme);
    expect(SEMICOLON);

    return print;
}

struct IfStatement *parser::parse_while_stmt(struct StatementNode *stmt)
{
    // Consume WHILE
    expect(WHILE);

    // Create new IfStatement for while loop
    struct IfStatement *whileStmt = parse_condition();

    // While Statement's true_branch = whatever the body afterward is
    whileStmt->true_branch = parse_body();

    /***************GOTO NODE***************/

    // Create a new StatementNode to hold a GotoStatement
    struct StatementNode *GotoNode = new StatementNode;

    struct GotoStatement *GotoStmt = new GotoStatement;

    // Set the type to tell the compiler to run Goto
    GotoNode->type = GOTO_STMT;
    GotoNode->goto_stmt = GotoStmt;

    // Set the Goto node's target to the beginning of the loop
    GotoNode->goto_stmt->target = stmt;

    // Set the next statement in the true branch to be the Goto node so it loops back

    // a = a + 1; <- truebranch
    // b = b + 1;
    // b = b + 1;
    // b = b + 1; <- end
    // goto

    struct StatementNode *end = whileStmt->true_branch;

    while (end->next != NULL)
    {
        end = end->next;
    }

    end->next = GotoNode;

    /***************GOTO END***************/

    struct StatementNode *noop = new StatementNode;
    noop->type = NOOP_STMT;
    noop->next = NULL;

    whileStmt->false_branch = noop;
    stmt->next = noop;

    GotoNode->next = noop;

    return whileStmt;
}

int parser::parse_relop()
{
    Token t = lexer.GetToken();
    return t.token_type;
}

struct IfStatement *parser::parse_condition()
{

    struct IfStatement *stmt = new IfStatement;

    // Get first operand
    stmt->condition_operand1 = find_valuenode(parse_primary().lexeme);

    switch (parse_relop())
    {
    case 25:
        stmt->condition_op = CONDITION_GREATER;
        break; // GREATER
    case 26:
        stmt->condition_op = CONDITION_LESS;
        break; // LESS
    case 24:
        stmt->condition_op = CONDITION_NOTEQUAL;
        break; // NOTEQUAL
    default:
        syntax_error();
    }

    stmt->condition_operand2 = find_valuenode(parse_primary().lexeme);

    return stmt;
}

struct IfStatement *parser::parse_if_stmt(struct StatementNode *stmt)
{
    expect(IF);
    struct IfStatement *ifStmt = parse_condition();

    ifStmt->true_branch = parse_body();
    //ifStmt->false_branch = stmt->next;

    struct StatementNode *noop = new StatementNode;
    noop->type = NOOP_STMT;
    noop->next = NULL;

    struct StatementNode *end = ifStmt->true_branch;

    while (end->next != NULL)
    {
        end = end->next;
    }

    end->next = noop;

    ifStmt->false_branch = noop;
    stmt->next = noop;

    return ifStmt;
}

struct StatementNode *parser::parse_switch_stmt()
{
    expect(SWITCH);

    struct IfStatement *switchStmt = new IfStatement;

    switchStmt->condition_operand1 = find_valuenode(parse_primary().lexeme);

    switchStmt->condition_op = CONDITION_NOTEQUAL;

    expect(LBRACE);

    struct StatementNode *noop = new StatementNode;
    noop->type = NOOP_STMT;
    noop->next = NULL;

    // Make the above line a noo[p] 
    struct StatementNode *stmt = parse_case_list(switchStmt, noop);

    struct StatementNode *endStmt = stmt;

    //TODO: Find the last IF statement
    struct StatementNode *lastIf = stmt;

    while (endStmt->next != NULL){
        endStmt = endStmt->next;

        if (endStmt->type == IF_STMT) {
            lastIf = endStmt;
        }
    }
    
    Token t = peek();
    if (t.token_type == DEFAULT)
    {
        endStmt->next = parse_default_case();
        lastIf->if_stmt->true_branch->next = endStmt->next; //noop -> next == parse_default_case()
    } else {
        endStmt->next = noop;
        lastIf->if_stmt->true_branch->next = endStmt->next;
    }

    
    expect(RBRACE);

    return stmt;
}

struct StatementNode *parser::parse_case_list(struct IfStatement *switchStmt, struct StatementNode *endStmt)
{

    struct StatementNode *stmt = new StatementNode;

    stmt->type = IF_STMT;
    stmt->if_stmt = parse_case(switchStmt, endStmt);

    Token t = peek();
    if (t.token_type == CASE){
        stmt->next = parse_case_list(switchStmt, endStmt);

        struct StatementNode *endStmt = stmt->if_stmt->true_branch;
        endStmt->next = stmt->next;
    } 

    return stmt;
}

struct IfStatement *parser::parse_case(struct IfStatement *switchStmt, struct StatementNode *endStmt)
{

    expect(CASE);

    Token t = parse_primary();

    expect(COLON);

    struct IfStatement *caseStmt = new IfStatement;

    // Copy the operand1 and conditional_op to the new IfStatement that we're using
    caseStmt->condition_operand1 = switchStmt->condition_operand1;
    caseStmt->condition_op = switchStmt->condition_op;
    caseStmt->condition_operand2 = find_valuenode(t.lexeme);

    caseStmt->false_branch = parse_body();

    struct StatementNode *noop = new StatementNode;
    noop->type = NOOP_STMT;
    noop->next = NULL;

    caseStmt->true_branch = noop;

    // Create a new StatementNode to hold a GotoStatement
    struct StatementNode *GotoNode = new StatementNode;
    struct GotoStatement *GotoStmt = new GotoStatement;

    GotoNode->type = GOTO_STMT;
    GotoNode->goto_stmt = GotoStmt;
    GotoNode->goto_stmt->target = endStmt;
    GotoNode->next = noop;

    // Constructing end
    struct StatementNode *end = caseStmt->false_branch;
    while (end->next != NULL)
    {
        end = end->next;
    }
    end->next = GotoNode;

    // while (noop->next != NULL)
    // {
    //     noop = noop->next;
    // }

    // caseStmt->true_branch = noop;

    // stmt->next = noop;

    return caseStmt;
}

struct StatementNode *parser::parse_default_case()
{
    expect(DEFAULT);
    expect(COLON);
    struct StatementNode *defaultCase = parse_body();
    return defaultCase;
}

struct StatementNode *parser::parse_for_stmt()
{
    expect(FOR);
    expect(LPAREN);

    struct StatementNode *assign1 = new StatementNode;
    assign1->assign_stmt = parse_assign_stmt();
    assign1->type = ASSIGN_STMT;

    // This is going to be used to hold the whileStmt
    struct StatementNode *condition = new StatementNode;
    struct IfStatement *whileStmt = parse_condition();
    condition->if_stmt = whileStmt;
    condition->type = IF_STMT;

    // Chain the next instruction
    assign1->next = condition;

    expect(SEMICOLON);

    // This assign will happen after every iteration of the while statement
    struct StatementNode *assign2 = new StatementNode;
    assign2->type = ASSIGN_STMT;
    assign2->assign_stmt = parse_assign_stmt();
    expect(RPAREN);

    // Finished setting up the structure, now to parse the body
    // The body is basically the same structure as the while loop, but with assign2 being the next of the whileStmt

    // While Statement's true_branch = whatever the body afterward is
    whileStmt->true_branch = parse_body();

    // Create a new StatementNode to hold a GotoStatement
    struct StatementNode *GotoNode = new StatementNode;
    struct GotoStatement *GotoStmt = new GotoStatement;

    GotoNode->type = GOTO_STMT;
    GotoNode->goto_stmt = GotoStmt;
    GotoNode->goto_stmt->target = condition;

    struct StatementNode *end = whileStmt->true_branch;

    while (end->next != NULL)
    {
        end = end->next;
    }
    // Do the assignment
    end->next = assign2;
    // After doing the assignment, go to the beginning of the loop
    assign2->next = GotoNode;

    struct StatementNode *noop = new StatementNode;
    noop->type = NOOP_STMT;
    noop->next = NULL;

    whileStmt->false_branch = noop;

    condition->if_stmt = whileStmt;

    GotoNode->next = noop;

    condition->next = noop;

    return assign1;
}