#include <stdexcept>
#include "Parser.h"

Parser::Parser(const char* inputFileName) :
    fileName(inputFileName)
{
    file.open(fileName);

    if ( !file.is_open() )
        throw std::invalid_argument("Unable to open input file.");
    else
    {
        nextLine();
        if ( currentLine.compare(0,3,"Sat") == 0 )
            instanceType = SAT;
        else if ( currentLine.compare(0,4,"Cons") == 0 )
            instanceType = CONS;
        else
            throw std::invalid_argument("Not in standard formula format.");
    }
}

Parser::~Parser()
{
    file.close();
}

void Parser::nextLine()
{
    getline(file,currentLine);

    while ( ( ( currentLine.compare(0,1,"c") == 0 ) || ( currentLine.empty() ) ) && !file.eof() )
        getline(file, currentLine);
}

lukaFormula::Formula Parser::buildFormula()
{
    lukaFormula::UnitIndex unitCounter = 0;
    std::vector<lukaFormula::UnitClause> unitClauses;
    std::vector<lukaFormula::Negation> negations;
    std::vector<lukaFormula::LDisjunction> lDisjunctions;
    std::vector<lukaFormula::LConjunction> lConjunctions;
    std::vector<lukaFormula::Equivalence> equivalences;
    std::vector<lukaFormula::Implication> implications;
    std::vector<lukaFormula::Maximum> maximums;
    std::vector<lukaFormula::Minimum> minimums;

    nextLine();

    while ( currentLine.compare(0,4,"Unit") == 0 )
    {
        unitCounter++;
        if ( stoul(currentLine.substr(5,currentLine.find_first_of(" ",5)-5)) != unitCounter )
            throw std::invalid_argument("Not in standard formula format.");

        size_t beginPosition, endPosition, beginPositionNd, endPositionNd;
        beginPosition = currentLine.find_first_of(" ", 5) + 4;

        if ( currentLine.compare(beginPosition,6,"Clause") == 0 )
        {
            lukaFormula::Clause clau;
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);

            while ( ( beginPosition != std::string::npos + 1 ) && ( beginPosition != currentLine.size() ) )
            {
                clau.push_back(stoi(currentLine.substr(beginPosition, endPosition-beginPosition)));
                beginPosition = endPosition + 1;
                endPosition = currentLine.find_first_of(" ", beginPosition);
            }

            unitClauses.push_back(lukaFormula::UnitClause(unitCounter,clau));
        }
        else if ( currentLine.compare(beginPosition,8,"Negation") == 0 )
        {
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);
            negations.push_back(lukaFormula::Negation( unitCounter,
                                                       (lukaFormula::UnitIndex) stol(currentLine.substr( beginPosition,
                                                                                                         endPosition-beginPosition )) ));
        }
        else if ( currentLine.compare(beginPosition,11,"Disjunction") == 0 )
        {
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);
            beginPositionNd = endPosition + 1;
            endPositionNd = currentLine.find_first_of(" ", beginPositionNd);
            lDisjunctions.push_back(lukaFormula::LDisjunction( unitCounter,
                                                               (lukaFormula::UnitIndex) stol(currentLine.substr( beginPosition,
                                                                                                                 endPosition-beginPosition )),
                                                               (lukaFormula::UnitIndex) stol(currentLine.substr( beginPositionNd,
                                                                                                                 endPositionNd-beginPositionNd )) ));
        }
        else if ( currentLine.compare(beginPosition,11,"Conjunction") == 0 )
        {
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);
            beginPositionNd = endPosition + 1;
            endPositionNd = currentLine.find_first_of(" ", beginPositionNd);
            lConjunctions.push_back(lukaFormula::LConjunction( unitCounter,
                                                               (lukaFormula::UnitIndex) stol(currentLine.substr( beginPosition,
                                                                                                                 endPosition-beginPosition )),
                                                               (lukaFormula::UnitIndex) stol(currentLine.substr( beginPositionNd,
                                                                                                                 endPositionNd-beginPositionNd )) ));
        }
        else if ( currentLine.compare(beginPosition,11,"Equivalence") == 0 )
        {
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);
            beginPositionNd = endPosition + 1;
            endPositionNd = currentLine.find_first_of(" ", beginPositionNd);
            equivalences.push_back(lukaFormula::Equivalence( unitCounter,
                                                             (lukaFormula::UnitIndex) stol(currentLine.substr( beginPosition,
                                                                                                               endPosition-beginPosition )),
                                                             (lukaFormula::UnitIndex) stol(currentLine.substr( beginPositionNd,
                                                                                                               endPositionNd-beginPositionNd )) ));
        }
        else if ( currentLine.compare(beginPosition,11,"Implication") == 0 )
        {
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);
            beginPositionNd = endPosition + 1;
            endPositionNd = currentLine.find_first_of(" ", beginPositionNd);
            implications.push_back(lukaFormula::Implication( unitCounter,
                                                             (lukaFormula::UnitIndex) stol(currentLine.substr( beginPosition,
                                                                                                               endPosition-beginPosition )),
                                                             (lukaFormula::UnitIndex) stol(currentLine.substr( beginPositionNd,
                                                                                                               endPositionNd-beginPositionNd )) ));
        }
        else if ( currentLine.compare(beginPosition,7,"Maximum") == 0 )
        {
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);
            beginPositionNd = endPosition + 1;
            endPositionNd = currentLine.find_first_of(" ", beginPositionNd);
            maximums.push_back(lukaFormula::Maximum( unitCounter,
                                                     (lukaFormula::UnitIndex) stol(currentLine.substr( beginPosition,
                                                                                                       endPosition-beginPosition )),
                                                     (lukaFormula::UnitIndex) stol(currentLine.substr( beginPositionNd,
                                                                                                       endPositionNd-beginPositionNd )) ));
        }
        else if ( currentLine.compare(beginPosition,7,"Minimum") == 0 )
        {
            beginPosition += 15;
            endPosition = currentLine.find_first_of(" ", beginPosition);
            beginPositionNd = endPosition + 1;
            endPositionNd = currentLine.find_first_of(" ", beginPositionNd);
            minimums.push_back(lukaFormula::Minimum( unitCounter,
                                                     (lukaFormula::UnitIndex) stol(currentLine.substr( beginPosition,
                                                                                                       endPosition-beginPosition )),
                                                     (lukaFormula::UnitIndex) stol(currentLine.substr( beginPositionNd,
                                                                                                       endPositionNd-beginPositionNd )) ));
        }
        else
            throw std::invalid_argument("Not in standard formula format.");

        nextLine();
    }

    lukaFormula::Formula formula(unitClauses, negations, lDisjunctions, lConjunctions,
                                 equivalences, implications, maximums, minimums);
    return formula;
}

void Parser::buildInstance()
{
    nextLine();

    if ( instanceType == SAT )
    {
        while ( !file.eof() )
        {
            if ( currentLine.compare(0,2,"f:") != 0 )
                throw std::invalid_argument("Not in standard formula format.");
            else
                satInstance.push_back(buildFormula());
        }
    }
    else if ( instanceType == CONS )
    {
        while ( currentLine.compare(0,2,"f:") == 0 )
            satInstance.push_back(buildFormula());

        if ( currentLine.compare(0,2,"C:") != 0 )
            throw std::invalid_argument("Not in standard formula format.");
        else consInstance = buildFormula();
    }

    instance = true;
}

std::vector<lukaFormula::Formula> Parser::getSatInstance()
{
    if ( !instance )
        buildInstance();

    return satInstance;
}

lukaFormula::Formula Parser::getConsInstance()
{
    if ( instanceType == SAT )
        throw std::domain_error("Not a consequence input file.");
    if ( !instance )
        buildInstance();

    return consInstance;
}
