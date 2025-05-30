#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <vector>
#include <string>

using namespace std;

class Semantic;

class Assembly
{
private:
    vector<string> data;
    vector<string> text;

    string tempAccAddress = "1000";   // Address for the temporary accumulator
    string tempValueAddress = "1001"; // Address for the temporary value

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
    void addData(const string id, const string value);

    void addData(const string id, const int arrayLength);

    void addText(const string instruction, const string operand);

    void addBlankLine();

    void addComment(const string &comment);

    string generateAssembly();

    string generateAssemblyLabel(const string &id, int scope);

    void emitLoad(SymbolTable &symTable,
                  const ExpressionController::ExpressionsEntry &entry,
                  Semantic *semantic);

    void emitUnaryOp(SymbolTable &symTable,
                     const ExpressionController::ExpressionsEntry &op,
                     const ExpressionController::ExpressionsEntry &operand,
                     bool shouldLoad,
                     Semantic *semantic);

    void emitBinaryOp(SymbolTable &symTable,
                      const ExpressionController::ExpressionsEntry &op,
                      const ExpressionController::ExpressionsEntry &left,
                      const ExpressionController::ExpressionsEntry &right,
                      bool shouldLoad,
                      Semantic *semantic);
};

#endif // ASSEMBLY_H