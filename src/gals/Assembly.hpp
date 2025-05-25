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
        data.push_back("\t" + id + " : " + value);
    }

    void addData(const string id, const vector<string> valuesArray = vector<string>({"0"}))
    {
        string values = "";
        for (size_t i = 0; i < valuesArray.size(); i++)
        {
            if (i != 0)
                values += ", ";
            values += valuesArray[i];
        }
        data.push_back("\t" + id + " : " + values);
    }

    void addText(const string instruction, const string operand)
    {
        text.push_back("\t" + instruction + " " + operand);
    }

    void addBlankLine()
    {
        text.push_back("");
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