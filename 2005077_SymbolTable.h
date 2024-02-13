#include <iostream>
#include <cstdio>
#include <vector>
using namespace std;

class SymbolInfo;
class LinkedList
{
public:
    SymbolInfo *head;
    SymbolInfo *tail;
    int length;

    LinkedList();
    ~LinkedList();
    void insert(SymbolInfo *s);
    void clear();
    int getLength();
};

class SymbolInfo
{
    string name;
    string type; // For functions return type is saved here..
    int flag;    // 0 for variable, 1 for array, 2 for function,
                 // 3 for ParseTree Memeber. Some parse tree members also need to be identified as
                 // variable, array or function. For that purpose this flag is used.
public:
    SymbolInfo *next;

    LinkedList *params;
    bool isDefined;
    bool isDeclared;

    string dType; // Some symbols like factor, expression requires a extra data type.
                  // This is used for that purpose.
    bool isZero;  // For expressions, factors etc that evaluates to zero

    bool error; // For error recovery. Default value for it is false

    SymbolInfo *children; // For the parse Tree childrens
    string leftPart;      // Parse Tree: Defines the left part of the production rule used
    string rightPart;     // Parse Tree: Defines the right part of the production rule used
    int startLine;        // The starting line
    int endLine;          // The ending line
    bool isLeaf;          // To know if it is leaf or not
    int depth;            // For printing the spaces in the parse Tree

    bool isGlobal; // For detecting global Variables.
    int offset;    // For accessing variables in the stack
                   // compound_statement use this variable to store...
                   // the stack offset that needs tobe added...after the function
                   // ends
                   // variable use this to hold the offset of the corrosponding ID

    string lTrue;                 // Label if condtion is true
    string lFalse;                // Label if condition is false
    string lNext;                 // The next label
    bool isCond;                  // To recognize whether the expression is coming from a condition or a statement
    vector<int> varDecOffsetList; // This array is used to move Sp during var declaration. Only used by var_declaration

    int arraySize; // For array
    SymbolInfo(string name = "", string type = "", SymbolInfo *next = NULL);
    SymbolInfo(string name, string type, int flag);
    ~SymbolInfo();
    static SymbolInfo *getVariableSymbol(string name, string type);
    static SymbolInfo *getArrayTypeSymbol(string name, string type, int arraySize);
    int getFlag();
    void setFlag(int flag);
    string getName();
    void setName(string name);
    string getType();
    void setType(string type);
    SymbolInfo *getNext();
    void setNext(SymbolInfo *next);

    // Static vars for code generation
    static vector<SymbolInfo *> globalVars;
    void generateCode(FILE *ic, int level);
    SymbolInfo *getIthChildren(int i);
    string getRelopTag(string symbol);
};

class ScopeTable
{
private:
    unsigned long long hash(const string &str);
    void deleteRecur(SymbolInfo *node);

    int totalBuckets;
    string id;
    SymbolInfo **table;

    int childNum; // This is used to track the scopeTables childs so that we can
                  // create the id;
public:
    ScopeTable *parentScope;
    int stackOffset;

    ScopeTable(string id, int totalBuckets, ScopeTable *parentScope = NULL);
    ~ScopeTable();
    void childAdded();
    int getChildNum();
    string getId();
    int getTotalBuckets();
    SymbolInfo *lookUp(string name, bool print = false);
    bool insert(string name, string type, int flag, SymbolInfo *data = NULL);
    bool insert(SymbolInfo *sym);
    bool Delete(string name, bool print = false);
    void print();
    void printInFile(FILE *file);
    vector<SymbolInfo *> getTableVars();
};

class SymbolTable
{
    ScopeTable *cur;
    int totalBuckets;

    void deleteRecur(ScopeTable *table);

public:
    SymbolTable(int totalBuckets, bool print = false);
    ~SymbolTable();
    void enterScope(bool print = false);
    void exitScope(bool print = false);
    // Extra data are passed via a SymbolInfo pointer
    bool insert(string name, string type, int flag, SymbolInfo *data = NULL);
    bool insert(SymbolInfo *sym);
    bool remove(string name, bool print = false);
    SymbolInfo *lookUp(string name, bool print = false);
    void printCurScopeTable();
    void printAllScopeTable();
    void printCurScopeTableInFile(FILE *file);
    void printAllScopeTableInFile(FILE *file);

    string getCurId();
    vector<SymbolInfo *> getGlobalVars();
    int getStackOffset();
    void setStackOffset(int offset);
};

class LabelMaker
{
public:
    static int labelCount;
    static string getLable()
    {
        string s = "L" + to_string(labelCount);
        labelCount++;
        return s;
    }
};