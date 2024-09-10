#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"
#include <iostream>
#include <unordered_map>

using namespace std;

class syntax_analyzer {
	private:
		LexicalAnalyzer lexer;
		Token Expect(TokenType);

		int mapCheck(std::string);
		int Parse_Primary();

		InstructionNode* Parse_Body();
		InstructionNode* Parse_Stmt_List();
		InstructionNode* ParseStmt();
		InstructionNode* ParseAssignStmt();

		InstructionNode* Parse_Output_Stmt();
		InstructionNode* Parse_Input_Stmt();
		InstructionNode* Parse_While_Stmt();
		InstructionNode* Parse_If_Stmt();
		InstructionNode* Parse_Switch_Stmt();
		InstructionNode* Parse_For_Stmt();

		InstructionNode* Parse_Default_Case();

		ArithmeticOperatorType Parse_Operation();
		ConditionalOperatorType Parse_Reloop();

		void Parse_Condition(InstructionNode*);
		void Parse_Id_List();
		void Parse_Expr(InstructionNode*);
		void Parse_Case_List(InstructionNode*, InstructionNode*);
		void Parse_Case(InstructionNode*, InstructionNode*);
		void Parse_Num_List();

	public:
		InstructionNode* Parse_Program();
		std::unordered_map<std::string, int> location;
};

struct InstructionNode* parse_generate_intermediate_representation() {
	syntax_analyzer inputValue;
	InstructionNode* node = inputValue.Parse_Program();
	return node;
}

Token syntax_analyzer::Expect(TokenType expected_output) {
	Token currentToken = lexer.GetToken();

	if (currentToken.token_type != expected_output) {
		cout << "SYNTAX ERROR" << endl;
		exit(1);
	}
	return currentToken;
}

int syntax_analyzer::mapCheck(string parse_val) {
	for (auto var : location) {
		if (var.first == parse_val) {
			return var.second; 
		}
	}
	return next_available++;
}

InstructionNode* syntax_analyzer::Parse_Program() {
	InstructionNode* node;

	Parse_Id_List();
	Expect(SEMICOLON);

	node = ParseBody();
	ParseNumList();
	Expect(END_OF_FILE);

	return node;
}

void syntax_analyzer::Parse_Id_List() {
	int locationMap = 0;
	Token token = Expect(ID);

	locationMap = mapCheck(token.lexeme);
	location[token.lexeme] = locationMap;
	mem[locationMap] = 0;

	token = lexer.GetToken();
	if (token.token_type == COMMA) { 
		Parse_Id_List();
	}
	else { 
		lexer.UngetToken(1); 
	}
}

InstructionNode* syntax_analyzer::Parse_Body() {
	Expect(LBRACE);

	InstructionNode* node = Parse_Stmt_List();

	Expect(RBRACE);

	return node;
}

InstructionNode* syntax_analyzer::Parse_Stmt_List() {
	Token token = lexer.peek(1);

	InstructionNode* node = Parse_Stmt();
	InstructionNode* nxt;
	InstructionNode* ptr;

	if (token.token_type == ID || p.token_type == IF || p.token_type == WHILE || p.token_type == SWITCH || p.token_type == FOR || p.token_type == INPUT || p.token_type == OUTPUT) {
		nxt = Parse_Stmt_List();
		ptr = node;

		while ((*ptr).nxt != NULL) {
			ptr = (*ptr).nxt; 
		}

		(*ptr).nxt = nxt;
	}

	return node;
}

InstructionNode* syntax_analyzer::Parse_Stmt() {
	InstructionNode* node = NULL;
	Token token = lexer.peek(1);


	if (token.token_type == ID) { 
		node = Parse_Assign_Stmt(); 
	}
	else if (token.token_type == WHILE) { 
		node = Parse_While_Stmt(); 
	}
	else if (token.token_type == IF) { 
		node = Parse_If_Stmt(); 
	}
	else if (token.token_type == SWITCH) {
		node = Parse_Switch_Stmt(); 
	}
	else if (token.token_type == FOR) {
		node = Parse_For_Stmt(); 
	}
	else if (token.token_type == OUTPUT) {
		node = Parse_Output_Stmt(); 
	}
	else if (token.token_type == INPUT) {
		node = Parse_Input_Stmt(); 
	}

	return node;
}

InstructionNode* syntax_analyzer::Parse_Assign_Stmt() {
	InstructionNode* node = new InstructionNode;
	Token tokenID = Expect(ID);
	Token tokenPeek = lexer.peek(1);

	(*node).type = ASSIGN;
	(*node).assign_inst.left_hand_side_index = mapCheck(tokenID.lexeme);
	(*node).next = NULL;

	Expect(EQUAL);

	if (tokenPeek.token_type == ID || tokenPeek.token_type == NUM) {
		tokenPeek = lexer.peek(2);

		if (tokenPeek.token_type == PLUS || tokenPeek.token_type == MINUS || tokenPeek.token_type == MULT || tokenPeek.token_type == DIV) {
			Parse_Expr(node);
		}
		else {
			(*node).assign_inst.op = OPERATOR_NONE;
			(*node).assign_inst.operand1_index = ParsePrimary();
		}
	}
	else { 
		cout << "SYNTAX ERROR" << endl;
		exit(1);
	}

	Expect(SEMICOLON);

	return node;
}

void syntax_analyzer::Parse_Expr(InstructionNode* pNode) {

	(*pNode).assign_inst.operand1_index = Parse_Primary();
	(*pNode).assign_inst.op = Parse_Operator();
	(*pNode).assign_inst.operand2_index = Parse_Primary();

}



int syntax_analyzer::Parse_Primary() {
	int locationMap = 0;
	Token token = lexer.GetToken();

	locationMap = Location(t.lexeme);

	if (t.token_type == NUM) {
		location[token.lexeme] = locationMap;
		mem[locationMap] = std::stoi(token.lexeme);
	}
	else if (token.token_type != ID) {
		cout << "SYNTAX ERROR" << endl;
		exit(1);
	}

	return locationMap;
}

ArithmeticOperatorType syntax_analyzer::Parse_Operator() {
	Token token = lexer.GetToken();

	if (t.token_type == PLUS) { 
		return OPERATOR_PLUS; 
	}
	else if (t.token_type == MINUS) {
		return OPERATOR_MINUS; 
	}
	else if (t.token_type == MULT) {
		return OPERATOR_MULT; 
	}
	else if (t.token_type == DIV) {
		return OPERATOR_DIV; 
	}
	else { 
		cout << "SYNTAX ERROR" << endl;
		exit(1);
	}
}

InstructionNode* syntax_analyzer::Parse_Output_Stmt() {
	InstructionNode* node = new InstructionNode;
	Token token = Expect(ID);

	Expect(OUTPUT);

	(*node).type = OUT;
	(*node).output_inst.var_index = mapCheck(token.lexeme);
	(*node).next = NULL;

	Expect(SEMICOLON);

	return node;
}



InstructionNode* syntax_analyzer::ParseInputStmt() {
	InstructionNode* node = new InstructionNode;
	Token token = Expect(ID);

	Expect(INPUT);

	(*node).type = IN;
	(*node).input_inst.var_index = mapCheck(token.lexeme);
	(*node).next = NULL;

	Expect(SEMICOLON);

	return node;
}

InstructionNode* syntax_analyzer::ParseWhileStmt() {
	InstructionNode* node = new InstructionNode;
	InstructionNode* noop = new InstructionNode;
	InstructionNode* jump = new InstructionNode;

	(*node).type = CJMP;
	(*node).cjmp_inst.target = noop;
	(*jump).type = JMP;
	(*jump).jmp_inst.target = node;
	(*jump).next = noop;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	Expect(WHILE);
	Parse_Condition(node);
	(*node).next = Parse_Body();

	InstructionNode* pointer = node;
	while ((*pointer).next != NULL) { 
		pointer = (*pointer).next; 
	}
	(*pointer).next = jump;

	return node;
}

InstructionNode* syntax_analyzer::Parse_If_Stmt() {
	InstructionNode* node = new InstructionNode;
	InstructionNode* noop = new InstructionNode;

	Expect(IF);
	(*node).type = CJMP;
	(*node).cjmp_inst.target = noop;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	ParseCondition(node);
	(*node).next = Parse_Body();

	InstructionNode* pointer = node;
	while ((*pointer).next != NULL) {
		pointer = (*pointer).next; 
	}
	(*pointer).next = noop;

	return node;
}

void syntax_analyzer::Parse_Condition(InstructionNode* pNode) {
	(*pNode).cjmp_inst.operand1_index = Parse_Primary();
	(*pNode).cjmp_inst.condition_op = Parse_Reloop();
	(*pNode).cjmp_inst.operand2_index = Parse_Primary();
}

ConditionalOperatorType syntax_analyzer::Parse_Reloop() {
	Token token = lexer.GetToken();

	if (token.token_type != LESS && token.token_type != GREATER && token.token_type != NOTEQUAL) {
		cout << "SYNTAX ERROR" << endl;
		exit(1);
	}

	if (t.token_type == LESS) {
		return CONDITION_LESS;
	}
	else if (t.token_type == GREATER) {
		return CONDITION_GREATER;
	}
	else if (t.token_type == NOTEQUAL) {
		return CONDITION_NOTEQUAL; 
	}
}

InstructionNode* syntax_analyzer::Parse_Switch_Stmt() {
	InstructionNode* node = new InstructionNode;
	InstructionNode* noop = new InstructionNode;

	Token tokenPeek = lexer.peek(1);
	Token tokenID = Expect(ID);

	Expect(SWITCH);

	(*node).type = CJMP;
	(*node).cjmp_inst.operand1_index = mapCheck(tokenID.lexeme);;
	(*node).cjmp_inst.condition_op = CONDITION_NOTEQUAL;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	Expect(LBRACE);

	ParseCaseList(node, noop);

	InstructionNode* pointer = node;
	while ((*pointer).next != NULL) {
		pointer = (*pointer).next; 
	}

	if (tokenPeek.token_type == DEFAULT) {
		(*pointer).next = ParseDefaultCase();

		pointer = (*pointer).next;
		while ((*pointer).next != NULL) {
			pointer = (*pointer).next; 
		}
	}

	(*pointer).next = noop;

	Expect(RBRACE);

	return node;
}

InstructionNode* syntax_analyzer::Parse_For_Stmt() {
	InstructionNode* comp = new InstructionNode;
	InstructionNode* noop = new InstructionNode;
	InstructionNode* jump = new InstructionNode;

	Expect(FOR);

	Expect(LPAREN);

	InstructionNode* node = Parse_Assign_Stmt();

	Parse_Condition(comp);

	Expect(SEMICOLON);

	InstructionNode* calc = ParseAssignStmt();

	Expect(RPAREN);

	InstructionNode* body = ParseBody();

	(*calc).next = jump;
	(*comp).type = CJMP;
	(*comp).next = body;
	(*comp).cjmp_inst.target = noop;
	(*jump).type = JMP;
	(*jump).jmp_inst.target = comp;
	(*jump).next = noop;
	(*node).next = comp;
	(*noop).type = NOOP;
	(*noop).next = NULL;

	InstructionNode* pointer = (*comp).next;
	while ((*pointer).next != NULL) {
		pointer = (*pointer).next;
	}
	(*pointer).next = calc;

	return node;
}

void syntax_analyzer::Parse_Case_List(InstructionNode* pNode, InstructionNode* pNoop) {
	Token tokenPeek = lexer.peek(1);

	Parse_Case(pNode, pNoop);

	if (tokenPeek.token_type == CASE) {
		InstructionNode* curremtNode = new InstructionNode;

		(*pNode).next = curremtNode;

		(*curremtNode).type = CJMP;
		(*curremtNode).cjmp_inst.operand1_index = (*pNode).cjmp_inst.operand1_index;
		(*curremtNode).cjmp_inst.condition_op = CONDITION_NOTEQUAL;

		ParseCaseList(node, pNoop);
	}
	else { (*pNode).next = NULL; }
}

void syntax_analyzer::Parse_Case(InstructionNode* pNode, InstructionNode* pNoop) {
	Expect(CASE);

	Token token = Expect(NUM);
	int position;

	position = mapCheck(token.lexeme);
	location[token.lexeme] = position;
	mem[position] = std::stoi(token.lexeme);

	(*pNode).cjmp_inst.operand2_index = position;

	Expect(COLON);

	(*pNode).cjmp_inst.target = ParseBody();

	InstructionNode* pointer = (*pNode).cjmp_inst.target;

	while ((*pointer).next != NULL) {
		pointer = (*pointer).next;
	}
	(*pointer).next = pNoop;
}

InstructionNode* syntax_analyzer::Parse_Default_Case() {
	Expect(DEFAULT);
	Expect(COLON);
	InstructionNode* node = Parse_Body();

	return node;
}

void syntax_analyzer::Parse_Num_List() {

	Token token = Expect(NUM);

	inputs.push_back(std::stoi(token.lexeme));

	Token tokenPeek = lexer.peek(1);
	if (tokenPeek.token_type == NUM) {
		Parse_Num_List(); 
	}
}

