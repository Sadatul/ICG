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
    // FILE *lib = fopen("printLibrary.txt", "r");
    // char ch;
    // if (lib != NULL)
    // {
    //     string codeSeg = ".code\nmain proc\n\tmov ax, @data\n\tmov ds, ax\n";

    //     fprintf(ic, "%s", codeSeg.c_str());
    //     string test = "\tmov ax, 10\n\tcall print_output\n\tmov ah, 04ch\n\tint 21h\n\tmain endp\n";
    //     fprintf(ic, "%s", test.c_str());

    //     while ((ch = fgetc(lib)) != EOF)
    //         fputc(ch, ic);
    // }
    // printf("File copied successfully.");

    if (leftPart == "start" && rightPart == "program")
    {
        string header_details = ".model small\n.stack 100h\n";
        string dataSeg = ".data\n\tnumber db \"0000$\"\n";
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
        children->generateCode(ic, level);
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
    if (leftPart == "func_definition" && rightPart == "type_specifier ID LPAREN RPAREN compound_statement")
    {
        printf("INSIDE func_def\n");
        // children->next->next->next->next->generateCode(ic, level);
        fprintf(ic, "%s proc\n", getIthChildren(1)->getName().c_str());
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
        fprintf(ic, "%s endp\n", getIthChildren(1)->getName().c_str());
    }
    if (leftPart == "compound_statement" && rightPart == "LCURL statements RCURL")
    {
        printf("INSIDE compound_statement\n");
        // children->next->generateCode(ic, level);
        getIthChildren(1)->generateCode(ic, level);
    }
    if (leftPart == "statements" && rightPart == "statements statement")
    {
        printf("INSIDE statemets statement\n");
        getIthChildren(0)->generateCode(ic, level);
        getIthChildren(1)->generateCode(ic, level);
    }
    if (leftPart == "expression_statement" && rightPart == "expression SEMICOLON")
    {
        printf("INSIDE statemets statement\n");
        getIthChildren(0)->generateCode(ic, level);
    }
    if (leftPart == "expression" && rightPart == "variable ASSIGNOP logic_expression")
    {
    }
    if (leftPart == "statements" && rightPart == "statement")
    {
        printf("INSIDE statemets\n");
        children->generateCode(ic, level);
    }
    if (leftPart == "statement" && rightPart == "var_declaration")
    {
        printf("INSIDE statemet\n");
        children->generateCode(ic, level);
    }
    if (leftPart == "var_declaration" && rightPart == "type_specifier declaration_list SEMICOLON")
    {
        printf("INSIDE var_declaration %d\n", varDecOffsetList.size());
        for (int i : varDecOffsetList)
        {
            fprintf(ic, "\tsub sp, %d\n", i);
        }
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