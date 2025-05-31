#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <vector>
#include <stack>
#include <string>

using namespace std;

class Semantic;

class Assembly
{
private:
    vector<string> data;
    vector<string> text;

    stack<int> tempAccAddressStack = stack<int>({1002}); // Temporary accumulator address stack

    bool isNumber(const string &str, bool allowNegative = true)
    {
        if (str.empty())
            return false;

        size_t start = 0;
        if (allowNegative && str[0] == '-')
            start = 1;

        for (size_t i = start; i < str.size(); i++)
        {
            if (!isdigit(str[i]))
                return false;
        }
        return true;
    }

public:
    string tempValueAddress = "1000";  // Address for the temporary value
    string arrayIndexAddress = "1001"; // Address for the array index

    void addData(const string id, const string value);

    void addData(const string id, const int arrayLength);

    void addText(const string instruction, const string operand);

    void addBlankLine();

    void addComment(const string &comment);

    string generateAssembly();

    string generateAssemblyLabel(const string &id, int scope);

    void emitLoad(SymbolTable &symTable,
                  ExpressionController::ExpressionsEntry &entry,
                  Semantic *semantic,
                  bool shouldLoad);

    void emitUnaryOp(SymbolTable &symTable, ExpressionController::ExpressionsEntry &op, Semantic *semantic);

    void emitBinaryOp(SymbolTable &symTable,
                      const ExpressionController::ExpressionsEntry &op,
                      ExpressionController::ExpressionsEntry &left,
                      ExpressionController::ExpressionsEntry &right,
                      Semantic *semantic);

    void emitArrayAssignment(Semantic *semantic);
};

#endif // ASSEMBLY_H