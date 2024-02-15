#include "2005077_SymbolTable.h"

SymbolInfo::SymbolInfo(string name, string type, SymbolInfo *next)
{
    this->name = name;
    this->type = type;
    this->next = next;
    flag = 3;
    children = NULL;
    isLeaf = false;
    error = false;

    isCond = false;
    lTrue = "";
    lFalse = "";
    lNext = "";
}

SymbolInfo::SymbolInfo(string name, string type, int flag)
{
    this->name = name;
    this->type = type;
    this->flag = flag;
    next = NULL;
    children = NULL;
    if (flag == 2)
    {
        params = new LinkedList();
    }
    isLeaf = false;
    error = false;

    isCond = false;
    lTrue = "";
    lFalse = "";
    lNext = "";
}

SymbolInfo::~SymbolInfo()
{
    if (flag == 2)
    {
        delete params;
    }
}
SymbolInfo *SymbolInfo::getVariableSymbol(string name, string type)
{
    SymbolInfo *tmp = new SymbolInfo(name, type, 0);
    return tmp;
}

SymbolInfo *SymbolInfo::getArrayTypeSymbol(string name, string type, int arraySize)
{
    SymbolInfo *tmp = new SymbolInfo(name, type, 1);
    tmp->arraySize = arraySize;
    return tmp;
}

int SymbolInfo::getFlag()
{
    return flag;
}

void SymbolInfo::setFlag(int flag)
{
    this->flag = flag;
}

string SymbolInfo::getName()
{
    return name;
}

void SymbolInfo::setName(string name)
{
    this->name = name;
}

string SymbolInfo::getType()
{
    return type;
}

void SymbolInfo::setType(string type)
{
    this->type = type;
}

SymbolInfo *SymbolInfo::getNext()
{
    return next;
}

void SymbolInfo::setNext(SymbolInfo *next)
{
    this->next = next;
}

void SymbolInfo::generateCode(FILE *ic, int level)
{
    if (leftPart == "start" && rightPart == "program")
    {
        string header_details = ".model small\n.stack 1000h\n";
        string dataSeg = ".data\n\tnumber db \"00000$\"\n";
        fprintf(ic, "%s", header_details.c_str());
        fprintf(ic, "%s", dataSeg.c_str());
        for (SymbolInfo *i : globalVars)
        {
            if (i->getFlag() == 1)
            {
                fprintf(ic, "\t%s dw %d dup (0000h)\n", i->getName().c_str(), i->arraySize);
            }
            else
            {
                fprintf(ic, "\t%s dw 1 dup (0000h)\n", i->getName().c_str());
            }
            // printf("%s\n", i->getName().c_str());
        }
        fprintf(ic, ".code\n");
        children->generateCode(ic, level);
        FILE *lib = fopen("printLibrary.txt", "r");
        char ch;
        while ((ch = fgetc(lib)) != EOF)
            fputc(ch, ic);

        fprintf(ic, "\nend main\n");
    }
    if (leftPart == "program" && rightPart == "program unit")
    {
        printf("INSIDE PROGRAM\n");
        children->generateCode(ic, level);
        getIthChildren(1)->generateCode(ic, level);
    }
    if (leftPart == "program" && rightPart == "unit")
    {
        printf("INSIDE PROGRAM\n");
        children->generateCode(ic, level);
    }
    if (leftPart == "unit" && rightPart == "func_definition")
    {
        printf("INSIDE unit\n");
        children->generateCode(ic, level);
    }
    if (leftPart == "func_definition" && rightPart == "type_specifier ID LPAREN parameter_list RPAREN compound_statement")
    {
        fprintf(ic, "%s proc\t;Line %d\n", getIthChildren(1)->getName().c_str(), getIthChildren(0)->startLine);
        if (getIthChildren(1)->getName() == "main")
        {
            fprintf(ic, "\tmov ax, @data\n");
            fprintf(ic, "\tmov ds, ax\n");
        }
        fprintf(ic, "\tpush bp\n");
        fprintf(ic, "\tmov bp, sp\n");
        getIthChildren(5)->generateCode(ic, level);
        fprintf(ic, "%s_exit:\n", getIthChildren(1)->getName().c_str());
        if (getIthChildren(1)->getName() == "main")
        {
            fprintf(ic, "\tmov ax, 04ch\n");
            fprintf(ic, "\tint 21h\n");
        }
        fprintf(ic, "\tadd sp, %d\n", getIthChildren(5)->offset);
        fprintf(ic, "\tpop bp\n");
        if (getIthChildren(1)->getName() != "main")
        {
            fprintf(ic, "\tret %d\n", getIthChildren(3)->offset * 2);
        }
        fprintf(ic, "%s endp\n", getIthChildren(1)->getName().c_str());
    }
    if (leftPart == "func_definition" && rightPart == "type_specifier ID LPAREN RPAREN compound_statement")
    {
        printf("INSIDE func_def\n");
        // children->next->next->next->next->generateCode(ic, level);
        fprintf(ic, "%s proc\t;Line %d\n", getIthChildren(1)->getName().c_str(), getIthChildren(0)->startLine);
        if (getIthChildren(1)->getName() == "main")
        {
            fprintf(ic, "\tmov ax, @data\n");
            fprintf(ic, "\tmov ds, ax\n");
        }
        fprintf(ic, "\tpush bp\n");
        fprintf(ic, "\tmov bp, sp\n");
        getIthChildren(4)->generateCode(ic, level);
        fprintf(ic, "%s_exit:\n", getIthChildren(1)->getName().c_str());
        if (getIthChildren(1)->getName() == "main")
        {
            fprintf(ic, "\tmov ax, 04ch\n");
            fprintf(ic, "\tint 21h\n");
        }
        fprintf(ic, "\tadd sp, %d\n", getIthChildren(4)->offset);
        fprintf(ic, "\tpop bp\n");
        if (getIthChildren(1)->getName() != "main")
        {
            fprintf(ic, "\tret\n");
        }
        fprintf(ic, "%s endp\n", getIthChildren(1)->getName().c_str());
    }
    if (leftPart == "compound_statement" && rightPart == "LCURL statements RCURL")
    {
        printf("INSIDE compound_statement\n");
        // children->next->generateCode(ic, level);

        // We have to declare a next, otherwise ":" will be print.
        if (lNext == "")
            lNext = LabelMaker::getLable();
        getIthChildren(1)->lNext = lNext;
        getIthChildren(1)->generateCode(ic, level);
    }
    if (leftPart == "var_declaration" && rightPart == "type_specifier declaration_list SEMICOLON")
    {
        printf("INSIDE var_declaration %d\n", varDecOffsetList.size());
        for (int i : varDecOffsetList)
        {
            fprintf(ic, "\tsub sp, %d\t;Line %d\n", i, startLine);
        }
    }
    if (leftPart == "statements" && rightPart == "statements statement")
    {
        printf("INSIDE statemets statement\n");
        getIthChildren(0)->lNext = LabelMaker::getLable();
        getIthChildren(1)->lNext = lNext;
        getIthChildren(0)->generateCode(ic, level);
        // The label that needs to be printed here will be recursively printed.
        getIthChildren(1)->generateCode(ic, level);
        fprintf(ic, "%s:\t;Line %d\n", lNext.c_str(), startLine);
    }
    if (leftPart == "statements" && rightPart == "statement")
    {
        printf("INSIDE statemets\n");
        children->lNext = lNext;
        children->generateCode(ic, level);
        fprintf(ic, "%s:\t;Line %d\n", lNext.c_str(), startLine);
    }
    if (leftPart == "statement" && rightPart == "var_declaration")
    {
        printf("INSIDE statemet\n");
        children->generateCode(ic, level);
    }
    if (leftPart == "statement" && rightPart == "expression_statement")
    {
        printf("INSIDE statemets expression_statement\n");
        // Not supposed to need lNext here...so not passing
        getIthChildren(0)->generateCode(ic, level);
    }
    if (leftPart == "statement" && rightPart == "compound_statement")
    {
        // BOLD MOVE
        // BOLD MOVE
        // BOLD MOVE
        // BOLD MOVE
        children->lNext = LabelMaker::getLable();
        children->generateCode(ic, level);
    }
    if (leftPart == "statement" && rightPart == "FOR LPAREN expression_statement expression_statement expression RPAREN statement")
    {
        string loopBegin = LabelMaker::getLable();
        string exp3Begin = LabelMaker::getLable();

        SymbolInfo *exp1 = getIthChildren(2);
        SymbolInfo *exp2 = getIthChildren(3);
        SymbolInfo *exp3 = getIthChildren(4);
        SymbolInfo *stat = getIthChildren(6);

        exp2->isCond = true;
        exp2->lTrue = LabelMaker::getLable();
        exp2->lFalse = lNext;
        stat->lNext = exp3Begin;

        exp1->generateCode(ic, level);
        fprintf(ic, "%s:\t;Line %d\n", loopBegin.c_str(), startLine);
        exp2->generateCode(ic, level);
        fprintf(ic, "%s:\t;Line %d\n", exp2->lTrue.c_str(), startLine);
        stat->generateCode(ic, level);
        fprintf(ic, "%s:\t;Line %d\n", exp3Begin.c_str(), startLine);
        exp3->generateCode(ic, level);
        fprintf(ic, "\tjmp %s\t;Line %d\n", loopBegin.c_str(), startLine);
    }
    if (leftPart == "statement" && rightPart == "IF LPAREN expression RPAREN statement")
    {
        SymbolInfo *exp = getIthChildren(2);
        exp->isCond = true;
        exp->lTrue = LabelMaker::getLable();
        exp->lFalse = lNext;
        getIthChildren(4)->lNext = lNext;
        exp->generateCode(ic, level);
        fprintf(ic, "%s:\t;Line %d\n", exp->lTrue.c_str(), startLine);
        getIthChildren(4)->generateCode(ic, level);
    }
    if (leftPart == "statement" && rightPart == "IF LPAREN expression RPAREN statement ELSE statement")
    {
        SymbolInfo *exp = getIthChildren(2);
        SymbolInfo *stat1 = getIthChildren(4);
        SymbolInfo *stat2 = getIthChildren(6);

        exp->isCond = true;
        exp->lTrue = LabelMaker::getLable();
        exp->lFalse = LabelMaker::getLable();

        stat1->lNext = lNext;
        stat2->lNext = lNext;
        exp->generateCode(ic, level);
        fprintf(ic, "%s:\t;Line %d\n", exp->lTrue.c_str(), startLine);
        stat1->generateCode(ic, level);
        fprintf(ic, "\tjmp %s\t;Line %d\n", lNext.c_str(), startLine);
        fprintf(ic, "%s:\t;Line %d\n", exp->lFalse.c_str(), startLine);
        stat2->generateCode(ic, level);
    }
    if (leftPart == "statement" && rightPart == "WHILE LPAREN expression RPAREN statement")
    {
        SymbolInfo *exp = getIthChildren(2);
        SymbolInfo *stat = getIthChildren(4);

        string begin = LabelMaker::getLable();
        exp->isCond = true;
        exp->lTrue = LabelMaker::getLable();
        exp->lFalse = lNext;

        stat->lNext = begin;

        fprintf(ic, "%s: \t;Line %d\n", begin.c_str(), startLine);
        exp->generateCode(ic, level);
        fprintf(ic, "%s: \t;Line %d\n", exp->lTrue.c_str(), startLine);
        stat->generateCode(ic, level);
        fprintf(ic, "\tjmp %s \t;Line %d\n", begin.c_str(), startLine);
    }
    if (leftPart == "statement" && rightPart == "PRINTLN LPAREN ID RPAREN SEMICOLON")
    {
        SymbolInfo *id = getIthChildren(2);
        if (id->isGlobal)
        {
            fprintf(ic, "\tmov ax, %s\n", id->getName().c_str());
            fprintf(ic, "\tcall print_output \t;Line %d\n", startLine);
            fprintf(ic, "\tcall new_line\n");
        }
        else
        {
            fprintf(ic, "\tmov ax, %s\n", getLocalVar(id->offset).c_str());
            fprintf(ic, "\tcall print_output \t;Line %d\n", startLine);
            fprintf(ic, "\tcall new_line\n");
        }
    }
    if (leftPart == "statement" && rightPart == "RETURN expression SEMICOLON")
    {
        getIthChildren(1)->generateCode(ic, level);
        fprintf(ic, "\tmov dx, cx \t;Line %d\n", startLine);
        // function name is being passed via statement
        fprintf(ic, "\tjmp %s_exit \t;Line %d\n", getName().c_str(), startLine);
    }
    if (leftPart == "expression_statement" && rightPart == "expression SEMICOLON")
    {
        printf("INSIDE statemets statement\n");
        getIthChildren(0)->isCond = isCond;
        getIthChildren(0)->lTrue = lTrue;
        getIthChildren(0)->lFalse = lFalse;
        getIthChildren(0)->generateCode(ic, level);
    }
    if (leftPart == "expression" && rightPart == "variable ASSIGNOP logic_expression")
    {
        SymbolInfo *variable = getIthChildren(0);
        getIthChildren(2)->generateCode(ic, level);
        // We evaluate in such a that, we always get the value in CX
        if (variable->rightPart == "ID")
        {
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov %s, cx \t;Line %d\n", variable->children->getName().c_str(), startLine);
            }
            else
            {
                fprintf(ic, "\tmov %s, cx \t;Line %d\n", getLocalVar(variable->children->offset).c_str(), startLine);
            }
        }
        else
        {
            // Variable is an array.
            // variable : ID LSQUARE expression RSQUARE
            // as cx will be overwritten
            fprintf(ic, "\tmov ax, cx \t;Line %d\n", startLine);
            variable->getIthChildren(2)->generateCode(ic, level);
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov bx, cx\n");
                fprintf(ic, "\tadd bx, bx\n");
                fprintf(ic, "\tmov word ptr %s[bx], ax\n", variable->children->getName().c_str());
            }
            else
            {
                fprintf(ic, "\tpush bp\n");
                fprintf(ic, "\tsub bp, %d\n", variable->children->offset);
                fprintf(ic, "\tadd cx, cx\n");
                fprintf(ic, "\tadd bp, cx\n");
                fprintf(ic, "\tmov word ptr [bp], ax\n");
                fprintf(ic, "\tpop bp\n");
            }
            fprintf(ic, "\tmov cx, ax\n");
        }

        if (isCond)
        {
            fprintf(ic, "\tjcxz %s \t;Line %d\n", lFalse.c_str(), startLine);
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "expression" && rightPart == "logic_expression")
    {
        SymbolInfo *child = getIthChildren(0);
        child->isCond = isCond;
        child->lTrue = lTrue;
        child->lFalse = lFalse;
        child->generateCode(ic, level);
    }
    if (leftPart == "logic_expression" && rightPart == "rel_expression")
    {
        SymbolInfo *child = getIthChildren(0);
        child->isCond = isCond;
        child->lTrue = lTrue;
        child->lFalse = lFalse;
        child->generateCode(ic, level);
    }
    if (leftPart == "logic_expression" && rightPart == "rel_expression LOGICOP rel_expression")
    {
        SymbolInfo *rel1 = getIthChildren(0);
        SymbolInfo *rel2 = getIthChildren(2);

        rel1->isCond = true;
        string relop = getIthChildren(1)->getName();
        rel2->isCond = true;

        if (!isCond)
        {
            lTrue = LabelMaker::getLable();
            lFalse = LabelMaker::getLable();
        }
        if (relop == "||")
        {
            rel1->lTrue = lTrue;
            rel1->lFalse = LabelMaker::getLable();
            rel2->lTrue = lTrue;
            rel2->lFalse = lFalse;
        }
        else
        {
            rel1->lFalse = lFalse;
            rel1->lTrue = LabelMaker::getLable();
            rel2->lFalse = lFalse;
            rel2->lTrue = lTrue;
        }
        rel1->generateCode(ic, level);
        if (relop == "||")
        {
            fprintf(ic, "%s: \t;Line %d\n", rel1->lFalse.c_str(), startLine);
        }
        else
        {
            fprintf(ic, "%s: \t;Line %d\n", rel1->lTrue.c_str(), startLine);
        }
        rel2->generateCode(ic, level);

        if (!isCond)
        {
            fprintf(ic, "%s:\n", lTrue.c_str());
            fprintf(ic, "\tmov cx, 1\n");
            string exit = LabelMaker::getLable();
            fprintf(ic, "\tjmp %s\n", exit.c_str());
            fprintf(ic, "%s:\n", lFalse.c_str());
            fprintf(ic, "\tmov cx, 0\n");
            fprintf(ic, "%s:\n", exit.c_str());
        }
    }
    if (leftPart == "rel_expression" && rightPart == "simple_expression")
    {
        SymbolInfo *child = getIthChildren(0);
        child->isCond = isCond;
        child->lTrue = lTrue;
        child->lFalse = lFalse;
        child->generateCode(ic, level);
    }
    if (leftPart == "rel_expression" && rightPart == "simple_expression RELOP simple_expression")
    {
        getIthChildren(0)->generateCode(ic, level);
        fprintf(ic, "\tpush cx \t;Line %d\n", startLine);
        getIthChildren(2)->generateCode(ic, level);
        fprintf(ic, "\tpop ax\n");
        // Value of first simp_exp is in ax and 2nd simp_exp is in cx

        string tag = getRelopTag(getIthChildren(1)->getName());

        if (!isCond)
        {
            // Even if expression is not a condition we will treat it like
            lTrue = LabelMaker::getLable();
            lFalse = LabelMaker::getLable();
        }

        fprintf(ic, "\tcmp ax, cx\n");
        fprintf(ic, "\t%s %s\n", tag.c_str(), lTrue.c_str());
        fprintf(ic, "\tjmp %s\n", lFalse.c_str());

        if (!isCond)
        {
            fprintf(ic, "%s:\n", lTrue.c_str());
            fprintf(ic, "\tmov cx, 1\n");
            string exit = LabelMaker::getLable();
            fprintf(ic, "\tjmp %s\n", exit.c_str());
            fprintf(ic, "%s:\n", lFalse.c_str());
            fprintf(ic, "\tmov cx, 0\n");
            fprintf(ic, "%s:\n", exit.c_str());
        }
    }
    if (leftPart == "simple_expression" && rightPart == "term")
    {
        SymbolInfo *child = getIthChildren(0);
        child->isCond = isCond;
        child->lTrue = lTrue;
        child->lFalse = lFalse;
        child->generateCode(ic, level);
    }
    if (leftPart == "simple_expression" && rightPart == "simple_expression ADDOP term")
    {
        getIthChildren(0)->generateCode(ic, level);
        fprintf(ic, "\tpush cx \t;Line %d\n", startLine);
        getIthChildren(2)->generateCode(ic, level);
        fprintf(ic, "\tpop ax\n");
        // Now we have the value of simple_exp in ax and term in cx

        if (getIthChildren(1)->getName() == "+")
        {
            fprintf(ic, "\tadd cx, ax\n");
        }
        else if (getIthChildren(1)->getName() == "-")
        {
            // Need to move the result to cx after substraction.
            fprintf(ic, "\tsub ax, cx\n\tmov cx, ax\n");
        }
        if (isCond)
        {
            fprintf(ic, "\tjcxz %s\n", lFalse.c_str());
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "term" && rightPart == "unary_expression")
    {
        SymbolInfo *child = getIthChildren(0);
        child->isCond = isCond;
        child->lTrue = lTrue;
        child->lFalse = lFalse;
        child->generateCode(ic, level);
    }
    if (leftPart == "term" && rightPart == "term MULOP unary_expression")
    {
        getIthChildren(0)->generateCode(ic, level);
        fprintf(ic, "\tpush cx \t;Line %d\n", startLine);
        getIthChildren(2)->generateCode(ic, level);
        fprintf(ic, "\tpop ax\n");
        // Now we have the value of term in ax and unary_exp in cx
        if (getIthChildren(1)->getName() == "*")
        {
            fprintf(ic, "\timul cx\n");
            // As we are expecting to get the result in cx we mov the result to cx
            // Here we are only taking lower 2bytes of the 4 byte result
            fprintf(ic, "\tmov cx, ax\n");
        }
        else if (getIthChildren(1)->getName() == "/")
        {
            fprintf(ic, "\tcwd\n");
            fprintf(ic, "\tidiv cx\n");
            // As we are expecting to get the result in cx we mov the result to cx
            fprintf(ic, "\tmov cx, ax\n");
        }
        else if (getIthChildren(1)->getName() == "%")
        {
            fprintf(ic, "\tcwd\n");
            fprintf(ic, "\tidiv cx\n");
            // As we are expecting to get the result in cx we mov the result to cx
            fprintf(ic, "\tmov cx, dx\n");
        }
        if (isCond)
        {
            fprintf(ic, "\tjcxz %s\n", lFalse.c_str());
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "unary_expression" && rightPart == "ADDOP unary_expression")
    {
        SymbolInfo *child = getIthChildren(1);
        child->isCond = isCond;
        child->lTrue = lTrue;
        child->lFalse = lFalse;
        child->generateCode(ic, level);
        if (getIthChildren(0)->getName() == "-")
        {
            fprintf(ic, "\tneg cx \t;Line %d\n", startLine);
        }
    }
    if (leftPart == "unary_expression" && rightPart == "NOT unary_expression")
    {
        SymbolInfo *unExp = getIthChildren(1);
        unExp->isCond = isCond;

        // Borat says very nice
        unExp->lTrue = lFalse;
        unExp->lFalse = lTrue;
        unExp->generateCode(ic, level);
        if (!isCond)
        {
            string toOne = LabelMaker::getLable();
            string toEnd = LabelMaker::getLable();
            fprintf(ic, "\tjcxz %s \t;Line %d\n", toOne.c_str(), startLine);
            fprintf(ic, "\tmov cx, 0\n");
            fprintf(ic, "\tjmp %s\n", toEnd.c_str());
            fprintf(ic, "%s:\n", toOne.c_str());
            fprintf(ic, "\tmov cx, 1\n");
            fprintf(ic, "%s:\n", toEnd.c_str());
        }
    }
    if (leftPart == "unary_expression" && rightPart == "factor")
    {
        SymbolInfo *child = getIthChildren(0);
        child->isCond = isCond;
        child->lTrue = lTrue;
        child->lFalse = lFalse;
        child->generateCode(ic, level);
    }
    if (leftPart == "factor" && rightPart == "variable")
    {
        SymbolInfo *variable = children;
        if (variable->rightPart == "ID")
        {
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov cx, %s \t;Line %d\n", variable->children->getName().c_str(), startLine);
            }
            else
            {
                fprintf(ic, "\tmov cx, %s \t;Line %d\n", getLocalVar(variable->children->offset).c_str(), startLine);
            }
        }
        else
        {
            // Variable is an array.
            // variable : ID LSQUARE expression RSQUARE
            variable->getIthChildren(2)->generateCode(ic, level);
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov bx, cx \t;Line %d\n", startLine);
                fprintf(ic, "\tadd bx, bx\n");
                fprintf(ic, "\tmov cx, word ptr %s[bx]\n", variable->children->getName().c_str());
            }
            else
            {
                fprintf(ic, "\tpush bp \t;Line %d\n", startLine);
                fprintf(ic, "\tsub bp, %d\n", variable->children->offset);
                fprintf(ic, "\tadd cx, cx\n");
                fprintf(ic, "\tadd bp, cx\n");
                fprintf(ic, "\tmov cx, word ptr [bp]\n");
                fprintf(ic, "\tpop bp\n");
            }
        }

        if (isCond)
        {
            fprintf(ic, "\tjcxz %s\n", lFalse.c_str());
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "factor" && rightPart == "ID LPAREN argument_list RPAREN")
    {
        getIthChildren(2)->generateCode(ic, level);
        fprintf(ic, "\tcall %s \t;Line %d\n", getIthChildren(0)->getName().c_str(), startLine);
        fprintf(ic, "\tmov cx, dx\n");
        if (isCond)
        {
            fprintf(ic, "\tjcxz %s\n", lFalse.c_str());
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "factor" && rightPart == "LPAREN expression RPAREN")
    {
        SymbolInfo *exp = getIthChildren(1);
        exp->isCond = isCond;
        exp->lTrue = lTrue;
        exp->lFalse = lFalse;
        exp->generateCode(ic, level);
    }
    if (leftPart == "factor" && rightPart == "CONST_INT")
    {
        fprintf(ic, "\tmov cx, %s \t;Line %d\n", getIthChildren(0)->getName().c_str(), startLine);
        if (isCond)
        {
            fprintf(ic, "\tjcxz %s\n", lFalse.c_str());
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "factor" && rightPart == "variable INCOP")
    {
        SymbolInfo *variable = getIthChildren(0);
        if (variable->rightPart == "ID")
        {
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov cx, %s \t;Line %d\n", variable->children->getName().c_str(), startLine);
            }
            else
            {
                fprintf(ic, "\tmov cx, %s \t;Line %d\n", getLocalVar(variable->children->offset).c_str(), startLine);
            }
        }
        else
        {
            // Variable is an array.
            // variable : ID LSQUARE expression RSQUARE
            variable->getIthChildren(2)->generateCode(ic, level);
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov bx, cx \t;Line %d\n", startLine);
                fprintf(ic, "\tadd bx, bx\n");
                fprintf(ic, "\tmov cx, word ptr %s[bx]\n", variable->children->getName().c_str());
            }
            else
            {
                fprintf(ic, "\tpush bp \t;Line %d\n", startLine);
                fprintf(ic, "\tsub bp, %d\n", variable->children->offset);
                fprintf(ic, "\tadd cx, cx\n");
                fprintf(ic, "\tadd bp, cx\n");
                fprintf(ic, "\tmov cx, word ptr [bp]\n");
                fprintf(ic, "\tpop bp\n");
            }
        }

        // cx must retain the old value
        fprintf(ic, "\tpush cx \t;Line %d\n", startLine);
        fprintf(ic, "\tinc cx \t;Line %d\n", startLine);

        if (variable->rightPart == "ID")
        {
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov %s, cx \t;Line %d\n", variable->children->getName().c_str(), startLine);
            }
            else
            {
                fprintf(ic, "\tmov %s, cx \t;Line %d\n", getLocalVar(variable->children->offset).c_str(), startLine);
            }
        }
        else
        {
            // Variable is an array.
            // variable : ID LSQUARE expression RSQUARE
            // as cx will be overwritten
            fprintf(ic, "\tmov ax, cx \t;Line %d\n", startLine);
            variable->getIthChildren(2)->generateCode(ic, level);
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov bx, cx\n");
                fprintf(ic, "\tadd bx, bx\n");
                fprintf(ic, "\tmov word ptr %s[bx], ax\n", variable->children->getName().c_str());
            }
            else
            {
                fprintf(ic, "\tpush bp\n");
                fprintf(ic, "\tsub bp, %d\n", variable->children->offset);
                fprintf(ic, "\tadd cx, cx\n");
                fprintf(ic, "\tadd bp, cx\n");
                fprintf(ic, "\tmov word ptr [bp], ax\n");
                fprintf(ic, "\tpop bp\n");
            }
            fprintf(ic, "\tmov cx, ax\n");
        }

        fprintf(ic, "\tpop cx\n");
        if (isCond)
        {
            fprintf(ic, "\tjcxz %s\n", lFalse.c_str());
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "factor" && rightPart == "variable DECOP")
    {
        SymbolInfo *variable = getIthChildren(0);
        if (variable->rightPart == "ID")
        {
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov cx, %s \t;Line %d\n", variable->children->getName().c_str(), startLine);
            }
            else
            {
                fprintf(ic, "\tmov cx, %s \t;Line %d\n", getLocalVar(variable->children->offset).c_str(), startLine);
            }
        }
        else
        {
            // Variable is an array.
            // variable : ID LSQUARE expression RSQUARE
            variable->getIthChildren(2)->generateCode(ic, level);
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov bx, cx \t;Line %d\n", startLine);
                fprintf(ic, "\tadd bx, bx\n");
                fprintf(ic, "\tmov cx, word ptr %s[bx]\n", variable->children->getName().c_str());
            }
            else
            {
                fprintf(ic, "\tpush bp \t;Line %d\n", startLine);
                fprintf(ic, "\tsub bp, %d\n", variable->children->offset);
                fprintf(ic, "\tadd cx, cx\n");
                fprintf(ic, "\tadd bp, cx\n");
                fprintf(ic, "\tmov cx, word ptr [bp]\n");
                fprintf(ic, "\tpop bp\n");
            }
        }
        // cx must retain the old value
        fprintf(ic, "\tpush cx \t;Line %d\n", startLine);
        fprintf(ic, "\tdec cx \t;Line %d\n", startLine);

        if (variable->rightPart == "ID")
        {
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov %s, cx \t;Line %d\n", variable->children->getName().c_str(), startLine);
            }
            else
            {
                fprintf(ic, "\tmov %s, cx \t;Line %d\n", getLocalVar(variable->children->offset).c_str(), startLine);
            }
        }
        else
        {
            // Variable is an array.
            // variable : ID LSQUARE expression RSQUARE
            // as cx will be overwritten
            fprintf(ic, "\tmov ax, cx \t;Line %d\n", startLine);
            variable->getIthChildren(2)->generateCode(ic, level);
            if (variable->children->isGlobal)
            {
                fprintf(ic, "\tmov bx, cx\n");
                fprintf(ic, "\tadd bx, bx\n");
                fprintf(ic, "\tmov word ptr %s[bx], ax\n", variable->children->getName().c_str());
            }
            else
            {
                fprintf(ic, "\tpush bp\n");
                fprintf(ic, "\tsub bp, %d\n", variable->children->offset);
                fprintf(ic, "\tadd cx, cx\n");
                fprintf(ic, "\tadd bp, cx\n");
                fprintf(ic, "\tmov word ptr [bp], ax\n");
                fprintf(ic, "\tpop bp\n");
            }
            fprintf(ic, "\tmov cx, ax\n");
        }

        fprintf(ic, "\tpop cx\n");
        if (isCond)
        {
            fprintf(ic, "\tjcxz %s\n", lFalse.c_str());
            fprintf(ic, "\tjmp %s\n", lTrue.c_str());
        }
    }
    if (leftPart == "argument_list" && rightPart == "arguments")
    {
        children->generateCode(ic, level);
    }
    if (leftPart == "arguments" && rightPart == "arguments COMMA logic_expression")
    {
        getIthChildren(2)->generateCode(ic, level);
        fprintf(ic, "\tpush cx \t;Line %d\n", startLine);
        getIthChildren(0)->generateCode(ic, level);
    }
    if (leftPart == "arguments" && rightPart == "logic_expression")
    {
        children->generateCode(ic, level);
        fprintf(ic, "\tpush cx \t;Line %d\n", startLine);
    }
}

SymbolInfo *SymbolInfo::getIthChildren(int i)
{
    SymbolInfo *cur = children;
    if (i < 0)
    {
        printf("Negative index given");
        return NULL;
    }
    while (i != 0)
    {
        cur = cur->next;
        i--;
    }
    return cur;
}

string SymbolInfo::getRelopTag(string symbol)
{
    string tag = "";
    if (symbol == "<")
    {
        tag = "jl";
    }
    else if (symbol == ">")
    {
        tag = "jg";
    }
    else if (symbol == ">=")
    {
        tag = "jge";
    }
    else if (symbol == "<=")
    {
        tag = "jle";
    }
    else if (symbol == "==")
    {
        tag = "je";
    }
    else if (symbol == "!=")
    {
        tag = "jne";
    }

    return tag;
}

string SymbolInfo::getLocalVar(int offset)
{
    if (offset < 0)
    {
        return "word ptr [bp + " + to_string(abs(offset)) + "]";
    }
    else
    {
        return "word ptr [bp - " + to_string(abs(offset)) + "]";
    }
}

unsigned long long ScopeTable::hash(const string &str)
{
    // Using sdbm hash
    unsigned long long hash = 0;
    for (char c : str)
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash % totalBuckets;
}

void ScopeTable::deleteRecur(SymbolInfo *node)
{
    if (node == NULL)
        return;

    deleteRecur(node->getNext());
    delete (node);
}

ScopeTable::ScopeTable(string id, int totalBuckets, ScopeTable *parentScope)
{
    this->id = id;
    this->totalBuckets = totalBuckets;
    table = new SymbolInfo *[totalBuckets];
    for (int i = 0; i < totalBuckets; i++)
    {
        table[i] = NULL;
    }
    this->parentScope = parentScope;
    childNum = 0;
}

ScopeTable::~ScopeTable()
{
    // cout << "\tScopeTable# " << id << " deleted" << endl;
    for (int i = 0; i < totalBuckets; i++)
    {
        deleteRecur(table[i]);
    }

    delete[] table;
}

void ScopeTable::childAdded()
{
    childNum++;
}

int ScopeTable::getChildNum()
{
    return childNum;
}
string ScopeTable::getId()
{
    return id;
}

// void setId(string id){
//     this->id = id;
// }

int ScopeTable::getTotalBuckets()
{
    return totalBuckets;
}

SymbolInfo *ScopeTable::lookUp(string name, bool print)
{
    int index = hash(name); // %totalBuckets is being done inside the hash function
    SymbolInfo *tmp = table[index];
    int i = 1;
    while (tmp)
    {
        if (tmp->getName() == name)
        {
            if (print)
            {
                cout << "\t\'" << name << "\' found at position <" << index + 1 << ", "
                     << i << "> of ScopeTable# " << id << endl;
            }
            return tmp;
        }

        tmp = tmp->getNext();
        i++;
    }

    return NULL;
}

bool ScopeTable::insert(string name, string type, int flag, SymbolInfo *data)
{
    SymbolInfo *newElement = NULL;
    if (flag == 0)
    {
        newElement = SymbolInfo::getVariableSymbol(name, type);
    }
    else if (flag == 1)
    {
        if (data == NULL)
        {
            printf("Error: Array size not given\n");
            return false;
        }
        newElement = SymbolInfo::getArrayTypeSymbol(name, type, data->arraySize);
    }
    else
    {
        printf("Error: Invalid flag\n");
        return false;
    }
    int index = hash(name); // %totalBuckets is being done inside the hash function

    int i = 1;
    if (table[index] == NULL)
    {
        table[index] = newElement;
        return true;
    }

    SymbolInfo *tmp = table[index];
    while (true)
    {
        if (tmp->getName() == name)
        {
            return false;
        }
        if (tmp->getNext() == NULL)
        {
            tmp->setNext(newElement);
            return true;
        }
        tmp = tmp->getNext();
        i++;
    }
}

bool ScopeTable::insert(SymbolInfo *sym)
{

    int index = hash(sym->getName()); // %totalBuckets is being done inside the hash function

    int i = 1;
    if (table[index] == NULL)
    {
        table[index] = sym;
        return true;
    }

    SymbolInfo *tmp = table[index];
    while (true)
    {
        if (tmp->getName() == sym->getName())
        {
            return false;
        }
        if (tmp->getNext() == NULL)
        {
            tmp->setNext(sym);
            return true;
        }
        tmp = tmp->getNext();
        i++;
    }
}

bool ScopeTable::Delete(string name, bool print)
{
    int index = hash(name); // %totalBuckets is being done inside the hash function
    if (table[index] == NULL)
    {
        if (print)
        {
            cout << "\tNot found in the current ScopeTable# " << id << endl;
        }
        return false;
    }

    int i = 1;
    if (table[index]->getName() == name)
    {
        SymbolInfo *tmp = table[index];
        table[index] = tmp->getNext();
        delete tmp;
        if (print)
        {
            cout << "\tDeleted \'" << name << "\' from position <" << index + 1 << ", "
                 << i << "> of ScopeTable# " << id << endl;
        }
        return true;
    }

    SymbolInfo *parent = table[index];

    while (SymbolInfo *child = parent->getNext())
    {
        if (child->getName() == name)
        {
            parent->setNext(child->getNext());
            delete child;
            cout << "\tDeleted \'" << name << "\' from position <" << index + 1 << ", "
                 << ++i << "> of ScopeTable# " << id << endl;
            return true;
        }
        parent = child;
        i++;
    }

    if (print)
    {
        cout << "\tNot found in the current ScopeTable# " << id << endl;
    }
    return false;
}

void ScopeTable::print()
{
    cout << "\tScopeTable# " << id << endl;
    for (int i = 0; i < totalBuckets; i++)
    {
        cout << "\t" << i + 1;

        SymbolInfo *tmp = table[i];
        while (tmp)
        {
            cout << " --> (" << tmp->getName() << "," << tmp->getType() << ")";
            tmp = tmp->getNext();
        }
        cout << endl;
    }
}

void ScopeTable::printInFile(FILE *file)
{
    fprintf(file, "\tScopeTable# %s\n", id.c_str());
    for (int i = 0; i < totalBuckets; i++)
    {
        SymbolInfo *tmp = table[i];
        if (tmp == NULL)
        {
            continue;
        }
        fprintf(file, "\t%d-->", i + 1);
        while (tmp)
        {
            if (tmp->getFlag() == 0)
            {
                fprintf(file, " <%s,%s>", tmp->getName().c_str(), tmp->getType().c_str());
            }
            else if (tmp->getFlag() == 1)
            {
                fprintf(file, " <%s,ARRAY>", tmp->getName().c_str());
            }
            else if (tmp->getFlag() == 2)
            {
                // For debugging params.

                // string s = "";
                // SymbolInfo *tmp1 = tmp->params->head;
                // while (tmp1)
                // {
                //     s += tmp1->getType();
                //     tmp1 = tmp1->next;
                // }
                // fprintf(file, " <%s,FUNCTION%s,%s>", tmp->getName().c_str(),
                //         s.c_str(), tmp->getType().c_str());

                fprintf(file, " <%s,FUNCTION,%s>", tmp->getName().c_str(), tmp->getType().c_str());
            }
            tmp = tmp->getNext();
        }
        fprintf(file, " \n");
    }
}

vector<SymbolInfo *> ScopeTable::getTableVars()
{
    vector<SymbolInfo *> result;
    if (id != "1")
    {
        return result;
    }

    for (int i = 0; i < totalBuckets; i++)
    {
        SymbolInfo *tmp = table[i];
        while (tmp != NULL)
        {
            if (tmp->getFlag() == 0 || tmp->getFlag() == 1)
            {
                result.push_back(tmp);
            }
            tmp = tmp->next;
        }
    }
    return result;
}
void SymbolTable::deleteRecur(ScopeTable *table)
{
    if (table == NULL)
    {
        return;
    }

    deleteRecur(table->parentScope);
    delete table;
}

SymbolTable::SymbolTable(int totalBuckets, bool print)
{
    this->totalBuckets = totalBuckets;
    cur = new ScopeTable("1", totalBuckets);
    if (print)
    {
        cout << "\tScopeTable# 1 created" << endl;
    }
}

SymbolTable::~SymbolTable()
{
    ScopeTable *tmp = cur;
    while (tmp != NULL)
    {
        ScopeTable *tmp1 = tmp->parentScope;
        delete tmp;
        tmp = tmp1;
    }
}

void SymbolTable::enterScope(bool print)
{
    cur->childAdded();
    string id = cur->getId() + "." + to_string(cur->getChildNum());
    cur = new ScopeTable(id, totalBuckets, cur);
    cur->stackOffset = 0;
    if (print)
    {
        cout << "\tScopeTable# " << id << " created" << endl;
    }
}

void SymbolTable::exitScope(bool print)
{
    if (cur->parentScope == NULL)
    {
        if (print)
        {
            cout << "\tScopeTable# 1 cannot be deleted" << endl;
        }
        return;
    }

    // if (print)
    // {
    //     cout << "\tScopeTable# " << cur->getId() << " deleted" << endl;
    // }

    ScopeTable *tmp = cur;
    cur = cur->parentScope;
    delete (tmp);
}

// Extra data are passed via a SymbolInfo pointer
bool SymbolTable::insert(string name, string type, int flag, SymbolInfo *data)
{
    return cur->insert(name, type, flag, data);
}
bool SymbolTable::insert(SymbolInfo *sym)
{
    return cur->insert(sym);
}

bool SymbolTable::remove(string name, bool print)
{
    return cur->Delete(name, print);
}

SymbolInfo *SymbolTable::lookUp(string name, bool print)
{
    ScopeTable *tmp = cur;
    while (tmp)
    {
        SymbolInfo *symbol = tmp->lookUp(name, print);
        if (symbol != NULL)
        {
            return symbol;
        }
        tmp = tmp->parentScope;
    }

    if (print)
    {
        cout << "\t\'" << name << "\' not found in any of the ScopeTables" << endl;
    }
    return NULL;
}

void SymbolTable::printCurScopeTable()
{
    cur->print();
}

void SymbolTable::printAllScopeTable()
{
    ScopeTable *tmp = cur;
    while (tmp)
    {
        tmp->print();
        tmp = tmp->parentScope;
    }
}

void SymbolTable::printCurScopeTableInFile(FILE *file)
{
    cur->printInFile(file);
}

void SymbolTable::printAllScopeTableInFile(FILE *file)
{
    ScopeTable *tmp = cur;
    while (tmp)
    {
        tmp->printInFile(file);
        tmp = tmp->parentScope;
    }
}

vector<SymbolInfo *> SymbolTable::getGlobalVars()
{
    ScopeTable *tmp = cur;
    if (tmp->parentScope != NULL)
    {
        tmp = tmp->parentScope;
    }

    return tmp->getTableVars();
}

string SymbolTable::getCurId()
{
    return cur->getId();
}
int SymbolTable::getStackOffset()
{
    return cur->stackOffset;
};
void SymbolTable::setStackOffset(int offset)
{
    cur->stackOffset = offset;
};

LinkedList::LinkedList()
{
    head = NULL;
    tail = NULL;
    length = 0;
}

LinkedList::~LinkedList()
{
    clear();
}
void LinkedList::insert(SymbolInfo *s)
{
    if (head == NULL)
    {
        head = s;
        tail = s;
    }
    else
    {
        tail->next = s;
        tail = s;
    }
    length++;
}

void LinkedList::clear()
{
    SymbolInfo *temp = head;
    while (temp != NULL)
    {
        SymbolInfo *temp2 = temp;
        temp = temp->next;
        delete temp2;
    }
    head = NULL;
    tail = NULL;
    length = 0;
}

int LinkedList::getLength()
{
    return length;
}