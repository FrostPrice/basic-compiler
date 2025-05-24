#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <vector>
#include <string>

using namespace std;

class Assembly
{
private:
    vector<string> data;
    vector<string> text;

public:
    void addData(const string id, const string value = "0")
    {
        data.push_back(id + " : " + value);
    }

    void addText(const string instruction, const string operand)
    {
        text.push_back(instruction + " " + operand);
    }

    string generateAssembly()
    {
        string assemblyCode;
        assemblyCode += ".data\n";
        for (const auto &d : data)
        {
            assemblyCode += d + "\n";
        }
        assemblyCode += "\n.text\n";
        for (const auto &t : text)
        {
            assemblyCode += t + "\n";
        }
        return assemblyCode;
    }
};

#endif // ASSEMBLY_H