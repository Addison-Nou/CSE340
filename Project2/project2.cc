/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>

#include "project2.h"

using namespace std;

// Hashmap to keep track of IDs
std::map<string, int> idToLocation;

std::vector<string> idList;
std::vector<string> printVector;
std::vector<bool> generators; //Vector of bools holding the state of if the character is generating or not
std::vector<bool> reachables; //Vector of bools holding the state of if the character is reachable or not
std::vector<bool> reachableRules; //Vector of bools holding the state of if the character is reachable or not

std::vector<std::deque<string>> FIRST;
std::vector<std::deque<string>> FOLLOW;
std::vector<string> LHS_thingies;
std::vector<string> terminal_thingies;

//parser
Token project2::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

void calc_FIRST_sets(std::vector<struct rule *> CFG, project2 p){
    //cout << "Test 3" << endl;
    //Method to generate FIRSTs of the grammar

    //Applying first rule; loop through the grammar and set every terminal's FIRST to contain itself
    //Note: This also covers the second rule

    //Checking for non-terminals by iterating through the hashmap
    for (std::map<string, int>::iterator it = idToLocation.begin(); it != idToLocation.end(); it++)
    {
        bool isVariable = false;

        //Check the grammar's rules to see if the currently checked value within the hashmap is a LHS rule
        for (int i = 0; i < CFG.size(); i++)
        {
            if (it->second == idToLocation[CFG[i]->LHS])
            {
                isVariable = true;
            }
        }
        FIRST[it->second].push_back(it->first);

        if (isVariable == false)
        { //If the symbol is not a variable (it is a terminal)
            //cout << it->first << " is a terminal" << endl;
            FIRST[it->second].push_back(it->first); //Set it's FIRST to itself
        }
    }

    bool hasChanged = true;

    while (hasChanged)
    {
        hasChanged = false;

        //Iterate through the grammar
        for (int CFG_index = 0; CFG_index < CFG.size(); CFG_index++)
        {
            //cout << "CFG SIZE: " << CFG.size() << endl;
            //cout << "LHS: " << CFG_index << endl;

            // Rule 3
            //Applying third rule; if A -> Xa is a grammar rule, add FIRST(X) - {#} to FIRST(A)

            //If the rule is something followed by any number of something elses (A -> X alpha), then add FIRST(X) - epsilon to FIRST(A)
            if (CFG[CFG_index]->RHS.size() > 0)
            {
                //Loop through FIRST(X) and add them to FIRST(A)
                std::deque<string> firstOfRHS = FIRST[idToLocation[CFG[CFG_index]->RHS[0]]];
                // cout << "RULE 3" << endl;
                // cout << "RHS: " << CFG[CFG_index]->RHS[0] << endl;
                // cout << "LHS: " << CFG[CFG_index]->LHS << endl;
                if (p.addFirstSets(FIRST[idToLocation[CFG[CFG_index]->LHS]], firstOfRHS))
                    hasChanged = true;
            }

            //Applying 4th rule; if all the symbols' FIRST sets have epsilon except the last one, then add the last one's FIRST set to FIRS ST(A)

            //Check all symbols in the RHS except the last one
            if (CFG[CFG_index]->RHS.size() > 0)
            {
                //cout << "RHS 0: " << CFG[CFG_index]->RHS[0] << endl;
                bool hasEpsilon = false;

                //Loop through all RHS rules' FIRST sets
                for (int j = 0; j < CFG[CFG_index]->RHS.size(); j++)
                {

                    //When first checking the symbol, set hasEpsilon flag to false
                    hasEpsilon = false;

                    // We want to check if the FIRST of the RHS contains epsilon. If it does, then add the FIRST of the next RHS value

                    // Need another loop to check the FIRST set
                    std::deque<string> RHS_FIRST_SET = FIRST[idToLocation[CFG[CFG_index]->RHS[j]]];
                    for (int k = 1; k < RHS_FIRST_SET.size(); k++)
                    {
                        //cout << "RHS FIRST SET k => " << RHS_FIRST_SET[k] << " (RHS index " << k << ") == "
                                //<< "#?" << endl;

                        if (RHS_FIRST_SET[k] == "#")
                        { //If the symbol within the RHS symbol's FIRST set contains an epsilon, then set hasEpsilon flag to true and move on
                            hasEpsilon = true;

                            //cout << "Has epsilon " << endl;

                            if (j == CFG[CFG_index]->RHS.size() - 1)
                            {
                                // Implement Rule 5
                                // S -> A B C (size == 4)
                                // A contains epsilon
                                // B contains epsilon
                                // C contains epsilon
                                // j will equal 3 if all of them contain epsilon

                                string RHS_value = "#";
                                std::deque<string> LHS_FIRST_SET = FIRST[idToLocation[CFG[CFG_index]->LHS]];
                                bool found = (std::find(LHS_FIRST_SET.begin(), LHS_FIRST_SET.end(), RHS_value) != LHS_FIRST_SET.end());

                                if (!found)
                                {
                                    std::deque<string>::iterator it = FIRST[idToLocation[CFG[CFG_index]->LHS]].begin();
                                    ++it;
                                    FIRST[idToLocation[CFG[CFG_index]->LHS]].insert(it, "#");
                                    //cout << "Rule 5 " << endl;
                                    hasChanged = true;
                                }
                            }
                            else
                            {
                                // Add the first of RHS to first of LHS
                                std::deque<string> firstOfRHS = FIRST[idToLocation[CFG[CFG_index]->RHS[j + 1]]];
                                if (p.addFirstSets(FIRST[idToLocation[CFG[CFG_index]->LHS]], firstOfRHS))
                                    hasChanged = true;
                                //cout << "Rule 4 " << endl;

                                // S -> A B C D E
                                // A contains epsilon
                                // Add FIRST(B) to FIRST(S)
                                // B contains epsilon
                                // Add FIRST(C) to FIRST(S)
                                // C does not contain epsilon
                                // STOP RULE #4
                            }

                            break;
                        }
                    }

                    if (!hasEpsilon)
                    {
                        break;
                    }
                }
            }
        }

        //Printing FIRST sets
        //cout << "------------------------------------------------------------------" << endl;
        //cout << "Printing ALL FIRST sets: " << endl;
    }
}

void remove_Useless_Symbols(std::vector<struct rule *> CFG)
{
    // cout << "Test 2" << endl;
    //generators

    // Initialize generators
    // Initialize LHS (non-terminals)
    for (int nonterm_index = 0; nonterm_index < LHS_thingies.size(); nonterm_index++) {
        generators[idToLocation[LHS_thingies[nonterm_index]]] = false;
        // cout << LHS_thingies[nonterm_index] << endl;

    }

    // Initialize Terminals
    for (int term_index = 0; term_index < terminal_thingies.size(); term_index++) {
        generators[idToLocation[terminal_thingies[term_index]]] = true;
        // cout << terminal_thingies[term_index] << endl;
    }

    bool hasChanged = true;
    
    while (hasChanged == true) {
        hasChanged = false;

        // Loop thorough rules
        for (rule* currentRule : CFG) {
            // If the current rule's LHS is generating then continue
            if (generators[idToLocation[currentRule->LHS]] == true)
                continue;

            //Generating if X -> terminals
            //Generating if X -> A B C, where A B C are all marked 'generating' (generators[A], generators[B], generators[C] == true)

            bool rhs_gens = true;
            for (string rhs_symbol : currentRule->RHS) {
                if (generators[idToLocation[rhs_symbol]] == false) {
                    rhs_gens = false;
                    break;
                }
            }
            
            //If the RHS is all true, then set the LHS rule we're checking to true
            if (rhs_gens) {
                generators[idToLocation[currentRule->LHS]] = true;
                hasChanged = true;
            }
            
        }
    }

    //Reachable Symbols
    hasChanged = true;

    reachables = vector<bool>(idToLocation.size(), false);   //Initializing reachables vector to false
    reachableRules = vector<bool>(CFG.size(), false);   //Initializing reachableRules vector to false

    // If the starting symbol is not generating, then none of the rules are reachable
    string startVar = CFG[0]->LHS;

    if (generators[idToLocation[startVar]] == true)
        reachables[idToLocation[startVar]] = true;
    else
        return;

    while (hasChanged == true) {
        hasChanged = false;
        
        // Loop thorough rules
        for (int rule_idx = 0; rule_idx < CFG.size(); rule_idx++) {
            
            rule* currentRule = CFG[rule_idx];
            // If LHS is reachable, look through RHS. If all RHS symbols are generating then make each symbol reachable.

            // If LHS is reachable, entire RHS is reachable
            
            if (reachables[idToLocation[currentRule->LHS]]){
                bool rhs_gens = true;

                for(string rhs_symbol : currentRule->RHS) {
                    // Check if every symbol on the RHS generates
                    if (generators[idToLocation[rhs_symbol]] != true) {
                        rhs_gens = false;
                        break;
                    }
                }

                if (rhs_gens)   //Make every symbol in the RHS reachable
                { 
                    for (string rhs_symbol : currentRule->RHS)
                    {
                        if (reachables[idToLocation[rhs_symbol]] == false)
                        {
                            reachables[idToLocation[rhs_symbol]] = true;
                            hasChanged = true;
                        }
                    }
                    reachableRules[rule_idx] = true;
                }
            }
        }
    }
}

void calc_FOLLOW_sets(std::vector<struct rule *> CFG, project2 p)
{
    //cout << "Test 4" << endl;

    //To do FOLLOW sets, you must first do FIRST sets
    calc_FIRST_sets(CFG, p);
    //cout << endl << "Printing FOLLOW sets: " << endl << endl;
    //printFollowSets_working();

    //cout << endl << "FOLLOW sets printed" << endl << endl;

    //RULE 1: SET STARTING'S FOLLOW TO CONTAIN '$'

    //Checking for non-terminals by iterating through the hashmap
    for (std::map<string, int>::iterator it = idToLocation.begin(); it != idToLocation.end(); it++)
    {

        FOLLOW[it->second].push_back(it->first);

        if (it->second == idToLocation[CFG[0]->LHS])
        {
            // cout << "it->second = " << idToLocation[CFG[0]->LHS] << ", pushing it to FOLLOW[" << it->second << "]" << endl;
            FOLLOW[it->second].push_back("$");
            // cout << "Pushed it to FOLLOW[" << it->second << "]" << endl;
        }
    }

    // Loop through CFG rules (FOR RULES 4 and 5)
    for (int CFG_index = 0; CFG_index < CFG.size(); CFG_index++)
    {
        struct rule *rule = CFG[CFG_index];

        //RULE 4: If S -> a B C D E is a grammar rule, add FIRST(E) to FOLLOW(D)
        // Loop through the RHS backwards (FOR RULE 4 and 5)
        for (int RHS_index = rule->RHS.size() - 1; RHS_index > 0; RHS_index--)
        {
            string E = rule->RHS[RHS_index];
            string D = rule->RHS[RHS_index-1];

            p.addFirstToFollowSets(FOLLOW[idToLocation[D]], FIRST[idToLocation[E]]);
            // cout << "Rule 4 triggered" << endl;

            //RULE 5: if A -> a B C D E is a grammar rule D contains epsilon, add FIRST(E) to FOLLOW(C)

            // If D contains epsilon, then add FIRST(E) to FOLLOW(C).
            // If not then, break out of loop
            // Otherwise, if C contains epsilon, then add FIRST(E) to FOLLOW(B)

            for (int RHS_contains_epsilon = RHS_index - 1; RHS_contains_epsilon > 0; RHS_contains_epsilon--) {
                
                string beforeE = rule->RHS[RHS_contains_epsilon];

                bool containsEpsilon = (find(FIRST[idToLocation[beforeE]].begin(), FIRST[idToLocation[beforeE]].end(), "#") != FIRST[idToLocation[beforeE]].end());
                if (containsEpsilon)
                {
                    // Add FIRST(E) to FOLLOW(C)
                    string C = rule->RHS[RHS_contains_epsilon - 1];
                    p.addFirstToFollowSets(FOLLOW[idToLocation[C]], FIRST[idToLocation[E]]);
                }
                else break;
            }
        } 
    }


    bool hasChanged = true;

    while (hasChanged)
    {

        hasChanged = false;

        // Loop through CFG rules (FOR RULES 2 and 3)
        for (int CFG_index = 0; CFG_index < CFG.size(); CFG_index++)
        {
            struct rule *rule = CFG[CFG_index];
            
            // Loop through the RHS backwards (FOR RULE 2 and 3)
            for (int RHS_index = rule->RHS.size() - 1; RHS_index >= 0; RHS_index--)
            {
                // S -> A B
                string B = rule->RHS[RHS_index];
                string S = rule->LHS;

                // If B is not a non-terminal, move on
                if (find(LHS_thingies.begin(), LHS_thingies.end(), B) == LHS_thingies.end())
                    break;

                //RULE 2: If S -> A B is a grammar rule, add FOLLOW(S) to FOLLOW(B)
                if (RHS_index == rule->RHS.size() - 1)  //If we're looking at the last token
                {
                    if (p.addFollowSets(FOLLOW[idToLocation[B]], FOLLOW[idToLocation[S]]))
                    {
                        hasChanged = true;
                        // cout << "Rule 2 applied: Follow " << S << " to Follow " << B << endl;
                    }
                    // else
                        // cout << "Rule 2 not applied to " << B << endl;
                }

                //RULE 3: If S -> A B C D E is a grammar rule, where the FIRST SETS of C,D, and E contain EPSILON, then add FOLLOW(S) to FOLLOW(B)
                // If FIRST(E) contains epsilon
                bool containsEpsilon = (find(FIRST[idToLocation[B]].begin(), FIRST[idToLocation[B]].end(), "#") != FIRST[idToLocation[B]].end());
                if (RHS_index > 0 && containsEpsilon)
                {
                    // Add FOLLOW(S) to FOLLOW(D)
                    string D = rule->RHS[RHS_index - 1];
                    if (p.addFollowSets(FOLLOW[idToLocation[D]], FOLLOW[idToLocation[S]]))
                    {
                        hasChanged = true;
                        // cout << "Rule 3 applied to " << D << endl;
                    }
                    // else
                        // cout << "Rule 3 not applied to " << D << endl;
                }
                else break; //Move onto next CFG rule

            } // END LOOP for RULE 2 and 3

        } // END LOOP for ALL RULES
    }
}

void project2::syntax_error()
{
    cout << "SYNTAX ERROR\n";
    exit(1);
}

void project2::print_CFG(std::vector<struct rule *> CFG)
{

    cout << "Current CFG: " << endl;
    for (int i = 0; i < CFG.size(); i++)
    {
        cout << CFG[i]->LHS << " -> ";
        for (int j = 0; j < CFG[i]->RHS.size(); j++)
        {
            cout << CFG[i]->RHS[j] << " ";
        }
        cout << endl;
    }
}

std::vector<struct rule *> project2::parse_rule_list()
{

    std::vector<struct rule *> CFG;
    Token t = lexer.GetToken();

    while (true)
    {
        struct rule *rule = new struct rule;
        rule->valid = true;

        if (t.token_type == ID)
        { //valid input

            //if the ID is not in the table, it gets added
            if (idToLocation.find(t.lexeme) == idToLocation.end())
            {                                                 //If the variable location exists
                idToLocation[t.lexeme] = idToLocation.size(); //Add the id to the hashmap
            }

            rule->LHS = t.lexeme; //Set the left hand side of the current rule

            parse_rule(rule);
            CFG.push_back(rule);
        }
        else if (t.token_type == DOUBLEHASH)
            return CFG;
        else
            syntax_error();

        t = lexer.GetToken();
    }
}

void project2::parse_rule(struct rule *rule)
{
    Token t = lexer.GetToken();
    if (t.token_type == ARROW)
    {                                //If the token is ARROW
        parse_right_hand_side(rule); //Parse the right hand side
        t = lexer.GetToken();
        if (t.token_type != HASH)
        {                   //After parsing the right hand side, if we do NOT have a hash (there has to be a hash, since it must be done parsing after parsing the right hand side)
            syntax_error(); //Return an error
        }
    }
    else
        syntax_error();
}

void project2::parse_right_hand_side(struct rule *rule)
{

    Token t = peek();
    if (t.token_type == ID)
    {                        //If the token is an ID
        parse_id_list(rule); //Parse ID List
    }
    else if (t.token_type == HASH)
    { //If it's an epsilon (hash)
        rule->RHS.push_back("#");
    }
    else
        syntax_error();
}

void project2::parse_id_list(struct rule *rule)
{
    Token t = lexer.GetToken();
    if (t.token_type == ID)
    {
        if (idToLocation.find(t.lexeme) == idToLocation.end())
        {                                                 //If the variable location exists
            idToLocation[t.lexeme] = idToLocation.size(); //Add the id to the hashmap
        }

        rule->RHS.push_back(t.lexeme); //Set the left hand side of the current rule

        t = peek();
        if (t.token_type == ID)
            parse_id_list(rule);
    }
    else if (t.token_type == HASH)
        return;
}

bool project2::addFirstSets(std::deque<string>& LHS_FIRST_SET, std::deque<string> RHS_FIRST_SET)
{
    bool changed = false;
    if (RHS_FIRST_SET.size() > 1) {
        for (int j = 1; j < RHS_FIRST_SET.size(); j++)
        {

            std::string RHS_value = RHS_FIRST_SET[j];

            bool found = (std::find(LHS_FIRST_SET.begin(), LHS_FIRST_SET.end(), RHS_value) != LHS_FIRST_SET.end());
            if (RHS_value != "#" && !found)
            { //If it's not epsilon then add it
                LHS_FIRST_SET.push_back(RHS_value);
                // cout << "Push back " << RHS_value << " to " << LHS_FIRST_SET[0] << endl;
                changed = true;
            }
        }
    }

    return changed;
}

bool project2::addFollowSets(std::deque<string> &LHS_FOLLOW_SET, std::deque<string> RHS_FOLLOW_SET)
{
    bool changed = false;
    if (RHS_FOLLOW_SET.size() > 1)
    {
        for (int j = 1; j < RHS_FOLLOW_SET.size(); j++)
        {

            std::string RHS_value = RHS_FOLLOW_SET[j];

            bool found = (std::find(LHS_FOLLOW_SET.begin(), LHS_FOLLOW_SET.end(), RHS_value) != LHS_FOLLOW_SET.end());

            if (!found) {
                LHS_FOLLOW_SET.push_back(RHS_value);
                //cout << "(Add Follow Sets) Push back " << RHS_value << " to " << LHS_FOLLOW_SET[0] << endl;
                changed = true;
            }
        }
    }

    return changed;
}

bool project2::addFirstToFollowSets(std::deque<string>& RHS_FOLLOW_SET, std::deque<string> LHS_FIRST_SET)
{

    // ////////////
    // for (int LHS_index = 0; LHS_index < LHS_thingies.size(); LHS_index++)
    // {
    //     for (int i = 0; i < FIRST.size(); i++)
    //     {

    //         if (FIRST[i][0] == LHS_thingies[LHS_index])
    //         {
    //             cout << "FIRST(" << FIRST[i][0] << ") = { ";
    //             for (int j = 1; j < FIRST[i].size(); j++)
    //             {
    //                 cout << FIRST[i][j];
    //                 if (j == FIRST[i].size() - 1) {
    //                     cout << " ";
    //                 } else {
    //                     cout << ", ";
    //                 }
    //             }
    //             cout << "}" << endl;
    //         }
    //     }
    // }
    // //////////////

    bool changed = false;
    //cout << endl << "LHS_FIRST_SET SIZE: " << LHS_FIRST_SET.size() << endl;
    //cout << "LHS_FIRST_SET[0] = " << LHS_FIRST_SET[1] << endl;

    for (int j = 1; j < LHS_FIRST_SET.size(); j++)
    {
        
        std::string LHS_value = LHS_FIRST_SET[j];
        //cout << "LHS = " << LHS_FIRST_SET[0] << endl;
        //cout << "LHS_value = " << LHS_value << endl;
        //cout << "RHS = " << RHS_FOLLOW_SET[0] << endl;

        if (LHS_value == "#")
            continue;

        bool found = (std::find(RHS_FOLLOW_SET.begin(), RHS_FOLLOW_SET.end(), LHS_value) != RHS_FOLLOW_SET.end());

        //cout << "found: " << found << endl;

        if (!found) //If doesn't exist yet then add it
        {
            RHS_FOLLOW_SET.push_back(LHS_value);
            // cout << "(Add First to Follow) Push back " << LHS_value << " to " << RHS_FOLLOW_SET[0] << endl;
            changed = true;
        }
    }

    return changed;
}

void createNonTerminals(std::vector<struct rule *> CFG) {
    // Create a list of LHS variables
    for (int i = 0; i < CFG.size(); i++)
    {

        //Check to see if it has been added to the LHS vector yet
        bool hasBeenAdded = false;
        for (int j = 0; j < LHS_thingies.size(); j++)
        {
            if (CFG[i]->LHS == LHS_thingies[j])
            {
                hasBeenAdded = true;
            }
        }

        //If it has not been added to the print vector already, then add it
        if (!hasBeenAdded)
        {
            LHS_thingies.push_back(CFG[i]->LHS); //Add it to the print vector so we don't print it again
            //cout << "Added LHS: " << it->first << endl;
        }
    }
}

void createTerminals(std::vector<struct rule *> CFG) {
    
    // Create a list of terminals
    for (int i = 0; i < CFG.size(); i++)
    {
        for (string RHS_token : CFG[i]->RHS) {
            
            bool inLHS = (std::find(LHS_thingies.begin(), LHS_thingies.end(), RHS_token) != LHS_thingies.end());
            
            bool in_terminal_thingies = (std::find(terminal_thingies.begin(), terminal_thingies.end(), RHS_token) != terminal_thingies.end());
            if (!inLHS && !in_terminal_thingies)
                terminal_thingies.push_back(RHS_token); //Add it to the print vector so we don't print it again
        }
    }
}

// SORTING
bool sortingFunction(string i, string j)
{
    return (idToLocation)[i] < (idToLocation)[j];
}

void printFirstSets() {
    // SORT RHS OF EVERY FIRST SET
    for (int i = 0; i < FIRST.size(); i++) {
        sort(FIRST[i].begin()+1, FIRST[i].end(), sortingFunction);
    }

    for (int i = 0; i < FIRST.size(); i++)
    {
        cout << "FIRST(" << FIRST[i][0] << ") = { ";
        for (int j = 1; j < FIRST[i].size(); j++)
        {
            cout << FIRST[i][j];
            if (j == FIRST[i].size() - 1)
            {
                cout << " ";
            }
            else
            {
                cout << ", ";
            }
        }
        cout << "}" << endl;
    }
}

void printFollowSets() {
    // SORT RHS OF EVERY FIRST SET
    for (int i = 0; i < FOLLOW.size(); i++) {
        sort(FOLLOW[i].begin()+1, FOLLOW[i].end(), sortingFunction);
    }

    for (int i = 0; i < FIRST.size(); i++)
    {
        cout << "FOLLOW(" << FOLLOW[i][0] << ") = { ";
        for (int j = 1; j < FOLLOW[i].size(); j++)
        {
            cout << FOLLOW[i][j];
            if (j == FOLLOW[i].size() - 1)
            {
                cout << " ";
            }
            else
            {
                cout << ", ";
            }
        }
        cout << "}" << endl;
    }
}

void printFollowSets_working() {
    
    // SORT RHS OF EVERY FOLLOW SET
    for (int i = 0; i < FOLLOW.size(); i++) {
        sort(FOLLOW[i].begin()+1, FOLLOW[i].end(), sortingFunction);
    }

    for (int i = 0; i < LHS_thingies.size(); i++) {
        sort(LHS_thingies.begin(), LHS_thingies.end(), sortingFunction);
    }

    for (int LHS_index = 0; LHS_index < LHS_thingies.size(); LHS_index++)
    {
        for (int i = 0; i < FOLLOW.size(); i++)
        {

            if (FOLLOW[i][0] == LHS_thingies[LHS_index])
            {
                cout << "FOLLOW(" << FOLLOW[i][0] << ") = { ";
                for (int j = 1; j < FOLLOW[i].size(); j++)
                {
                    cout << FOLLOW[i][j];
                    if (j == FOLLOW[i].size() - 1) {
                        cout << " ";
                    } else {
                        cout << ", ";
                    }
                }
                cout << "}" << endl;
            }
        }
    }
}

void printFirstSets_working() {
    
    // SORT RHS OF EVERY FIRST SET
    for (int i = 0; i < FIRST.size(); i++) {
        sort(FIRST[i].begin()+1, FIRST[i].end(), sortingFunction);
    }

    for (int i = 0; i < LHS_thingies.size(); i++) {
        sort(LHS_thingies.begin(), LHS_thingies.end(), sortingFunction);
    }

    for (int LHS_index = 0; LHS_index < LHS_thingies.size(); LHS_index++)
    {
        for (int i = 0; i < FIRST.size(); i++)
        {

            if (FIRST[i][0] == LHS_thingies[LHS_index])
            {
                cout << "FIRST(" << FIRST[i][0] << ") = { ";
                for (int j = 1; j < FIRST[i].size(); j++)
                {
                    cout << FIRST[i][j];
                    if (j == FIRST[i].size() - 1) {
                        cout << " ";
                    } else {
                        cout << ", ";
                    }
                }
                cout << "}" << endl;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // cout << "Beginning of Main" << endl;
    idToLocation["#"] = 0; //Instantiating first pair
    idToLocation["$"] = 1; //Instantiating second pair
    project2 p;

    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);

    // TODO: Read the input grammar at this point from standard input
    //cout << "Pre parse rules" << endl;
    
    std::vector<struct rule *> CFG = p.parse_rule_list();

    //p.print_CFG(CFG);
    createNonTerminals(CFG);
    createTerminals(CFG);
    /*
       Hint: You can modify and use the lexer from previous project
       to read the input. Note that there are only 4 token types needed
       for reading the input in this project.

       WARNING: You will need to modify lexer.cc and lexer.h to only
       support the tokens needed for this project if you are going to
       use the lexer.
     */



    FIRST = std::vector<std::deque<string>>(idToLocation.size(), std::deque<string>(0)); // Defaults to zero initial value
    FOLLOW = std::vector<std::deque<string>>(idToLocation.size(), std::deque<string>(0));
    generators = std::vector<bool>(idToLocation.size(), false);

    // cout << "End of Main" << endl;

    switch (task)
    {
    case 1:
        // cout << "Test 1" << endl;
        //DESC: Output the list of non-terminals followed by the lister of terminals in the order in which they appear in the grammar rules

        // Walk through grammar, print out the nonterminals if they haven't been printed yet



        //Checking for non-terminals
        //Iterate through the CFG
        for (int i = 0; i < CFG.size(); i++)
        {

            //Check to see if it has been added to the print vector yet
            bool print = true;
            for (int j = 0; j < printVector.size(); j++)
            {
                if (CFG[i]->LHS == printVector[j])
                {
                    print = false;
                }
            }

            //If the LHS value has not been added to the print vector already, then add it
            if (print)
            {
                printVector.push_back(CFG[i]->LHS); //Add it to the print vector so we don't print it again
                //cout << "Added non-terminal (LHS): " << CFG[i]->LHS << endl;
            }

            // Loop thorugh RHS
            for (int k = 0; k < CFG[i]->RHS.size(); k++)
            {

                //If the token on the RHS is a non-terminal, then continue to check if it exists in the printVector yet
                bool isALHS = false;
                for (int l = 0; l < LHS_thingies.size(); l++)
                {
                    if (LHS_thingies[l] == CFG[i]->RHS[k])
                    {
                        isALHS = true;
                        break;
                    }
                }

                if (isALHS)
                { //String exists in LHS_thingies dictionary

                    //Check to see if it has been added to the print vector yet
                    print = true;
                    for (int m = 0; m < printVector.size(); m++)
                    {
                        if (CFG[i]->RHS[k] == printVector[m])
                        {
                            print = false;
                        }
                    }
                }
                else
                {
                    continue; // Move on to the next RHS value
                }

                //If it has not been added to the print vector already, then add it
                if (print)
                {
                    printVector.push_back(CFG[i]->RHS[k]); //Add it to the print vector so we don't print it again
                    //cout << "Added non-terminal (RHS): " << CFG[i]->RHS[k] << endl;
                }
            }
        }

        //Checking for terminals
        //Iterate through the hashmap
        for (int i = 0; i < CFG.size(); i++)
        {

            for (int j = 0; j < CFG[i]->RHS.size(); j++)
            {

                //Check the grammar's rules to see if the currently checked value within the hashmap is a LHS rule
                bool print = true;

                if (CFG[i]->RHS[j] == CFG[i]->LHS)
                { //If it is a LHS rule, then set print to false
                    print = false;
                }

                //Check to see if it has been added to the print vector yet
                for (int k = 0; k < printVector.size(); k++)
                {
                    if (CFG[i]->RHS[j] == printVector[k] || CFG[i]->RHS[j] == "#")
                    {
                        print = false;
                    }
                }

                //If it has not been added to the print vector already
                if (print)
                {
                    printVector.push_back(CFG[i]->RHS[j]); //Add it to the print vector so we don't print it again
                    //cout << "Added terminal: " << CFG[i]->RHS[j] << endl;
                }
            }
        }

        //Printing out the printVector
        for (int i = 0; i < printVector.size(); i++)
        {
            cout << printVector[i] << " ";
        }

        break;

    case 2:
        remove_Useless_Symbols(CFG);

        for (int i = 0; i < reachableRules.size(); i++) {
            if (reachableRules[i]) {
                cout << CFG[i]->LHS << " -> ";
                for (int j = 0; j < CFG[i]->RHS.size(); j++)
                {
                    cout << CFG[i]->RHS[j] << " ";
                }
                cout << endl;
            }
        }
        
    break;

    case 3:
    {
        calc_FIRST_sets(CFG, p);
        printFirstSets_working();
    }
    break;

    case 4:

        //Printing FOLLOW sets
        //p.print_CFG(CFG);
        calc_FOLLOW_sets(CFG, p);
        //cout << "Case 4 done" << endl;
        //printFollowSets();
        //cout << endl;
        //printFirstSets();
        printFollowSets_working();

        break;

    case 5:
    {
        //cout << "Test 5" << endl;
        //The parser is predictive if two different rules lead to the same (non)terminal
        bool pred = true;
        for (int i = 0; i < CFG.size(); i++)
        {
            for (int j = 0; j < CFG.size(); j++)
            {
                if (i != j)
                {
                    if (CFG[i]->RHS == CFG[j]->RHS)
                    { //If two different rules have the same RHS
                        pred = false;
                    }
                }
            }
        }
        if (pred)
            cout << "YES" << endl;
        else
            cout << "NO" << endl;
            }

            break;

            default:
                cout << "Error: unrecognized task number " << task << "\n";
                break;
            }
            return 0;
        }
    