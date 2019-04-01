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
std::vector<std::string> uninitialized_variables;
std::vector<std::string> declaration_errors;
std::vector<std::string> type_mismatch_errors;
std::vector<std::string> variable_references;

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
int project3::getVariableLocation(std::string variableName, bool lookLocally = false)
{
    string keywords[] = {
        "REAL", "INT", "BOOLEAN", "STRING",
        "WHILE", "TRUE", "FALSE"};

    for (auto &keyword : keywords)
    {
        if (keyword.compare(variableName) == 0)
        {
            syntax_error();
        }
    }

    //Checking all scopes to see if the variable has been declared yet
    struct scope *scopeCursor = currentScope;

    do
    {
        // cout << "Variable Name: " << variableName << endl;
        if (scopeCursor == NULL)
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

            int var_loc = scopeCursor->variables[variableName];
            variable_references.push_back(variableName + " " + to_string(lexer.get_line_no()) + " " + to_string(memory[var_loc].line_no_declared));

            return var_loc;
        }

        //Move down the scope list to check the previous scope
        else
        {
            scopeCursor = scopeCursor->previousScope;
        }

    } while (scopeCursor != NULL && lookLocally == false);

    return -1;
}

//program -> scope
struct stmt *project3::parse_program()
{
    struct stmt *program = new struct stmt;
    parse_scope();

    if (peek().token_type != END_OF_FILE)
        syntax_error();
    return program;
}

//scope -> LBRACE scope_list RBRACE
struct stmt *project3::parse_scope()
{

    expect(LBRACE);

    //Updating the new scope
    struct scope *newScope = new scope;
    newScope->previousScope = currentScope;
    currentScope = newScope;

    parse_scope_list();

    expect(RBRACE);

    currentScope = currentScope->previousScope;
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
        // scope_list -> assign_stmt
        if (t2.token_type == EQUAL)
        {
            parse_stmt();
        } // scope_list - > var_decl
        else if (t2.token_type == COLON)
        {
            parse_var_decl();
        }
        else
        {
            syntax_error();
        }
    }
    else if (t.token_type == WHILE) //scope_list -> while_stmt
    {
        parse_stmt();
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
    vector<id_line_pair> ids = parse_id_list();

    // Make sure no duplicate variables are declared
    for (auto &pair : ids)
    {
        if (getVariableLocation(pair.id, true) != -1)
        {
            declaration_errors.push_back("ERROR CODE 1.1 " + pair.id);
        }
    }

    expect(COLON);

    TokenType type = parse_type_name();

    // Create variables in current scope
    for (auto &pair : ids)
    {
        struct variable x;
        x.type = type;
        x.line_no_declared = pair.line_no;
        memory.push_back(x);
        unusedVariables.insert(pair.id);
        currentScope->variables[pair.id] = memory.size() - 1;
    }

    expect(SEMICOLON);
}

//id_list -> ...
std::vector<id_line_pair> project3::parse_id_list()
{

    vector<id_line_pair> ids;

    do
    {
        Token t = expect(ID);
        id_line_pair new_pair;
        new_pair.id = t.lexeme;
        new_pair.line_no = t.line_no;
        ids.push_back(new_pair);

        Token t2 = lexer.GetToken();
        if (t2.token_type != COMMA)
            lexer.UngetToken(t2);
    } while (peek().token_type == ID);

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
        declaration_errors.push_back("ERROR CODE 1.2 " + t.lexeme);
    }

    assignStmt->LHS = variableLocation;

    expect(EQUAL);

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

        type_mismatch_errors.push_back("TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C1");
    }
    if (memory[variableLocation].type == REAL && (memory[assignStmt->op1].type != REAL && memory[assignStmt->op1].type != INT))
    {
        type_mismatch_errors.push_back("TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C2");
    }

    // Keep track of initialized variabled in current scope
    currentScope->initialized_variables.push_back(assignStmt->LHS);
    memory[assignStmt->LHS].initialized = true;

    return firstStmt;
}

//while_stmt -> ...
struct stmt *project3::parse_while_stmt()
{
    struct scope *newScope = new scope;
    newScope->previousScope = currentScope;
    currentScope = newScope;

    Token t = expect(WHILE);

    struct stmt *whileStmt = parse_condition();

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

    // Undo all initializations
    for (int &initialized_var : currentScope->initialized_variables)
    {
        memory[initialized_var].initialized = false;
    }
    currentScope = currentScope->previousScope;

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

        if ((memory[temp->op1].type != REAL && memory[temp->op1].type != INT) ||
            (memory[temp->op2].type != REAL && memory[temp->op2].type != INT))
        {
            // cout << "Memory Location 1: " << temp->op1 << endl;
            // cout << "Memory Location 2: " << temp->op2 << endl;
            // cout << "TYPE1: " << memory[temp->op1].type << endl;
            // cout << "TYPE2: " << memory[temp->op2].type << endl;
            // cout << "TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C3" << endl;
            type_mismatch_errors.push_back("TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C3");
        }

        if (memory[temp->op1].type == REAL || memory[temp->op2].type == REAL || t.token_type == DIV)
        {
            memory[temp->LHS].type = REAL;
        }
        else
        {
            memory[temp->LHS].type = memory[temp->op1].type;
        }

        // Set up ordering for execution of statements
        result1->next = result2;
        result2->next = temp;

        return result1;
    }

    //expr -> binary_boolean_operator expr expr
    else if (t.token_type == AND || t.token_type == OR || t.token_type == XOR)
    {
        temp->operator_symbol = parse_binary_boolean_operator();
        struct stmt *result1 = parse_expr();
        struct stmt *result2 = parse_expr();
        temp->op1 = result1->LHS;
        temp->op2 = result2->LHS;

        if ((memory[temp->op1].type != BOOLEAN) ||
            (memory[temp->op2].type != BOOLEAN))
        {
            type_mismatch_errors.push_back("TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C4");
        }

        // Set up ordering for execution of statements
        result1->next = result2;
        result2->next = temp;
        memory[temp->LHS].type = memory[temp->op1].type;

        return result1;
    }

    //expr -> relational_operator expr expr
    else if (t.token_type == GREATER || t.token_type == GTEQ || t.token_type == LESS || t.token_type == NOTEQUAL || t.token_type == LTEQ)
    {
        temp->operator_symbol = parse_relational_operator();
        struct stmt *result1 = parse_expr();
        struct stmt *result2 = parse_expr();
        temp->op1 = result1->LHS;
        temp->op2 = result2->LHS;

        if ((memory[temp->op1].type == STRING && memory[temp->op2].type != STRING) ||
            (memory[temp->op1].type == BOOLEAN && memory[temp->op2].type != BOOLEAN))
        {
            type_mismatch_errors.push_back("TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C5");
        }

        else if ((memory[temp->op1].type == REAL && (memory[temp->op2].type != REAL && memory[temp->op2].type != INT)) ||
                 (memory[temp->op1].type == INT && (memory[temp->op2].type != REAL && memory[temp->op2].type != INT)))
        {
            type_mismatch_errors.push_back("TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C6");
        }
        else
        {
            memory[temp->LHS].type = BOOLEAN;
        }
        // Set up ordering for execution of statements
        result1->next = result2;
        result2->next = temp;

        return result1;
    }

    //expr -> NOT expr
    else if (t.token_type == NOT)
    {
        temp->operator_symbol = NOT;
        lexer.GetToken();
        struct stmt *result1 = parse_expr();
        temp->op1 = result1->LHS;
        memory[temp->LHS].type = memory[temp->op1].type;

        return result1;
    }

    //expr -> primary
    else if (t.token_type == ID || t.token_type == NUM || t.token_type == REALNUM || t.token_type == STRING_CONSTANT || t.token_type == TRUE || t.token_type == FALSE)
    {
        int variableLocation = parse_primary();
        temp->op1 = variableLocation;

        if (temp->op1 != -1) // -1 from parse_primary. Meaning it doesn't exist.
        {
            memory[temp->LHS].type = memory[temp->op1].type;
        }
        return temp;
    }
    else
    {
        syntax_error();
    }
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
            // cout << "ERROR FINDING LEXEME: " << t.lexeme << endl;
            declaration_errors.push_back("ERROR CODE 1.2 " + t.lexeme);
            return -1;
        }
        else if (memory[location].initialized == false)
        {
            uninitialized_variables.push_back("UNINITIALIZED " + t.lexeme + " " + std::to_string(lexer.get_line_no()));
            return location;
        }
        else
        {
            //cout << "MEMORY AT LOCATION: " << memory[location].type << endl;
            // if (memory[location].type == BOOLEAN){
            //     cout << "MEMORY AT LOCATION: " << memory[location].type << endl;
            //     struct variable var;
            //     var.type = BOOLEAN;
            //     var.value = 1;
            //     memory.push_back(var);
            //     return memory.size() - 1;
            // }
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
    struct stmt *finalStmt = st;
    while (finalStmt->next != NULL)
    {
        finalStmt = finalStmt->next;
    }

    if (finalStmt->operator_symbol != GREATER && finalStmt->operator_symbol != GTEQ && finalStmt->operator_symbol != LESS && finalStmt->operator_symbol != LTEQ && finalStmt->operator_symbol != NOTEQUAL)
    {
        if (memory[finalStmt->LHS].type == BOOLEAN)
        {
            //cout << "No operator found, is it bool?" << endl;
            //cout << "LHS: " << memory[finalStmt->LHS].type << endl;
            expect(RPAREN);
            return st;
        }
        else
            // cout << "ERROR: Condition without relational operator: " << finalStmt->operator_symbol << endl;
            type_mismatch_errors.push_back("TYPE MISMATCH " + to_string(lexer.get_line_no()) + " C7");
    }

    expect(RPAREN);

    return st;
}

int main()
{
    project3 proj;
    proj.parse_program();

    if (declaration_errors.size() > 0)
    {
        cout << declaration_errors[0] << endl;
        exit(1);
    }
    else if (unusedVariables.size() > 0)
    {

        cout << "ERROR CODE 1.3 ";
        set<std::string>::iterator it;
        for (it = unusedVariables.begin(); it != unusedVariables.end(); ++it)
        {
            cout << *it << " ";
            break;
        }
        cout << endl;
        exit(1);
    }
    else if (type_mismatch_errors.size() > 0)
    {
        cout << type_mismatch_errors[0] << endl;
        exit(1);
    }
    else if (uninitialized_variables.size() > 0)
    {
        // Take account of uninitialized variables
        for (std::string &str : uninitialized_variables)
        {
            cout << str << endl;
        }
        exit(1);
    }
    else
    {
        for (string &ref : variable_references)
        {
            cout << ref << endl;
        }
    }

    return 0;
}