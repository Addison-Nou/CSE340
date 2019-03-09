#ifndef __PROJECT2_H__
#define __PROJECT2_H__

#include <string>
#include "lexer.h"
#include <vector>

struct rule{
    std::string LHS;
    std::vector<std::string> RHS;
    bool valid;
};

class project2 {
  public:
    std::vector<struct rule*> parse_rule_list();
    void print_CFG(std::vector<struct rule*>);
    bool addFirstSets(std::deque<std::string>& LHS_FIRST_SET, std::deque<std::string> RHS_FIRST_SET);
    bool addFollowSets(std::deque<std::string>& LHS_FOLLOW_SET, std::deque<std::string> RHS_FOLLOW_SET);
    bool addFirstToFollowSets(std::deque<std::string>& RHS_FOLLOW_SET, std::deque<std::string> LHS_FIRST_SET);

    void calc_FIRST_sets(std::vector<struct rule *> CFG, project2 p);

  private:
    LexicalAnalyzer lexer;

    Token peek();
    void syntax_error();

    void parse_id_list(struct rule*);
    void parse_rule(struct rule*);
    void parse_right_hand_side(struct rule*);

};

#endif

