
// Duyen Vu

#include <iostream>
#include <unordered_map>
#include "compiler.h"
#include "lexer.h"

using namespace std;

class syntax_analyzer {

public:
	std::unordered_map<std::string, int> location;
	InstructionNode* Parse_Prg();

private:
	LexicalAnalyzer lexer;

	Token MatchToken(TokenType);

	int mapcheck(std::string);
	int Parse_Main();

	InstructionNode* Parse_Body();
	InstructionNode* Parse_Statement_List();
	InstructionNode* Parse_Stmt();
	InstructionNode* Parse_Assign();

	ConditionalOperatorType Parse_Reloop();
	ArithmeticOperatorType Parse_Operation();

	InstructionNode* Parse_Output();
	InstructionNode* Parse_Input();
	InstructionNode* Parse_While();
	InstructionNode* Parse_If();
	InstructionNode* Parse_Switch();
	InstructionNode* Parse_For();
	InstructionNode* Default_Case();

	void Parse_Expr(InstructionNode*);
	void Parse_Case_List(InstructionNode*, InstructionNode*);
	void Parse_Case(InstructionNode*, InstructionNode*);
	void Condition(InstructionNode*);

	void Parse_Var();
	void Parse_Id_List();
	void Parse_Load();
	void Parse_Num_List();
	void Error_Handling();
};

struct InstructionNode* parse_generate_intermediate_representation() {
	InstructionNode* instNode;
	syntax_analyzer val;

	instNode = val.Parse_Prg();
	return instNode;
}

// Checking if the token is of the wanted type
Token syntax_analyzer::MatchToken(TokenType expected_output) {
	Token token = lexer.GetToken();

	if (token.token_type != expected_output) {
		Error_Handling();
	}
	return token;
}


// Getting the second value if the location variable matches the parameter var
int syntax_analyzer::mapcheck(string pVar) {
	for (auto var : location) {
		if (var.first == pVar) {
			return var.second;
		}
	}

	return next_available++;
}

InstructionNode* syntax_analyzer::Parse_Prg() {
	InstructionNode* instNode;

	Parse_Var();
	instNode = Parse_Body();
	Parse_Load();
	MatchToken(END_OF_FILE);

	return instNode;
}

void syntax_analyzer::Parse_Var() {
	Parse_Id_List();

	MatchToken(SEMICOLON);
}

// Add the var and its position to map
void syntax_analyzer::Parse_Id_List() {
	int p;

	Token token = MatchToken(ID);

	p = mapcheck(token.lexeme);
	location[token.lexeme] = p;
	mem[p] = 0;

	token = lexer.GetToken();
	if (token.token_type == COMMA) {
		Parse_Id_List();
	}
	else {
		lexer.UngetToken(1);
	}
}

InstructionNode* syntax_analyzer::Parse_Body() {
	InstructionNode* instNode;

	MatchToken(LBRACE);

	instNode = Parse_Statement_List();

	MatchToken(RBRACE);

	return instNode;
}

InstructionNode* syntax_analyzer::Parse_Statement_List() {
	InstructionNode* newVar;
	InstructionNode* p;

	InstructionNode* instNode;
	instNode = Parse_Stmt();

	Token tokenPeek;
	tokenPeek = lexer.peek(1);

	if (tokenPeek.token_type == ID || tokenPeek.token_type == WHILE || tokenPeek.token_type == IF || tokenPeek.token_type == SWITCH || tokenPeek.token_type == FOR || tokenPeek.token_type == OUTPUT || tokenPeek.token_type == INPUT) {
		newVar = Parse_Statement_List();

		p = instNode;
		while ((*p).next != NULL) {
			p = (*p).next;
		}
		(*p).next = newVar;
	}

	return instNode;
}

InstructionNode* syntax_analyzer::Parse_Stmt() {
	InstructionNode* instNode;
	instNode = NULL;
	Token tokenPeek = lexer.peek(1);


	if (tokenPeek.token_type == ID) {
		instNode = Parse_Assign();
	}
	else if (tokenPeek.token_type == WHILE) {
		instNode = Parse_While();
	}
	else if (tokenPeek.token_type == IF) {
		instNode = Parse_If();
	}
	else if (tokenPeek.token_type == SWITCH) {
		instNode = Parse_Switch();
	}
	else if (tokenPeek.token_type == FOR) {
		instNode = Parse_For();
	}
	else if (tokenPeek.token_type == OUTPUT) {
		instNode = Parse_Output();
	}
	else if (tokenPeek.token_type == INPUT) {
		instNode = Parse_Input();
	}
	return instNode;
}

InstructionNode* syntax_analyzer::Parse_Assign() {
	InstructionNode* instNode = new InstructionNode;

	Token tokenID;
	tokenID = MatchToken(ID);

	(*instNode).type = ASSIGN;
	(*instNode).assign_inst.left_hand_side_index = mapcheck(tokenID.lexeme);
	(*instNode).next = NULL;

	MatchToken(EQUAL);

	Token tokenPeek;
	tokenPeek = lexer.peek(1);

	if (tokenPeek.token_type == ID || tokenPeek.token_type == NUM) {
		tokenPeek = lexer.peek(2);

		if (tokenPeek.token_type == PLUS || tokenPeek.token_type == MINUS || tokenPeek.token_type == MULT || tokenPeek.token_type == DIV) {
			Parse_Expr(instNode);
		}
		else {
			(*instNode).assign_inst.op = OPERATOR_NONE;
			(*instNode).assign_inst.operand1_index = Parse_Main();
		}
	}
	else {
		Error_Handling();
	}

	MatchToken(SEMICOLON);

	return instNode;
}

void syntax_analyzer::Parse_Expr(InstructionNode* parseNode) {

	(*parseNode).assign_inst.operand1_index = Parse_Main();
	(*parseNode).assign_inst.op = Parse_Operation();
	(*parseNode).assign_inst.operand2_index = Parse_Main();
}

int syntax_analyzer::Parse_Main() {
	Token token;
	token = lexer.GetToken();

	int p;
	p = mapcheck(token.lexeme);

	if (token.token_type == NUM) {
		location[token.lexeme] = p;
		mem[p] = std::stoi(token.lexeme);
	}
	else if (token.token_type != ID) {
		Error_Handling();
	}

	return p;
}

ArithmeticOperatorType syntax_analyzer::Parse_Operation() {
	Token token;

	token = lexer.GetToken();
	if (token.token_type == PLUS) {
		return OPERATOR_PLUS;
	}
	else if (token.token_type == MINUS) {
		return OPERATOR_MINUS;
	}
	else if (token.token_type == MULT) {
		return OPERATOR_MULT;
	}
	else if (token.token_type == DIV) {
		return OPERATOR_DIV;
	}
	else {
		Error_Handling();
	}
}

InstructionNode* syntax_analyzer::Parse_Input() {
	InstructionNode* instNode = new InstructionNode;

	MatchToken(INPUT);

	Token token = MatchToken(ID);

	(*instNode).type = IN;
	(*instNode).input_inst.var_index = mapcheck(token.lexeme);
	(*instNode).next = NULL;

	MatchToken(SEMICOLON);

	return instNode;
}

InstructionNode* syntax_analyzer::Parse_Output() {
	InstructionNode* instNode = new InstructionNode;

	MatchToken(OUTPUT);
	Token token = MatchToken(ID);

	(*instNode).type = OUT;
	(*instNode).output_inst.var_index = mapcheck(token.lexeme);
	(*instNode).next = NULL;

	MatchToken(SEMICOLON);

	return instNode;
}

InstructionNode* syntax_analyzer::Parse_While() {
	InstructionNode* instNode = new InstructionNode;
	InstructionNode* instNoop = new InstructionNode;
	InstructionNode* j = new InstructionNode;
	InstructionNode* ptr;

	(*instNode).type = CJMP;
	(*instNode).cjmp_inst.target = instNoop;
	(*j).type = JMP;
	(*j).jmp_inst.target = instNode;
	(*j).next = instNoop;
	(*instNoop).type = NOOP;
	(*instNoop).next = NULL;

	MatchToken(WHILE);
	Condition(instNode);
	(*instNode).next = Parse_Body();

	ptr = instNode;
	while ((*ptr).next != NULL) {
		ptr = (*ptr).next;
	}
	(*ptr).next = j;

	return instNode;
}

InstructionNode* syntax_analyzer::Parse_If() {
	InstructionNode* instNode = new InstructionNode;
	InstructionNode* instNoop = new InstructionNode;
	InstructionNode* ptr;


	MatchToken(IF);
	(*instNode).type = CJMP;
	(*instNode).cjmp_inst.target = instNoop;
	(*instNoop).type = NOOP;
	(*instNoop).next = NULL;

	Condition(instNode);
	(*instNode).next = Parse_Body();

	ptr = instNode;
	while ((*ptr).next != NULL) {
		ptr = (*ptr).next;
	}
	(*ptr).next = instNoop;

	return instNode;
}

void syntax_analyzer::Condition(InstructionNode* pNode) {

	(*pNode).cjmp_inst.operand1_index = Parse_Main();
	(*pNode).cjmp_inst.condition_op = Parse_Reloop();
	(*pNode).cjmp_inst.operand2_index = Parse_Main();
}

ConditionalOperatorType syntax_analyzer::Parse_Reloop() {
	Token token = lexer.GetToken();

	if (token.token_type != GREATER && token.token_type != LESS && token.token_type != NOTEQUAL) {
		Error_Handling();
	}

	if (token.token_type == GREATER) {
		return CONDITION_GREATER;
	}
	else if (token.token_type == LESS) {
		return CONDITION_LESS;
	}
	else if (token.token_type == NOTEQUAL) {
		return CONDITION_NOTEQUAL;
	}
}

InstructionNode* syntax_analyzer::Parse_Switch() {
	InstructionNode* instNode = new InstructionNode;
	InstructionNode* instNoop = new InstructionNode;
	InstructionNode* p;

	MatchToken(SWITCH);
	Token tokenID = MatchToken(ID);

	(*instNode).type = CJMP;
	(*instNode).cjmp_inst.operand1_index = mapcheck(tokenID.lexeme);
	(*instNode).cjmp_inst.condition_op = CONDITION_NOTEQUAL;
	(*instNoop).type = NOOP;
	(*instNoop).next = NULL;

	MatchToken(LBRACE);

	Parse_Case_List(instNode, instNoop);

	p = instNode;
	while ((*p).next != NULL) {
		p = (*p).next;
	}

	Token tokenPeek = lexer.peek(1);
	if (tokenPeek.token_type == DEFAULT) {
		(*p).next = Default_Case();

		p = (*p).next;
		while ((*p).next != NULL) {
			p = (*p).next;
		}
	}

	(*p).next = instNoop;

	MatchToken(RBRACE);

	return instNode;
}

InstructionNode* syntax_analyzer::Parse_For() {
	InstructionNode* comp = new InstructionNode;
	InstructionNode* j = new InstructionNode;
	InstructionNode* instNoop = new InstructionNode;

	MatchToken(FOR);
	MatchToken(LPAREN);

	InstructionNode* instNode = Parse_Assign();
	Condition(comp);

	MatchToken(SEMICOLON);

	InstructionNode* calc = Parse_Assign();

	MatchToken(RPAREN);

	InstructionNode* body = Parse_Body();

	(*calc).next = j;
	(*comp).type = CJMP;
	(*comp).next = body;
	(*comp).cjmp_inst.target = instNoop;
	(*j).type = JMP;
	(*j).jmp_inst.target = comp;
	(*j).next = instNoop;
	(*instNode).next = comp;
	(*instNoop).type = NOOP;
	(*instNoop).next = NULL;

	InstructionNode* ptr = (*comp).next;
	while ((*ptr).next != NULL) {
		ptr = (*ptr).next;
	}
	(*ptr).next = calc;

	return instNode;
}

void syntax_analyzer::Parse_Case_List(InstructionNode* pNode, InstructionNode* pNoop) {
	Token tokenPeek;

	Parse_Case(pNode, pNoop);

	tokenPeek = lexer.peek(1);
	if (tokenPeek.token_type == CASE) {

		InstructionNode* instNode = new InstructionNode;

		(*pNode).next = instNode;

		(*instNode).type = CJMP;
		(*instNode).cjmp_inst.operand1_index = (*pNode).cjmp_inst.operand1_index;
		(*instNode).cjmp_inst.condition_op = CONDITION_NOTEQUAL;

		Parse_Case_List(instNode, pNoop);
	}
	else {
		(*pNode).next = NULL;
	}
}

void syntax_analyzer::Parse_Case(InstructionNode* pNode, InstructionNode* pNoop) {
	InstructionNode* ptr;
	int p;

	MatchToken(CASE);

	Token token = MatchToken(NUM);
	p = mapcheck(token.lexeme);
	location[token.lexeme] = p;
	mem[p] = std::stoi(token.lexeme);

	(*pNode).cjmp_inst.operand2_index = p;

	MatchToken(COLON);

	(*pNode).cjmp_inst.target = Parse_Body();

	ptr = (*pNode).cjmp_inst.target;

	while ((*ptr).next != NULL) {
		ptr = (*ptr).next;
	}

	(*ptr).next = pNoop;
}

void syntax_analyzer::Error_Handling() {
	cout << "SYNTAX ERROR !!!" << endl;
	exit(1);
}

InstructionNode* syntax_analyzer::Default_Case() {
	MatchToken(DEFAULT);
	MatchToken(COLON);

	InstructionNode* instNode = Parse_Body();

	return instNode;
}

void syntax_analyzer::Parse_Load() {
	Parse_Num_List();
}

void syntax_analyzer::Parse_Num_List() {
	Token token = MatchToken(NUM);

	inputs.push_back(std::stoi(token.lexeme));

	Token tokenPeek = lexer.peek(1);
	if (tokenPeek.token_type == NUM) {
		Parse_Num_List();
	}
}
