#ifndef YICESSOLVER_H
#define YICESSOLVER_H

#include <yices.h>
#include <map>
#include "lukasol.h"
#include "Formula.h"

class YicesSolver
{
    public:
        YicesSolver(std::vector<lukaFormula::Formula> satInstanceInput);
        YicesSolver(std::vector<lukaFormula::Formula> premInstanceInput,
                    lukaFormula::Formula consInstanceInput);
        ~YicesSolver();

        InstanceSolution solve();

    protected:

    private:
        std::vector<lukaFormula::Formula> satInstance;
        lukaFormula::Formula consInstance;

        context_t *yicesCtx;
        std::map<unsigned,term_t> yicesVariablesMap;
        std::map<unsigned,term_t> yicesNegVariablesMap;

        term_t addYicesDisjunction(term_t term1, term_t term2);
        term_t addYicesConjunction(term_t term1, term_t term2);
        term_t addYicesClause(const lukaFormula::Clause& clau);
        term_t addYicesNegation(term_t term);
        term_t addYicesEquivalence(term_t term1, term_t term2);
        term_t addYicesImplication(term_t term1, term_t term2);
        term_t addYicesMaximum(term_t term1, term_t term2);
        term_t addYicesMinimum(term_t term1, term_t term2);

        term_t setYicesFormula(const lukaFormula::Formula& form);
        void setYicesContext();
};

#endif // YICESSOLVER_H
