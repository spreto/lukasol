#include "YicesSolver.h"

YicesSolver::YicesSolver(std::vector<lukaFormula::Formula> satInstanceInput) :
    satInstance(satInstanceInput)
{
    yices_init();
    ctx_config_t *config = yices_new_config();
    yices_default_config_for_logic(config, "QF_LRA");
    yices_set_config(config, "mode", "one-shot");
    yicesCtx = yices_new_context(config);
    yices_free_config(config);
}

YicesSolver::YicesSolver(std::vector<lukaFormula::Formula> premInstanceInput,
                         lukaFormula::Formula consInstanceInput) :
    YicesSolver(premInstanceInput)
{
    consInstance = consInstanceInput;
}

YicesSolver::~YicesSolver()
{
    yices_exit();
}

term_t YicesSolver::addYicesDisjunction(term_t term1, term_t term2)
{
    term_t auxTerm = yices_add(term1, term2);
    return yices_ite(yices_arith_lt_atom(auxTerm, yices_int32(1)), auxTerm, yices_int32(1));
}

term_t YicesSolver::addYicesClause(const lukaFormula::Clause& clau)
{
    std::vector<term_t> terms;

    for ( lukaFormula::Literal lit : clau )
    {
        std::map<unsigned,term_t>::iterator it;
        it = yicesVariablesMap.find((unsigned) abs(lit));

        if ( it == yicesVariablesMap.end() )
        {
            yicesVariablesMap[(unsigned) abs(lit)] = yices_new_uninterpreted_term(yices_real_type());
            term_t interval0 = yices_arith_geq0_atom(yicesVariablesMap[(unsigned) abs(lit)]);
            term_t interval1 = yices_arith_leq_atom(yicesVariablesMap[(unsigned) abs(lit)], yices_int32(1));
            yices_assert_formula(yicesCtx, interval0);
            yices_assert_formula(yicesCtx, interval1);
        }

        if ( lit >= 0 )
            terms.push_back(yicesVariablesMap[(unsigned) lit]);
        else
        {
            it = yicesNegVariablesMap.find((unsigned) abs(lit));

            if ( it == yicesNegVariablesMap.end() )
                yicesNegVariablesMap[(unsigned) abs(lit)] = yices_sub(yices_int32(1), yicesVariablesMap[(unsigned) abs(lit)]);

            terms.push_back(yicesNegVariablesMap[(unsigned) abs(lit)]);
        }
    }

    if (terms.size() > 1)
    {
        term_t term = addYicesDisjunction(terms.at(0), terms.at(1));

        for ( size_t i = 2; i < terms.size(); i++ )
            term = addYicesDisjunction(term, terms.at(i));

        return term;
    }
    else
        return terms.at(0);
}

term_t YicesSolver::addYicesNegation(term_t term)
{
    return yices_sub(yices_int32(1), term);
}

term_t YicesSolver::addYicesConjunction(term_t term1, term_t term2)
{
    term_t auxTerm = yices_sub(yices_add(term1, term2), yices_int32(1));
    return yices_ite(yices_arith_lt_atom(auxTerm, yices_int32(0)), yices_int32(0), auxTerm);
}

term_t YicesSolver::addYicesEquivalence(term_t term1, term_t term2)
{
    return yices_sub(yices_int32(1), yices_abs(yices_sub(term1, term2)));
}

term_t YicesSolver::addYicesImplication(term_t term1, term_t term2)
{
    term_t auxTerm = yices_add(yices_sub(yices_int32(1), term1), term2);
    return yices_ite(yices_arith_lt_atom(auxTerm, yices_int32(1)), auxTerm, yices_int32(1));
}

term_t YicesSolver::addYicesMaximum(term_t term1, term_t term2)
{
    return yices_ite(yices_arith_gt_atom(term1, term2), term1, term2);
}

term_t YicesSolver::addYicesMinimum(term_t term1, term_t term2)
{
    return yices_ite(yices_arith_lt_atom(term1, term2), term1, term2);
}

term_t YicesSolver::setYicesFormula(const lukaFormula::Formula& form)
{
    std::vector<term_t> terms;

    unsigned disjunctionsCounter = 0;
    unsigned unitClausesCounter = 0;
    unsigned negationsCounter = 0;
    unsigned conjunctionsCounter = 0;
    unsigned equivalencesCounter = 0;
    unsigned implicationsCounter = 0;
    unsigned maximumsCounter = 0;
    unsigned minimumsCounter = 0;

    if ( form.isEmpty() )
        throw std::invalid_argument("Empty formula.");

    for ( unsigned i = 1; i <= form.getUnitCounter(); i++ )
    {
        if ( (disjunctionsCounter < form.getLDisjunctions().size()) &&
             (std::get<0>(form.getLDisjunctions().at(disjunctionsCounter)) == i) )
        {
            terms.push_back(addYicesDisjunction(terms.at(std::get<1>(form.getLDisjunctions().at(disjunctionsCounter))-1),
                                                terms.at(std::get<2>(form.getLDisjunctions().at(disjunctionsCounter))-1)));
            disjunctionsCounter++;
        }
        else if ( (unitClausesCounter < form.getUnitClauses().size()) &&
                  (form.getUnitClauses().at(unitClausesCounter).first == i) )
        {
            terms.push_back(addYicesClause(form.getUnitClauses().at(unitClausesCounter).second));
            unitClausesCounter++;
        }
        else if ( (negationsCounter < form.getNegations().size()) &&
                  (form.getNegations().at(negationsCounter).first == i) )
        {
            terms.push_back(addYicesNegation(terms.at(form.getNegations().at(negationsCounter).second-1)));
            negationsCounter++;
        }
        else if ( (conjunctionsCounter < form.getLConjunctions().size()) &&
                  (std::get<0>(form.getLConjunctions().at(conjunctionsCounter)) == i) )
        {
            terms.push_back(addYicesConjunction(terms.at(std::get<1>(form.getLConjunctions().at(conjunctionsCounter))-1),
                                                terms.at(std::get<2>(form.getLConjunctions().at(conjunctionsCounter))-1)));
            conjunctionsCounter++;
        }
        else if ( (equivalencesCounter < form.getEquivalences().size()) &&
                  (std::get<0>(form.getEquivalences().at(equivalencesCounter)) == i) )
        {
            terms.push_back(addYicesEquivalence(terms.at(std::get<1>(form.getEquivalences().at(equivalencesCounter))-1),
                                                terms.at(std::get<2>(form.getEquivalences().at(equivalencesCounter))-1)));
            equivalencesCounter++;
        }
        else if ( (implicationsCounter < form.getImplications().size()) &&
                  (std::get<0>(form.getImplications().at(implicationsCounter)) == i) )
        {
            terms.push_back(addYicesImplication(terms.at(std::get<1>(form.getImplications().at(implicationsCounter))-1),
                                                terms.at(std::get<2>(form.getImplications().at(implicationsCounter))-1)));
            implicationsCounter++;
        }
        else if ( (maximumsCounter < form.getMaximums().size()) &&
                  (std::get<0>(form.getMaximums().at(maximumsCounter)) == i) )
        {
            terms.push_back(addYicesMaximum(terms.at(std::get<1>(form.getMaximums().at(maximumsCounter))-1),
                                            terms.at(std::get<2>(form.getMaximums().at(maximumsCounter))-1)));
            maximumsCounter++;
        }
        else if ( (minimumsCounter < form.getMinimums().size()) &&
                  (std::get<0>(form.getMinimums().at(minimumsCounter)) == i) )
        {
            terms.push_back(addYicesMinimum(terms.at(std::get<1>(form.getMinimums().at(minimumsCounter))-1),
                                            terms.at(std::get<2>(form.getMinimums().at(minimumsCounter))-1)));
            minimumsCounter++;
        }
    }

    return terms.back();
}

void YicesSolver::setYicesContext()
{
    for ( lukaFormula::Formula form : satInstance )
        yices_assert_formula(yicesCtx, yices_arith_eq_atom(setYicesFormula(form), yices_int32(1)));

    if ( !consInstance.isEmpty() )
        yices_assert_formula(yicesCtx, yices_arith_lt_atom(setYicesFormula(consInstance), yices_int32(1)));
}

InstanceSolution YicesSolver::solve()
{
    setYicesContext();
    smt_status_t yicesSolution = yices_check_context(yicesCtx, NULL);

    if ( consInstance.isEmpty() )
    {
        if ( yicesSolution == STATUS_SAT )
            return yes;
        else if ( yicesSolution == STATUS_UNSAT )
            return no;
        else
            throw std::runtime_error("Yices could not handle the instance.");
    }
    else
    {
        if ( yicesSolution == STATUS_SAT )
            return no;
        else if ( yicesSolution == STATUS_UNSAT )
            return yes;
        else
            throw std::runtime_error("Yices could not handle the instance.");
    }
}
