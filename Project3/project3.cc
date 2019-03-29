#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <deque>
#include <vector>
#include "project3.h"

//Global scope pointer that points to current scope
struct scope *currentScope = new scope;

//Global vector containing all variables, their types and values
vector <struct variable> memory;

using namespace std;

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
    cout << "SYNTAX ERROR\n";
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

//program -> scope
struct stmt* project3::parse_program(){
    struct stmt* program = new struct stmt;
    parse_scope();
    return program;
}

//scope -> LBRACE scope_list RBRACE
struct stmt* project3::parse_scope(){

    Token t = lexer.GetToken();

    if (t.token_type == LBRACE){

        //Updating the new scope
        struct scope* newScope = new scope;
        newScope->previousScope = currentScope;
        currentScope = newScope;

        parse_scope_list();

        t = expect(RBRACE);
        
    } else {
        syntax_error();
    }
}

//scope_list -> ...
struct stmt* project3::parse_scope_list(){
    Token t = lexer.GetToken();
    Token t2 = lexer.GetToken();
    Token t3 = peek();
    lexer.UngetToken(t2);
    lexer.UngetToken(t);

    //scope_list -> scope
    if (t.token_type == LBRACE){

        parse_scope();

        //scope_list -> scope scope_list            TOFIX: This is probably wrong
        if (t2.token_type == LBRACE){
            return parse_scope_list();
        }
    }

    if (t.token_type == ID){

        if (t2.token_type == COMMA){

            //scope_list -> var_decl scope_list     TOFIX: This is probably wrong
            if (t3.token_type == LBRACE){
                parse_var_decl();

                return parse_scope();

            //scope_list -> var_decl
            } else {
                return parse_var_decl();
            }
        }

        else if (t2.token_type == EQUAL){

            //scope_list -> stmt scope_list         TOFIX: This is probably wrong
            if (t2.token_type == LBRACE){
                parse_stmt();

                return parse_scope();

            //scope_list -> stmt
            } else {
                return parse_stmt();
            }
        }
    }
    
    if (t.token_type == WHILE){
        //scope_list -> stmt scope_list          TOFIX: This is definitely wrong
        if (t2.token_type == LBRACE){
            parse_stmt();

            return parse_scope();

        //scope_list -> stmt
        } else {
            return parse_stmt();
        }
    }

}

//var decl -> id_list COLON type_name SEMICOLON
struct stmt* project3::parse_var_decl(){
    Token t = lexer.GetToken();

    if (t.token_type == ID){
        parse_id_list();

        t = peek();

        if (t.token_type == REAL || INT || BOOLEAN || STRING){
            parse_type_name();

            t = lexer.GetToken();

            if (t.token_type != SEMICOLON){
                syntax_error();
            }

        } else {
            syntax_error();
        }

    } else {
        syntax_error();
    }
}

//id_list -> ...
struct stmt* project3::parse_id_list(){
    Token t = lexer.GetToken();

    //id_list -> ID
    if (t.token_type == ID){

        struct stmt* st = new stmt;

        //Variable has already been declared in the current scope

        //Creating a new scope to be the scope that we check in the loop
        struct scope* newScope = new scope;
        newScope = currentScope;

        bool alreadyDeclared = false;

        //Checking all scopes to see if the variable has been declared yet
        do{
            if (currentScope->variables.find(t.lexeme) != currentScope->variables.end()){
                //TODO: Print error here
                alreadyDeclared = true;     //Not explicitly necessary since seeing a variable already declared means we throw an error, but...
            }

            //Move down the scope list to check the previous scope
            else if (newScope->previousScope != NULL){
                newScope = newScope->previousScope;
            }

        } while (newScope->previousScope != NULL);

        //First time seeing the variable, so add it to the current scope's variable list
        if (!alreadyDeclared) {    
            struct variable *x = new variable;

            memory.push_back(x);
            currentScope->variables[t.lexeme] = memory.length-1;
            //st->op1 = currentScope->variables[t.lexeme];
        }

        //id_list -> ID COMMA id_list
        Token t2 = lexer.GetToken();
        if (t2.token_type == COMMA){
            return parse_id_list();

        } else {
            lexer.UngetToken(t2);
            return st;
        }
    }
}

//type_name -> ...
struct stmt* project3::parse_type_name(){

    Token t = lexer.GetToken();

    if (t.token_type == REAL){
        //Set variable type to 'REAL'
        

        //st -> stmt_type = "REAL";

    } else if (t.token_type == INT){
        //st -> stmt_type == "INT";

    } else if (t.token_type == BOOLEAN){
        //t -> stmt_type == "BOOLEAN";

    } else if (t.token_type == STRING){
        //st -> stmt_type == "STRING";

    } else {
        //print error
    }
}

//stmt_list -> ...
struct stmt* project3::parse_stmt_list(){
    Token t = peek();

    if (t.token_type == ID || WHILE){

        t = peek();

        //stmt_list -> stmt stmt_list
        if (t.token_type == ID || WHILE){
            return parse_stmt_list();

        //stmt_list -> stmt
        } else {
            return parse_stmt();
        }
    }
}

//stmt -> ...
struct stmt* project3::parse_stmt(){
    Token t = peek();

    //stmt -> assign_stmt
    if (t.token_type == ID){
        return parse_assign_stmt();

    //stmt -> while_stmt
    } else if (t.token_type == WHILE){
        return parse_while_stmt();

    } else {
        syntax_error();
    }

}

//assign_stmt -> ID EQUAL expr SEMICOLON
struct stmt* project3::parse_assign_stmt(){
    struct stmt* st = new stmt;

    Token t = expect(ID);
    st->LHS = currentScope->variables[t.lexeme];

    t = expect(EQUAL);
    
    parse_expr();
}

//while_stmt -> ...
struct stmt* project3::parse_while_stmt(){

    Token t = expect(WHILE);

    parse_condition();

    t = lexer.GetToken();

    //while_stmt -> WHILE condition LBRACE stmt_list RBRACE
    if (t.token_type == LBRACE){
        parse_stmt_list();
        t = expect(RBRACE);
    }

    //while_stmt -> WHILE condition stmt
    else if (t.token_type == ID || WHILE){
        lexer.UngetToken(t);
        parse_stmt();
    }
}

//expr -> ...
struct stmt* project3::parse_expr(){
    Token t = peek();

    //expr -> arithmetic_operator expr expr
    if (t.token_type == PLUS || MINUS || MULT || DIV){
        parse_arithmetic_operator();
        parse_expr();
        parse_expr();
    }

    //expr -> binary_boolean_operator expr expr
    else if (t.token_type == AND || OR || XOR){
        parse_binary_boolean_operator();
        parse_expr();
        parse_expr();
    }

    //expr -> relational_operator expr expr
    else if (t.token_type == GREATER || GTEQ || LESS || NOTEQUAL || LTEQ){
        parse_relational_operator();
        parse_expr();
        parse_expr();
    }

    //expr -> NOT expr
    else if (t.token_type == NOT){
        parse_expr();
    }

    //expr -> primary
    else if (t.token_type == ID || NUM || REALNUM || STRING_CONSTANT || TRUE || FALSE){
        parse_primary();
    }
}

//arithmetic_operator -> ...
struct stmt* project3::parse_arithmetic_operator(){
    Token t = lexer.GetToken();

    //arithmetic_operator -> PLUS
    if (t.token_type == PLUS){

    }

    //arithmetic_operator -> MINUS
    else if (t.token_type == MINUS){

    }

    //arithmetic_operator -> MULT
    else if (t.token_type == MULT){

    }

    //arithmetic_operator -> DIV
    else if (t.token_type == DIV){

    }

}

//binary_boolean_operator -> ...
struct stmt* project3::parse_binary_boolean_operator(){
    Token t = lexer.GetToken();

    //binary_boolean_operator -> AND
    if (t.token_type == AND){

    }

    //binary_boolean_operator -> OR
    else if (t.token_type == OR){

    }

    //binary_boolean_operator -> XOR
    else if (t.token_type == XOR){

    }
}

//relational_operator -> ...
struct stmt* project3::parse_relational_operator(){
    Token t = lexer.GetToken();

    //relational_operator -> GREATER
    if (t.token_type == GREATER){

    }

    //relational_operator -> GTEQ
    else if (t.token_type == GTEQ){

    }

    //relational_operator -> LESS
    else if (t.token_type == LESS){

    }

    //relational_operator -> NOTEQUAL
    else if (t.token_type == NOTEQUAL){

    }

    //relational_operator -> LTEQ
    else if (t.token_type == LTEQ){

    }
}

//primary -> ...
struct stmt* project3::parse_primary(){
    Token t = lexer.GetToken();

    //primary -> ID
    if (t.token_type == ID){

    }

    //primary -> NUM
    else if (t.token_type == NUM){

    }

    //primary -> REALNUM
    else if (t.token_type == REALNUM){

    }

    //primary -> STRING_CONSTANT
    else if (t.token_type == STRING_CONSTANT){

    }

    //primary -> bool_constant
    else if (t.token_type == TRUE || FALSE){
        lexer.UngetToken(t);
        parse_bool_const();
    }
}

//bool_constant -> ...
struct stmt* project3::parse_bool_const(){
    Token t = lexer.GetToken();

    //bool_constant -> TRUE
    if (t.token_type == TRUE){

    }

    //bool_constant -> FALSE
    else if (t.token_type == FALSE){

    }
}

//condition -> LPAREN expr RPAREN
struct stmt* project3::parse_condition(){
    Token t = expect(LPAREN);

    parse_expr();

    t = expect(RPAREN);
}

int main(){

    return 0;
}