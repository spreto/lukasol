#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include "Formula.h"

enum InstanceType { SAT, CONS };

class Parser
{
    public:
        Parser(const char* inputFileName);
        virtual ~Parser();

        InstanceType instanceType;

        InstanceType getInstanceType() { return instanceType; }
        std::vector<lukaFormula::Formula> getSatInstance();
        lukaFormula::Formula getConsInstance();

    protected:

    private:
        std::string fileName;
        std::ifstream file;
        std::string currentLine;

        std::vector<lukaFormula::Formula> satInstance;
        lukaFormula::Formula consInstance;
        bool instance = false;

        void nextLine();
        lukaFormula::Formula buildFormula();
        void buildInstance();
};

#endif // PARSER_H
