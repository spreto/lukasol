#include "SCIPsolver.h"

SCIPsolver::SCIPsolver(std::vector<lukaFormula::Formula> satInstanceInput) :
    satInstance(satInstanceInput)
{
    SCIPcreate(&scip);
    SCIPincludeDefaultPlugins(scip);
    SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip), TRUE);
    SCIPcreateProb(scip, "", NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

SCIPsolver::SCIPsolver(std::vector<lukaFormula::Formula> premInstanceInput,
                       lukaFormula::Formula consInstanceInput) :
    SCIPsolver(premInstanceInput)
{
    consInstance = consInstanceInput;
}

SCIPsolver::~SCIPsolver()
{
    for ( SCIP_VAR* var : SCIPvariables )
        SCIPreleaseVar(scip, &var);

    for ( SCIP_CONS* cons : SCIPconstraints )
        SCIPreleaseCons(scip, &cons);

    SCIPfree(&scip);
}

size_t SCIPsolver::addSCIPclause(const lukaFormula::Clause& clau)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_BINARY, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    size_t returnValue = SCIPvariables.size()-1;

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 0,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);

    for ( lukaFormula::Literal lit : clau )
    {
        std::map<unsigned,size_t>::iterator it;
        it = SCIPvariablesMap.find((unsigned) abs(lit));

        if ( it == SCIPvariablesMap.end() )
        {
            SCIPvariables.emplace_back();
            SCIPcreateVar(scip, &SCIPvariables.back(), "",
                          0, 1, 0,
                          SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                          NULL, NULL, NULL, NULL, NULL);
            SCIPaddVar(scip, SCIPvariables.back());

            SCIPvariablesMap[(unsigned) abs(lit)] = SCIPvariables.size()-1;
        }

        if ( lit >= 0 )
        {
            SCIPaddCoefLinear(scip, SCIPconstraints.at(SCIPconstraints.size()-2),
                              SCIPvariables.at(SCIPvariablesMap[(unsigned) abs(lit)]), -1);
            SCIPaddCoefLinear(scip, SCIPconstraints.at(SCIPconstraints.size()-1),
                              SCIPvariables.at(SCIPvariablesMap[(unsigned) abs(lit)]), -1);
        }
        else
        {
            it = SCIPnegVariablesMap.find((unsigned) abs(lit));

            size_t fCons = SCIPconstraints.size()-2;
            size_t sCons = SCIPconstraints.size()-1;

            if ( it == SCIPnegVariablesMap.end() )
            {
                SCIPvariables.emplace_back();
                SCIPcreateVar(scip, &SCIPvariables.back(), "",
                              0, 1, 0,
                              SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                              NULL, NULL, NULL, NULL, NULL);
                SCIPaddVar(scip, SCIPvariables.back());

                SCIPnegVariablesMap[(unsigned) abs(lit)] = SCIPvariables.size()-1;

                SCIPconstraints.emplace_back();
                SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                                     0, NULL, NULL,
                                     1, 1,
                                     TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
                SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariablesMap[(unsigned) abs(lit)]), 1);
                SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPnegVariablesMap[(unsigned) abs(lit)]), 1);
            }

            SCIPaddCoefLinear(scip, SCIPconstraints.at(fCons),
                              SCIPvariables.at(SCIPnegVariablesMap[(unsigned) abs(lit)]), -1);
            SCIPaddCoefLinear(scip, SCIPconstraints.at(sCons),
                              SCIPvariables.at(SCIPnegVariablesMap[(unsigned) abs(lit)]), -1);
        }
    }

    return returnValue;
}

size_t SCIPsolver::addSCIPnegation(size_t a)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         1, 1,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), 1);

    return SCIPvariables.size()-1;
}

size_t SCIPsolver::addSCIPdisjunction(size_t a, size_t b)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_BINARY, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 0,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    return SCIPvariables.size()-1;
}

size_t SCIPsolver::addSCIPconjunction(size_t a, size_t b)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_BINARY, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 1,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -1, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), -1,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    return SCIPvariables.size()-1;
}

size_t SCIPsolver::addSCIPequivalence(size_t a, size_t b)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_BINARY, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 1,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 1,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), 1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         1, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), 2);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -1, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), -2);

    return SCIPvariables.size()-1;
}

size_t SCIPsolver::addSCIPimplication(size_t a, size_t b)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_BINARY, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         1, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 1,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    return SCIPvariables.size()-1;
}

size_t SCIPsolver::addSCIPmaximum(size_t a, size_t b)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_BINARY, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 0,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 1,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), 1);

    return SCIPvariables.size()-1;
}

size_t SCIPsolver::addSCIPminimum(size_t a, size_t b)
{
    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_BINARY, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPvariables.emplace_back();
    SCIPcreateVar(scip, &SCIPvariables.back(), "",
                  0, 1, 0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE,
                  NULL, NULL, NULL, NULL, NULL);
    SCIPaddVar(scip, SCIPvariables.back());

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 0,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -SCIPinfinity(scip), 0,
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         0, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(a), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), 1);

    SCIPconstraints.emplace_back();
    SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                         0, NULL, NULL,
                         -1, SCIPinfinity(scip),
                         TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-1), 1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(b), -1);
    SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(SCIPvariables.size()-2), -1);

    return SCIPvariables.size()-1;
}

size_t SCIPsolver::setFormulaConstraints(lukaFormula::Formula form)
{
    std::vector<size_t> terms;

    unsigned unitClausesCounter = 0;
    unsigned negationsCounter = 0;
    unsigned disjunctionsCounter = 0;
    unsigned conjunctionsCounter = 0;
    unsigned equivalencesCounter = 0;
    unsigned implicationsCounter = 0;
    unsigned maximumsCounter = 0;
    unsigned minimumsCounter = 0;

    if ( form.isEmpty() )
        throw std::invalid_argument("Empty formula.");

    for ( unsigned i = 1; i <= form.getUnitCounter(); i++ )
    {
        if ( (unitClausesCounter < form.getUnitClauses().size()) &&
             (form.getUnitClauses().at(unitClausesCounter).first == i) )
        {
            terms.push_back(addSCIPclause(form.getUnitClauses().at(unitClausesCounter).second));
            unitClausesCounter++;
        }
        else if ( (negationsCounter < form.getNegations().size()) &&
                  (form.getNegations().at(negationsCounter).first == i) )
        {
            terms.push_back(addSCIPnegation(terms.at(form.getNegations().at(negationsCounter).second-1)));
            negationsCounter++;
        }
        else if ( (disjunctionsCounter < form.getLDisjunctions().size()) &&
                  (std::get<0>(form.getLDisjunctions().at(disjunctionsCounter)) == i) )
        {
            terms.push_back(addSCIPdisjunction(terms.at(std::get<1>(form.getLDisjunctions().at(disjunctionsCounter))-1),
                                               terms.at(std::get<2>(form.getLDisjunctions().at(disjunctionsCounter))-1)));
            disjunctionsCounter++;
        }
        else if ( (conjunctionsCounter < form.getLConjunctions().size()) &&
                  (std::get<0>(form.getLConjunctions().at(conjunctionsCounter)) == i) )
        {
            terms.push_back(addSCIPconjunction(terms.at(std::get<1>(form.getLConjunctions().at(conjunctionsCounter))-1),
                                               terms.at(std::get<2>(form.getLConjunctions().at(conjunctionsCounter))-1)));
            conjunctionsCounter++;
        }
        else if ( (equivalencesCounter < form.getEquivalences().size()) &&
                  (std::get<0>(form.getEquivalences().at(equivalencesCounter)) == i) )
        {
            terms.push_back(addSCIPequivalence(terms.at(std::get<1>(form.getEquivalences().at(equivalencesCounter))-1),
                                               terms.at(std::get<2>(form.getEquivalences().at(equivalencesCounter))-1)));
            equivalencesCounter++;
        }
        else if ( (implicationsCounter < form.getImplications().size()) &&
                  (std::get<0>(form.getImplications().at(implicationsCounter)) == i) )
        {
            terms.push_back(addSCIPimplication(terms.at(std::get<1>(form.getImplications().at(implicationsCounter))-1),
                                               terms.at(std::get<2>(form.getImplications().at(implicationsCounter))-1)));
            implicationsCounter++;
        }
        else if ( (maximumsCounter < form.getMaximums().size()) &&
                  (std::get<0>(form.getMaximums().at(maximumsCounter)) == i) )
        {
            terms.push_back(addSCIPmaximum(terms.at(std::get<1>(form.getMaximums().at(maximumsCounter))-1),
                                           terms.at(std::get<2>(form.getMaximums().at(maximumsCounter))-1)));
            maximumsCounter++;
        }
        else if ( (minimumsCounter < form.getMinimums().size()) &&
                  (std::get<0>(form.getMinimums().at(minimumsCounter)) == i) )
        {
            terms.push_back(addSCIPminimum(terms.at(std::get<1>(form.getMinimums().at(minimumsCounter))-1),
                                           terms.at(std::get<2>(form.getMinimums().at(minimumsCounter))-1)));
            minimumsCounter++;
        }
    }

    return terms.back();
}

void SCIPsolver::setSCIPcontext()
{
    for ( lukaFormula::Formula form : satInstance )
    {
        size_t formVarAddress = setFormulaConstraints(form);

        SCIPconstraints.emplace_back();
        SCIPcreateConsLinear(scip, &SCIPconstraints.back(), "",
                             0, NULL, NULL,
                             1, 1,
                             TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
        SCIPaddCoefLinear(scip, SCIPconstraints.back(), SCIPvariables.at(formVarAddress), 1);
    }

    if ( !consInstance.isEmpty() )
    {
        size_t formVarAddress = setFormulaConstraints(consInstance);
        SCIPchgVarObj(scip, SCIPvariables.at(formVarAddress), 1);
    }

    for ( SCIP_CONS* cons : SCIPconstraints )
        SCIPaddCons(scip, cons);
}

InstanceSolution SCIPsolver::solve()
{
    setSCIPcontext();

    if ( consInstance.isEmpty() )
    {
        SCIPsetObjsense(scip, SCIP_OBJSENSE_MAXIMIZE);
        SCIPsolve(scip);
        SCIP_SOL* SCIPsolution = SCIPgetBestSol(scip);

        if ( SCIPsolution == NULL )
            return no;
        else
            return yes;

    }
    else
    {
        SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE);
        SCIPsolve(scip);
        SCIP_SOL* SCIPsolution = SCIPgetBestSol(scip);

        if ( SCIPsolution != NULL )
        {
            if ( SCIPisFeasEQ(scip, SCIPgetSolOrigObj(scip, SCIPsolution), 1) )
                return yes;
            else
                return no;
        }
        else
            return yes;
    }
}
