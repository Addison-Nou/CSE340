#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <deque>
#include <vector>
#include <set>

#include "project3.h"

using namespace std;

//Global scope pointer that points to current scope
struct scope *currentScope = new scope;
std::set<std::string> unusedVariables;

//Global vector containing all variables, their types and values
std::vector<struct variable> memory;

// void print_error(string error_type, struct stmt){
//     switch(case_type){
//         case 1:
//             cout << "ERROR CODE " << code-> << symbol_name
//             break;
//         default:

//     }
//     cout << "TYPE " << endl;
// }

//Start of parser
Token project3::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

//Throws syntax error
void project3::syntax_error()
{
    cout << "Syntax Error\n";
    exit(1);
}

//Checks the expected type; throws syntax error otherwise
Token project3::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// Loops through scopes and attempts to find variable
int project3::getVariableLocation(std::string variableName)
{
    //Checking all scopes to see if the variable has been declared yet
    struct scope *scopeCursor = currentScope;

    do
    {
        // cout << "Variable Name: " << variableName << endl;
        if (scopeCursor == NULL || scopeCursor->variables.size() == 0)
        {
            break;
        }
        if (scopeCursor->variables.find(variableName) != scopeCursor->variables.end())
        {
            //TODO: Print error here
            //Variable has already bee n declared in the current scope
            if (unusedVariables.find(variableName) != unusedVariables.end())
            {
                unusedVariables.erase(variableName);
            }
            return scopeCursor->variables[variableName];
        }

        //Move down the scope list to check the previous scope
        else
        {
            scopeCursor = scopeCursor->previousScope;
        }

    } while (scopeCursor != NULL);

    return -1;
}

//program -> scope
struct stmt *project3::parse_program()
{
    struct stmt *program = new struct stmt;
    parse_scope();
    return program;
}

//scope -> LBRACE scope_list RBRACE
struct stmt *project3::parse_scope()
{

    Token t = expect(LBRACE);

    //Updating the new scope
    struct scope *newScope = new scope;
    newScope->previousScope = currentScope;
    currentScope = newScope;

    parse_scope_list();

    expect(RBRACE);
}

//scope_list -> ...
struct stmt *project3::parse_scope_list()
{
    Token t = lexer.GetToken();
    Token t2 = peek();
    lexer.UngetToken(t);

    //scope_list -> scope
    if (t.token_type == LBRACE)
    {
        parse_scope();
    }
    else if (t.token_type == ID)
    {
        // ID LIST
        if (t2.token_type == EQUAL)
        {
            parse_stmt();
        }
        else
        {
            parse_var_decl();
        }
    }
    else if (t.token_type == WHILE)
    {
        parse_while_stmt();
    }
    else
    {
        syntax_error();
    }

    // Does another scope list follow? This is the check.
    t = peek();
    if (t.token_type == LBRACE || t.token_type == ID || t.token_type == WHILE)
    {
        parse_scope_list();
    }
}

//var decl -> id_list COLON type_name SEMICOLON
struct stmt *project3::parse_var_decl()
{
    vector<std::string> ids = parse_id_list();

    // Make sure no duplicate variables are declared
    for (auto &id : ids)
    {
        if (getVariableLocation(id) != -1)
        {
            cout << "ERROR CODE 1.1 " << id << endl;
            exit(1);
        }
    }

    expect(COLON);

    TokenType type = parse_type_name();

    // Create variables in current scope
    for (auto &id : ids)
    {
        struct variable x;
        x.type = type;
        memory.push_back(x);
        unusedVariables.insert(id);
        currentScope->variables[id] = memory.size() - 1;
    }

    expect(SEMICOLON);
}

//id_list -> ...
std::vector<std::string> project3::parse_id_list()
{
    vector<std::string> ids;

    while (peek().token_type == ID)
    {
        Token t = expect(ID);
        ids.push_back(t.lexeme);

        Token t2 = lexer.GetToken();
        if (t2.token_type != COMMA)
            lexer.UngetToken(t2);
    }

    return ids;
}

//type_name -> ...
TokenType project3::parse_type_name()
{

    Token t = lexer.GetToken();

    if (t.token_type == REAL || t.token_type == INT || t.token_type == BOOLEAN || t.token_type == STRING)
    {
        return t.token_type;
    }
    else
    {
        syntax_error();
    }
}

//stmt_list -> ...
struct stmt *project3::parse_stmt_list()
{
    struct stmt *firstStmt = parse_stmt();
    Token t = peek();

    if (t.token_type == ID || t.token_type == WHILE)
    {
        firstStmt->next = parse_stmt_list();
    }

    return firstStmt;
}

//stmt -> ...
struct stmt *project3::parse_stmt()
{
    Token t = peek();

    //stmt -> assign_stmt
    if (t.token_type == ID)
    {
        return parse_assign_stmt();

        //stmt -> while_stmt
    }
    else if (t.token_type == WHILE)
    {
        return parse_while_stmt();
    }
    else
    {
        syntax_error();
    }
}

//assign_stmt -> ID EQUAL expr SEMICOLON
struct stmt *project3::parse_assign_stmt()
{
    struct stmt *assignStmt = new stmt;
    assignStmt->stmt_type = "ASSIGN";
    assignStmt->next = NULL;

    Token t = expect(ID);

    // Check if variable exists
    int variableLocation = getVariableLocation(t.lexeme);
    if (variableLocation == -1)
    {
        cout << "ERROR: Variable doesn't exist (parse assign stmt) REPLACE ME... " << t.lexeme << endl;
        syntax_error();
    }

    assignStmt->LHS = variableLocation;

    t = expect(EQUAL);

    struct stmt *firstStmt = parse_expr();
    struct stmt *finalStmt = firstStmt;
    while (finalStmt->next != NULL)
    {
        finalStmt = finalStmt->next;
    }

    assignStmt->op1 = finalStmt->LHS; // LHS stores value to all calculations
    finalStmt->next = assignStmt;

    expect(SEMICOLON);

    // Check to make sure left hand side and right hand side are of the same type
    if (memory[variableLocation].type != REAL && memory[variableLocation].type != memory[assignStmt->op1].type)
    {
        cout << "TYPE MISMATCH " << lexer.get_line_no() << " C1" << endl;
        exit(1);
    }
    if (memory[variableLocation].type == REAL && (memory[assignStmt->op1].type != REAL && memory[assignStmt->op1].type != INT))
    {
        cout << "TYPE MISMATCH " << lexer.get_line_no() << " C2" << endl;
        exit(1);
    }

    memory[assignStmt->LHS].initialized = true;

    return firstStmt;
}

//while_stmt -> ...
struct stmt *project3::parse_while_stmt()
{
    struct stmt *whileStmt = new stmt;
    whileStmt->stmt_type = "WHILE";
    whileStmt->next = NULL;

    Token t = expect(WHILE);

    struct stmt *condition = parse_condition();

    t = lexer.GetToken();

    //while_stmt -> WHILE condition LBRACE stmt_list RBRACE
    if (t.token_type == LBRACE)
    {
        struct stmt *firstStmt = parse_stmt_list();
        whileStmt->body = firstStmt;
        expect(RBRACE);
    }

    //while_stmt -> WHILE condition stmt
    else if (t.token_type == ID || WHILE)
    {
        lexer.UngetToken(t);
        struct stmt *firstStmt = parse_stmt();
        whileStmt->body = firstStmt;
    }

    return whileStmt;
}

//expr -> ...
struct stmt *project3::parse_expr()
{
    struct stmt *temp = new stmt;
    temp->stmt_type = "ASSIGN";
    temp->next = NULL;

    // New temp variable
    struct variable var;
    memory.push_back(var);
    temp->LHS = memory.size() - 1;

    struct stmt *returnMe = NULL;
    Token t = peek();

    //       (op1)     (op2)
    // X = + (result1) (result2)
    // X = + (+ 1 2) (+ 3 4)
    // parse assign stmt  X = temporary_value 3 (ASSIGN STMT)
    // parse expr         temporary_value_3 = temporary_value_1 + temporary_value_2 (FINAL RESULT STMT)
    // parse expr         temporary_value_1 = 1 + 2
    // prase expr         temporary_value_2 = 3 + 4
    //------------------------ REVERSE ORDER
    // parse expr         temporary_value_1 = 1 + 2 (FIRST STMT)
    // prase expr         temporary_value_2 = 3 + 4 (SECOND STMT)
    // parse expr         temporary_value_3 = temporary_value_1 + temporary_value_2 (FINAL RESULT STMT)
    // parse assign stmt  X = temporary_value 3 (ASSIGN STMT)

    //expr -> arithmetic_operator expr expr
    if (t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT || t.token_type == DIV)
    {
        temp->operator_symbol = parse_arithmetic_operator();
        struct stmt *result1 = parse_expr();
        struct stmt *result2 = parse_expr();
        temp->op1 = result1->LHS;
        temp->op2 = result2->LHS;

        // Set up ordering for execution of statements
        result1->next = result2;
        result2->next = temp;
        returnMe = result1;
    }

    //expr -> binary_boolean_operator expr expr
    else if (t.token_type == AND || t.token_type == OR || t.token_type == XOR)
    {
        temp->operator_symbol = parse_binary_boolean_operator();
        struct stmt *result1 = parse_expr();
        struct stmt *result2 = parse_expr();
        temp->op1 = result1->LHS;
        temp->op2 = result2->LHS;

        // Set up ordering for execution of statements
        result1->next = result2;
        result2->next = temp;
        returnMe = result1;
    }

    //expr -> relational_operator expr expr
    else if (t.token_type == GREATER || t.token_type == GTEQ || t.token_type == LESS || t.token_type == NOTEQUAL || t.token_type == LTEQ)
    {
        temp->operator_symbol = parse_relational_operator();
        struct stmt *result1 = parse_expr();
        struct stmt *result2 = parse_expr();
        temp->op1 = result1->LHS;
        temp->op2 = result2->LHS;

        // Set up ordering for execution of statements
        result1->next = result2;
        result2->next = temp;
        returnMe = result1;
    }

    //expr -> NOT expr
    else if (t.token_type == NOT)
    {
        temp->operator_symbol = NOT;
        struct stmt *result1 = parse_expr();
        temp->op1 = result1->LHS;
        returnMe = result1;
    }

    //expr -> primary
    else if (t.token_type == ID || t.token_type == NUM || t.token_type == REALNUM || t.token_type == STRING_CONSTANT || t.token_type == TRUE || t.token_type == FALSE)
    {
        int variableLocation = parse_primary();
        temp->op1 = variableLocation;
        returnMe = temp;
    }

    memory[temp->LHS].type = memory[temp->op1].type;

    return returnMe;
}

//arithmetic_operator -> ...
TokenType project3::parse_arithmetic_operator()
{
    Token t = lexer.GetToken();

    //arithmetic_operator -> PLUS
    if (t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT || t.token_type == DIV)
    {
        return t.token_type;
    }
    else
    {
        syntax_error();
    }
}

//binary_boolean_operator -> ...
TokenType project3::parse_binary_boolean_operator()
{
    Token t = lexer.GetToken();

    //binary_boolean_operator -> AND
    if (t.token_type == AND || t.token_type == OR || t.token_type == XOR)
    {
        return t.token_type;
    }
    else
    {
        syntax_error();
    }
}

//relational_operator -> ...
TokenType project3::parse_relational_operator()
{
    Token t = lexer.GetToken();

    //relational_operator -> GREATER
    if (t.token_type == GREATER || t.token_type == GTEQ || t.token_type == LESS || t.token_type == LTEQ || t.token_type == NOTEQUAL)
    {
        return t.token_type;
    }
    else
    {
        syntax_error();
    }
}

//primary -> ...
int project3::parse_primary()
{
    Token t = lexer.GetToken();

    //primary -> ID
    if (t.token_type == ID)
    {
        int location = getVariableLocation(t.lexeme);
        if (location == -1)
        {
            cout << "ERROR: VARIABLE DOESN'T EXIST. (parse primary) " << t.lexeme << endl;
            syntax_error();
        }
        else if (memory[location].initialized == false)
        {
            cout << "UNINITIALIZED " << t.lexeme << " " << lexer.get_line_no() << endl;
            return location;
        }
        else
        {
            return location;
        }
    }

    //primary -> NUM
    else if (t.token_type == NUM)
    {
        struct variable var;
        var.type = INT;
        var.value = atoi(t.lexeme.c_str());
        memory.push_back(var);
        return memory.size() - 1;
    }
    else if (t.token_type == REALNUM)
    {
        struct variable var;
        var.type = REAL;
        var.value = stof(t.lexeme.c_str());
        memory.push_back(var);
        return memory.size() - 1;
    }

    //primary -> STRING_CONSTANT
    else if (t.token_type == STRING_CONSTANT)
    {
        struct variable var;
        var.type = STRING;
        var.stringValue = t.lexeme;
        memory.push_back(var);
        return memory.size() - 1;
    }

    //primary -> bool_constant
    else if (t.token_type == TRUE || FALSE)
    {
        lexer.UngetToken(t);
        return parse_bool_const();
    }
    else
    {
        syntax_error();
    }
}

//bool_constant -> ...
int project3::parse_bool_const()
{
    Token t = lexer.GetToken();

    //bool_constant -> TRUE
    if (t.token_type == TRUE)
    {
        struct variable var;
        var.type = BOOLEAN;
        var.value = 1;
        memory.push_back(var);
        return memory.size() - 1;
    }

    //bool_constant -> FALSE
    else if (t.token_type == FALSE)
    {
        struct variable var;
        var.type = BOOLEAN;
        var.value = 0;
        memory.push_back(var);
        return memory.size() - 1;
    }
    else
    {
        syntax_error();
    }
}

//condition -> LPAREN expr RPAREN
struct stmt *project3::parse_condition()
{
    expect(LPAREN);

    struct stmt *st = parse_expr();

    if (st->operator_symbol != GREATER || st->operator_symbol != GTEQ || st->operator_symbol != LESS || st->operator_symbol != LTEQ || st->operator_symbol != NOTEQUAL)
    {
        cout << "ERROR: Condition without relational operator: " << st->operator_symbol << endl;
        syntax_error();
    }

    expect(RPAREN);

    return st;
}

int main()
{
    project3 proj;
    proj.parse_program();

    if (unusedVariables.size() > 0)
    {

        cout << "ERROR CODE 1.3 ";
        set<std::string>::iterator it;
        for (it = unusedVariables.begin(); it != unusedVariables.end(); ++it)
        {
            cout << *it << " ";
        }
        cout << endl;
    }
    return 0;
}