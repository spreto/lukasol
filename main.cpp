#include <iostream>
#include <stdexcept>
#include "lukasol.h"
#include "Parser.h"
#include "YicesSolver.h"
#include "SCIPsolver.h"

enum Solver { SMT, MIP };
std::string inputFileName;

int main(int argc, char **argv)
{
    std::cout << "-: LUKAsiewicz infinitely-valued logic SOLver :-" << std::endl;

    if ( argc > 3 )
        throw std::runtime_error("Invalid arguments.");

    Solver solver = SMT;

    for ( int argNum = 1; argNum < argc; argNum++ )
    {
        std::string arg(argv[argNum]);

        if ( arg.compare("-smt") == 0)
            solver = SMT;
        else if ( arg.compare("-mip") == 0)
            solver = MIP;
        else
            inputFileName = arg;
    }

    Parser parser(inputFileName.c_str());
    InstanceType instanceType = parser.getInstanceType();

    if ( instanceType == SAT )
        std::cout << "Analysing satisfiability... ";
    else
        std::cout << "Analysing consequence validity... ";

    InstanceSolution solution;

    switch ( solver )
    {
        case SMT:
            YicesSolver *yicesSolver;

            if ( instanceType == SAT )
                yicesSolver = new YicesSolver(parser.getSatInstance());
            else
                yicesSolver = new YicesSolver(parser.getSatInstance(), parser.getConsInstance());

            solution = yicesSolver->solve();
            delete yicesSolver;
            break;

        case MIP:
            SCIPsolver *scipSolver;

            if ( instanceType == SAT )
                scipSolver = new SCIPsolver(parser.getSatInstance());
            else
                scipSolver = new SCIPsolver(parser.getSatInstance(), parser.getConsInstance());

            solution = scipSolver->solve();
            delete scipSolver;
            break;
    }

    switch ( instanceType )
    {
        case SAT:
            switch ( solution )
            {
                case yes:
                    std::cout << "SAT" << std::endl;
                    break;
                case no:
                    std::cout << "unSAT" << std::endl;
                    break;
            }
            break;

        case CONS:
            switch ( solution )
            {
                case yes:
                    std::cout << "VALID" << std::endl;
                    break;
                case no:
                    std::cout << "INvalid" << std::endl;
                    break;
            }
            break;
    }

    return 0;
}
