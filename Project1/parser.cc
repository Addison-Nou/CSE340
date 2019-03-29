/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"
#include <map>
#include <deque>

using namespace std;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR\n";
    exit(1);
}

// Hashmap to keep track of the location of each variable in memory
std::map<std::string, int> variableToLocation;

// Hashmap to keep track of the procedures
std::map<std::string, struct stmt*> procedures;


//Global memory array to hold data values
int* memory = new int[99];

//Global location value
int next_available = 0;

std::deque<int> inputs;

/*
//Adding node to symbol table
void addToTable(struct stmt* node, string name){
    bool match = false;
    //string name = node->stmt_name;
    //Looking up token name in the symbol table
    for (int i = 0; i < next_available; i++){
        if (symbol_table[i]->stmt_name == name){
            found = true;
        }
    }
    if (!match){    //No matches, add to symbol table
        symbol_table[next_available] = node;
        memory[next_available] = node->op1;
        next_available++;
    }
}*/

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

// Parsing
struct stmt* Parser::parse_input(){
    struct stmt* program = parse_program();
    parse_inputs();
    return program;
}

struct stmt* Parser::parse_program(){
    Token t = peek();
    
    if (t.token_type == MAIN){
        return parse_main();
    }

    else if (t.token_type == PROC){
        parse_proc_decl_section();
        return parse_main();
    }

    else syntax_error();
}

void Parser::parse_proc_decl_section(){
    parse_proc_decl();

    Token t = peek();

    if (t.token_type == PROC){
        parse_proc_decl_section();
    }
}

void Parser::parse_proc_decl(){
    Token t = lexer.GetToken();

    if (t.token_type == PROC){
        string name = parse_procedure_name();
        struct stmt* body = parse_procedure_body();

        // Add procedure to the map
        procedures[name] = body;

        t = lexer.GetToken();
        if (t.token_type != ENDPROC){
            syntax_error();
        }

    } else {
        syntax_error();
    }
}

string Parser::parse_procedure_name(){
    Token t = lexer.GetToken();

    if (t.token_type == ID){
        //Store the token
        // Find the procedure and return it.
        return t.lexeme;
    }

    else if (t.token_type == NUM){
        //Store the token
        return t.lexeme;
    }

    else syntax_error();
    return "";
}

struct stmt* Parser::parse_procedure_body(){
    return parse_statement_list();
}

struct stmt* Parser::parse_statement_list(){

    struct stmt* st = parse_statement();

    Token t1 = lexer.GetToken();
    Token t2 = peek();
    lexer.UngetToken(t1);

    if ((t1.token_type == INPUT) || (t1.token_type == OUTPUT) || (t1.token_type == DO) || (t1.token_type == ID && (t2.token_type == SEMICOLON || t2.token_type == EQUAL)) || (t1.token_type == NUM && t2.token_type == SEMICOLON)){
        struct stmt* next_stmt = parse_statement_list();
        st->next = next_stmt;
    }

    return st;
}

struct stmt* Parser::parse_statement(){
    Token t = peek();

    if (t.token_type == INPUT){
        return parse_input_statement();
    }

    else if (t.token_type == OUTPUT){
        return parse_output_statement();
    }

    else if (t.token_type == ID){
        t = lexer.GetToken();
        Token temp = peek();
        lexer.UngetToken(t);
        
        if (temp.token_type == SEMICOLON){
            return parse_procedure_invocation();
        }

        else if (temp.token_type == EQUAL){
            return parse_assign_statement();
        }

        else syntax_error();
    }

    else if (t.token_type == NUM){
        return parse_procedure_invocation();
    }

    else if (t.token_type == DO){
        return parse_do_statement();
    } else {
        syntax_error();
    }
}

struct stmt* Parser::parse_input_statement(){
    Token t = lexer.GetToken();
    struct stmt* st = new stmt;
    st->stmt_type = INPUT_STMT;

    if (t.token_type == INPUT){
        t = lexer.GetToken();

        if (t.token_type == ID){
            
            // look up variable name (t.lexeme) and see if it's in the table

            // possibility 1: if it's in the table, nothing happens
            if (variableToLocation.find(t.lexeme) != variableToLocation.end()) { //If the variable location exists
                st->op1 = variableToLocation.find(t.lexeme)->second; // Get the location of the variable stored in memory

            } else { // possibility 2: if this is the first time we're seeing it, then allocate memory
                
                variableToLocation[t.lexeme] = next_available;
                next_available++;
                st->op1 = variableToLocation[t.lexeme];   // Address of the variable in memory
            }

            t = lexer.GetToken();
            if (t.token_type != SEMICOLON){
                syntax_error();
            }

            return st;

        } else syntax_error();
    } else {
        syntax_error();
    }
}

struct stmt* Parser::parse_output_statement(){
    Token t = lexer.GetToken();
    struct stmt* st = new stmt;
    st->stmt_type = OUTPUT_STMT;

    if (t.token_type == OUTPUT){
        t = lexer.GetToken();
        if (t.token_type == ID){

            //st->stmt_name = t.lexeme;
            //st->op1 = next_available;

            // possibility 1: if it's in the table, nothing happens
            if (variableToLocation.find(t.lexeme) != variableToLocation.end()) { //If the variable location exists
                st->op1 = variableToLocation[t.lexeme]; // Get the location of the variable stored in memory

            } else { // possibility 2: if this is the first time we're seeing it, then allocate memory

                variableToLocation[t.lexeme] = next_available;
                next_available++;
                st->op1 = variableToLocation[t.lexeme];   // Address of the variable in memory
            }

            t = lexer.GetToken();
            if (t.token_type != SEMICOLON){
                // output_statement -> OUTPUT ID SEMICOLON
                syntax_error();
            }

            return st;

        } else {
            syntax_error();
        }
    } else {
        syntax_error();
    }
}

struct stmt* Parser::parse_procedure_invocation(){    
    string procedure_name = parse_procedure_name();

    struct stmt* st = new stmt;
    st->stmt_type = PROC_INVOCATION_STMT;
    st->proc_code = procedures[procedure_name];

    Token t = lexer.GetToken();
    if (t.token_type != SEMICOLON){
        syntax_error();
    } else {
        return st;
    }
}

struct stmt* Parser::parse_do_statement(){
    struct stmt* st = new stmt;
    st->stmt_type = DO_STMT;

    Token t = lexer.GetToken();
    if (t.token_type == DO){
        t = lexer.GetToken();
        if (t.token_type == ID){
            //cout << "SETTING LOCATION TO " << variableToLocation[t.lexeme] << " FOR LEXEME " << t.lexeme << endl;
            st->op1 = variableToLocation[t.lexeme];
            st->proc_code = parse_procedure_invocation();
            return st;

            // do_statement -> DO ID procedure_invocation
        } else {
            syntax_error();
        }
    } else {
        syntax_error();
    }
}

struct stmt* Parser::parse_assign_statement(){
    Token t = lexer.GetToken();
    struct stmt* st = new stmt;
    st->stmt_type = ASSIGN_STMT;
    
    if (t.token_type == ID){
        //Store input (ID)
        // possibility 1: if it's in the table, nothing happens
        if (variableToLocation.find(t.lexeme) != variableToLocation.end()) { //If the variable location exists
            st->LHS = variableToLocation[t.lexeme]; // Get the location of the variable stored in memory

        } else { // possibility 2: if this is the first time we're seeing it, then allocate memory

            variableToLocation[t.lexeme] = next_available;
            next_available++;
            st->LHS = variableToLocation[t.lexeme];   // Address of the variable in memory
        }

    } else {
        syntax_error();
    }

    t = lexer.GetToken();

    if (t.token_type == EQUAL){
        //Store input (EQUAL)
    } else {
        syntax_error();
    }

    parse_expr(st);

    t = lexer.GetToken();
    if (t.token_type == SEMICOLON){
        //Store input SEMICOLON
    } else {
        syntax_error();
    }

    return st;
}

void Parser::parse_expr(struct stmt* st){

    st->op1 = parse_primary();

    Token t = peek();
    //cout << "PEEKED: " << t.token_type << endl;
    if ((t.token_type == PLUS) || (t.token_type == MINUS) || (t.token_type == MULT) || (t.token_type == DIV)){
        st->operand = parse_operator();
        //cout << "SET OPERAND TO:" << st->operand << endl;
        st->op2 = parse_primary();
    }
}

int Parser::parse_operator(){
    Token t = lexer.GetToken();
    if (t.token_type == PLUS){
        //operator -> PLUS
        return PLUS;
    } else if (t.token_type == MINUS){
        //operator -> MINUS
        return MINUS;
    } else if (t.token_type == MULT){
        //operator -> MULT
        return MULT;
    } else if (t.token_type == DIV){
        //operator -> DIV
        return DIV;
    } else {
        syntax_error();
    }
}

int Parser::parse_primary(){
    Token t = lexer.GetToken();

    if (t.token_type == ID){
        //Store input ID

        // possibility 1: if it's in the table, nothing happens
        if (variableToLocation.find(t.lexeme) != variableToLocation.end()) { //If the variable location exists
            return variableToLocation[t.lexeme]; // Get the location of the variable stored in memory

        } else { // possibility 2: if this is the first time we're seeing it, then allocate memory

            variableToLocation[t.lexeme] = next_available;
            next_available++;
            return variableToLocation[t.lexeme];
        }

    } else if (t.token_type == NUM){ // Constant
        //Store input NUM
        memory[next_available] = atoi(t.lexeme.c_str());
        next_available++;
        return next_available - 1;
    } else {
        syntax_error();
    }
}

struct stmt* Parser::parse_main(){
    Token t = lexer.GetToken();
    if (t.token_type == MAIN){
        return parse_procedure_body();
    } else {
        syntax_error();
    }
}

void Parser::parse_inputs(){
    Token t = lexer.GetToken();
    if (t.token_type == NUM){
        inputs.push_back(atoi(t.lexeme.c_str())); // Add to the inputs vector

        if (peek().token_type == NUM){
            parse_inputs();
        }
    } else {
        syntax_error();
    }
}

void execute_program(struct stmt* start) {
    struct stmt* pc;
    pc = start;

    while(pc != NULL) {
        // cout << "STATEMENT TYPE: " << pc->stmt_type << endl;
        // if (pc->stmt_type == ASSIGN_STMT) {
        //     cout << "OPERAND: " << pc->operand << endl;
        // }

        switch(pc->stmt_type) {
            case ASSIGN_STMT: switch (pc->operand) {

                case PLUS: 
                    // cout << memory[pc->LHS] << " = " << memory[pc->op1] << " + " << memory[pc->op2] << endl;
                    memory[pc->LHS] = memory[pc->op1] + memory[pc->op2];

                    
                    break;
                case MINUS: memory[pc->LHS] = memory[pc->op1] - memory[pc->op2];
                    break;
                case MULT: memory[pc->LHS] = memory[pc->op1] * memory[pc->op2];
                    break;
                case DIV: memory[pc->LHS] = memory[pc->op1] / memory[pc->op2];
                    break;
                default:
                    memory[pc->LHS] = memory[pc->op1];
                }

                break;
            case OUTPUT_STMT:

                cout << memory[pc->op1] << " "; // Printing out the value stored at the location (pc->op1)

                break;
            case INPUT_STMT:
                // cout << "INPUT CALLED!" << endl;
                // cout << inputs.front() << endl;

                if (!inputs.empty()){
                    memory[pc->op1] = inputs.front();
                    // cout << "CHANGED " << pc->op1 << " to " << memory[pc->op1] << endl;
                    inputs.pop_front();
                }

                break;
            case PROC_INVOCATION_STMT:

                execute_program(pc->proc_code);
                break;

            case DO_STMT:
                //cout << "EXECUTING DO STATEMENT " << pc->op1 << " TIMES " << endl;
                for (int i = 0; i < memory[pc->op1]; i++) 
                    execute_program(pc->proc_code);

                break;
        
        }
        pc = pc->next; // Move on to next statement/instruction.
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;
    Parser p;
    
    // Initialize memory to all 0s
    for(int i = 0; i < 99; ++i) {
        memory[i] = 0; 
    } 

    // token = lexer.GetToken();
    // token.Print();
    // while (token.token_type != END_OF_FILE)
    // {
    //     token = lexer.GetToken();
    //     token.Print();
    // }

    struct stmt* stmt_list = p.parse_input();
    execute_program(stmt_list);
}