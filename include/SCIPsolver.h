#ifndef SCIPSOLVER_H
#define SCIPSOLVER_H

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <map>
#include "lukasol.h"
#include "Formula.h"

class SCIPsolver
{
    public:
        SCIPsolver(std::vector<lukaFormula::Formula> satInstanceInput);
        SCIPsolver(std::vector<lukaFormula::Formula> premInstanceInput,
                   lukaFormula::Formula consInstanceInput);
        ~SCIPsolver();

        InstanceSolution solve();

    protected:

    private:
        std::vector<lukaFormula::Formula> satInstance;
        lukaFormula::Formula consInstance;

        SCIP *scip;
        std::vector<SCIP_VAR*> SCIPvariables;
        std::vector<SCIP_CONS*> SCIPconstraints;
        std::map<unsigned,size_t> SCIPvariablesMap;
        std::map<unsigned,size_t> SCIPnegVariablesMap;

        size_t addSCIPclause(const lukaFormula::Clause& clau);
        size_t addSCIPnegation(size_t a);
        size_t addSCIPdisjunction(size_t a, size_t b);
        size_t addSCIPconjunction(size_t a, size_t b);
        size_t addSCIPequivalence(size_t a, size_t b);
        size_t addSCIPimplication(size_t a, size_t b);
        size_t addSCIPmaximum(size_t a, size_t b);
        size_t addSCIPminimum(size_t a, size_t b);

        size_t setFormulaConstraints(lukaFormula::Formula form);
        void setSCIPcontext();
};

#endif // SCIPSOLVER_H
