''''
    Part of the code in this file is a derivative of the code found in
    http://github.com/spreto/pwl2limodsat
    which is available under the following license.

    MIT License

    Copyright (c) 2021 Sandro Preto

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
'''

#!/usr/bin/python3

from enum import Enum

import os
import subprocess
import csv
import math
import fractions
import random
import copy
import concurrent.futures

def createPwl(instance_data, pwl_file_name, instance_dimension):
    pwl_file = open(data_folder+pwl_file_name, "w")

    pwl_file.write("pwl\n\n")

    for b in range(len(instance_data[1])):
        pwl_file.write("b ")
        for bc in range(instance_dimension+1):
            pwl_file.write(str(instance_data[1][b][bc]))
            if bc != instance_dimension:
                pwl_file.write(" ")
        pwl_file.write("\n")

    pwl_file.write("\n")

    for p in range(len(instance_data[0])):
        pwl_file.write("p ")
        for c in range(2*(instance_dimension+1)):
            pwl_file.write(str(instance_data[0][p][c]))
            if c != 2*instance_dimension+1:
                pwl_file.write(" ")
        pwl_file.write("\n")
        for b in range(len(instance_data[2][p])):
            pwl_file.write(instance_data[2][p][b][0]+" "+str(instance_data[2][p][b][1]+1)+"\n")
        pwl_file.write("\n")

    pwl_file.close()

class pwlNorm(Enum):
    maxHalf = 1
    minHalf = 2
    wholeUnit = 3

def generatePwl_simp(instance_dimension, regional_parameter, normalization):
    coefficients = []

    for c in range(instance_dimension+1):
        coefficients.append(random.randint(-MAX_INTEGER,MAX_INTEGER))
        coefficients.append(random.randint(1,MAX_INTEGER))

    minimum = 0
    for c in range(4,2*(instance_dimension+1),2):
        if coefficients[c] < 0:
            minimum += coefficients[c]/coefficients[c+1]
    if instance_dimension > 0 and coefficients[2] < 0:
        minimum += (coefficients[2]*(1/regional_parameter)) / coefficients[3]
    minimum += coefficients[0]/coefficients[1]

    globalMinimum = minimum

    if minimum < 0:
        num = 1
        while num/coefficients[1] < abs(minimum):
            num += 1
        coefficients[0] += num
        globalMinimum += num/coefficients[1]

    maximum = 0
    for c in range(4,2*(instance_dimension+1),2):
        if coefficients[c] > 0:
            maximum += coefficients[c]/coefficients[c+1]
    if instance_dimension > 0 and coefficients[2] > 0:
        maximum += (coefficients[2]*(1/regional_parameter)) / coefficients[3]
    maximum += coefficients[0]/coefficients[1]

    globalMaximum = maximum

    if maximum > 1:
        for c in range(0,2*(instance_dimension+1),2):
            coefficients[c+1] *= math.ceil(maximum)
        globalMaximum /= math.ceil(maximum)
        globalMinimum /= math.ceil(maximum)

    boundary_prot = []

    for b in range(regional_parameter+1):
        bound = [0]*(instance_dimension+1)
        bound[0] = -b*(1/regional_parameter)
        bound[1] = 1
        boundary_prot.append(bound)

    for b in range(2,instance_dimension+1):
        bound = [0]*(instance_dimension+1)
        bound[0] = 0
        bound[b] = 1
        boundary_prot.append(bound)
        bound = [0]*(instance_dimension+1)
        bound[0] = -1
        bound[b] = 1
        boundary_prot.append(bound)

    function = []
    function.append(coefficients)

    boundary = []
    bound = []
    bound.append(['g', 0])
    bound.append(['l', 1])
    for b in range(regional_parameter+1, len(boundary_prot), 2):
        bound.append(['g', b])
        bound.append(['l', b+1])
    boundary.append(bound)

    for r in range(1,regional_parameter):
        coefficients = []
        for c in range(2*(instance_dimension+1)):
            coefficients.append(function[len(function)-1][c])

        minimum = 0
        for c in range(4,2*(instance_dimension+1),2):
            if coefficients[c] < 0:
                minimum += coefficients[c]/coefficients[c+1]
        minimum += (coefficients[2]*((r+1)/regional_parameter)) / coefficients[3]
        minimum += coefficients[0]/coefficients[1]

        maximum = 0
        for c in range(4,2*(instance_dimension+1),2):
            if coefficients[c] > 0:
                maximum += coefficients[c]/coefficients[c+1]
        maximum += (coefficients[2]*((r+1)/regional_parameter)) / coefficients[3]
        maximum += coefficients[0]/coefficients[1]

        rand = round(random.uniform(-minimum*regional_parameter, (1-maximum)*regional_parameter),int(math.log10(MAX_INTEGER))+1)
        q = fractions.Fraction.from_float(rand).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))

        aux = fractions.Fraction(coefficients[0],coefficients[1]) + q*fractions.Fraction(-r,regional_parameter)
        coefficients[0] = aux.numerator
        coefficients[1] = aux.denominator

        aux = fractions.Fraction(coefficients[2],coefficients[3]) + q
        coefficients[2] = aux.numerator
        coefficients[3] = aux.denominator

        minimum = 0
        for c in range(4,2*(instance_dimension+1),2):
            if coefficients[c] < 0:
                minimum += coefficients[c]/coefficients[c+1]
        minimum += (coefficients[2]*((r+1)/regional_parameter)) / coefficients[3]
        minimum += coefficients[0]/coefficients[1]

        if minimum < globalMinimum:
            globalMinimum = minimum

        maximum = 0
        for c in range(4,2*(instance_dimension+1),2):
            if coefficients[c] > 0:
                maximum += coefficients[c]/coefficients[c+1]
        maximum += (coefficients[2]*((r+1)/regional_parameter)) / coefficients[3]
        maximum += coefficients[0]/coefficients[1]

        if maximum > globalMaximum:
            globalMaximum = maximum

        function.append(coefficients)

        bound = []
        bound.append(['g', r])
        bound.append(['l', r+1])
        for b in range(regional_parameter+1, len(boundary_prot), 2):
            bound.append(['g', b])
            bound.append(['l', b+1])
        boundary.append(bound)

    fail = 0
    if normalization is pwlNorm.minHalf:
        if globalMaximum - globalMinimum == 0:
            fail = 1
        else:
            if globalMaximum - globalMinimum > 0.25:
                maxAux = fractions.Fraction.from_float(globalMaximum-globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    for c in range(0,2*(instance_dimension+1),2):
                        func[c] = func[c]*maxAux.denominator
                        func[c+1] = 4*func[c+1]*maxAux.numerator
                globalMinimum *= 0.25*((globalMaximum-globalMinimum)**-1)
            if globalMinimum < 0.75:
                minAux = fractions.Fraction.from_float(0.75-globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    aux = fractions.Fraction(func[0], func[1]) + minAux
                    func[0] = aux.numerator
                    func[1] = aux.denominator

    elif normalization is pwlNorm.maxHalf:
        if globalMaximum == 0:
            fail = 1
        else:
            if globalMaximum != 0.5:
                maxAux = fractions.Fraction.from_float(globalMaximum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    for c in range(0,2*(instance_dimension+1),2):
                        func[c] = func[c]*maxAux.denominator
                        func[c+1] = func[c+1]*2*maxAux.numerator

    elif normalization is pwlNorm.wholeUnit:
        if globalMaximum - globalMinimum == 0:
            fail = 1
        else:
            if globalMaximum - globalMinimum != 1:
                maxAux = fractions.Fraction.from_float(globalMaximum-globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    for c in range(0,2*(instance_dimension+1),2):
                        func[c] = func[c]*maxAux.denominator
                        func[c+1] = func[c+1]*maxAux.numerator
                globalMinimum *= (globalMaximum-globalMinimum)**-1
            if globalMinimum != 0:
                minAux = fractions.Fraction.from_float(globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    aux = fractions.Fraction(func[0], func[1]) - minAux
                    func[0] = aux.numerator
                    func[1] = aux.denominator

    if not fail:
        instance_data = []
        instance_data.append(function)
        instance_data.append(boundary_prot)
        instance_data.append(boundary)
        return instance_data
    else:
        return generatePwl_simp(instance_dimension, regional_parameter, normalization)

def generatePwl_cub(instance_dimension, regional_parameter, normalization):
    coefficients = []

    for c in range(instance_dimension+1):
        coefficients.append(random.randint(-MAX_INTEGER,MAX_INTEGER))
        coefficients.append(random.randint(1,MAX_INTEGER))

    minimum = 0
    for c in range(2,2*(instance_dimension+1),2):
        if coefficients[c] < 0:
            minimum += (coefficients[c]*(1/regional_parameter)) / coefficients[c+1]
    minimum += coefficients[0]/coefficients[1]

    if minimum < 0:
        num = 1
        while num/coefficients[1] < abs(minimum):
            num += 1
        coefficients[0] += num

    maximum = 0
    for c in range(2,2*(instance_dimension+1),2):
        if coefficients[c] > 0:
            maximum += (coefficients[c]*(1/regional_parameter)) / coefficients[c+1]
    maximum += coefficients[0]/coefficients[1]

    if maximum > 1:
        for c in range(0,2*(instance_dimension+1),2):
            coefficients[c+1] *= math.ceil(maximum)

    boundary_prot = []

    for b0 in range(1,instance_dimension+1):
        for b in range(regional_parameter+1):
            bound = [0]*(instance_dimension+1)
            bound[0] = -b*(1/regional_parameter)
            bound[b0] = 1
            boundary_prot.append(bound)

    function = []
    function.append(coefficients)

    boundary = []
    bound = []
    for b in range(0, len(boundary_prot), regional_parameter+1):
        bound.append(['g', b])
        bound.append(['l', b+1])
    boundary.append(bound)

    fIndex = []
    fIndex.append([0]*instance_dimension)

    f = [0]*instance_dimension
    f[0] = 1
    allDefined = 0

    if regional_parameter == 1:
        allDefined = 1

    globalMinimum = 0
    globalMaximum = 1

    while not allDefined:
        i = 0
        while f[i] == 0:
            i = i+1
        fAIndex = copy.deepcopy(f)
        fAIndex[i] = fAIndex[i] - 1
        r = i

        for j in range(len(fIndex)):
            if fIndex[j] == fAIndex:
                fA = j

        coefficients = []
        for c in range(2*(instance_dimension+1)):
            coefficients.append(function[fA][c])

        i = i+1
        while i < instance_dimension and f[i] == 0:
            i = i+1

        if i < instance_dimension:
            fBIndex = copy.deepcopy(f)
            fBIndex[i] = fBIndex[i] - 1

            fB = 0
            for j in range(len(fIndex)):
                if fIndex[j] == fBIndex:
                    fB = j

            qAux = (function[fB][2*r+2] / function[fB][2*r+3]) - (function[fA][2*r+2] / function[fA][2*r+3])
            q = fractions.Fraction.from_float(qAux).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))

            aux = fractions.Fraction(coefficients[0],coefficients[1]) + q*fractions.Fraction(-f[r],regional_parameter)
            coefficients[0] = aux.numerator
            coefficients[1] = aux.denominator

            aux = fractions.Fraction(coefficients[2*r+2],coefficients[2*r+3]) + q
            coefficients[2*r+2] = aux.numerator
            coefficients[2*r+3] = aux.denominator
        else:
            minimum = coefficients[0]/coefficients[1]
            for c in range(2,2*(instance_dimension+1),2):
                if coefficients[c] < 0:
                    minimum += ( coefficients[c]*((f[int(c/2)-1]+1)/regional_parameter) ) / coefficients[c+1]
                else:
                    minimum += ( coefficients[c]*((f[int(c/2)-1])/regional_parameter) ) / coefficients[c+1]

            maximum = coefficients[0]/coefficients[1]
            for c in range(2,2*(instance_dimension+1),2):
                if coefficients[c] > 0:
                    maximum += ( coefficients[c]*((f[int(c/2)-1]+1)/regional_parameter) ) / coefficients[c+1]
                else:
                    maximum += ( coefficients[c]*((f[int(c/2)-1])/regional_parameter) ) / coefficients[c+1]

            rand = round(random.uniform(-minimum*regional_parameter, (1-maximum)*regional_parameter),int(math.log10(MAX_INTEGER))+1)
            q = fractions.Fraction.from_float(rand).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))

            aux = fractions.Fraction(coefficients[0],coefficients[1]) + q*fractions.Fraction(-f[r],regional_parameter)
            coefficients[0] = aux.numerator
            coefficients[1] = aux.denominator

            aux = fractions.Fraction(coefficients[2*r+2],coefficients[2*r+3]) + q
            coefficients[2*r+2] = aux.numerator
            coefficients[2*r+3] = aux.denominator

        function.append(coefficients)

        minimum = coefficients[0]/coefficients[1]
        for c in range(2,2*(instance_dimension+1),2):
            if coefficients[c] < 0:
                minimum += ( coefficients[c]*((f[int(c/2)-1]+1)/regional_parameter) ) / coefficients[c+1]
            else:
                minimum += ( coefficients[c]*((f[int(c/2)-1])/regional_parameter) ) / coefficients[c+1]

        if minimum < globalMinimum:
            globalMinimum = minimum

        maximum = coefficients[0]/coefficients[1]
        for c in range(2,2*(instance_dimension+1),2):
            if coefficients[c] > 0:
                maximum += ( coefficients[c]*((f[int(c/2)-1]+1)/regional_parameter) ) / coefficients[c+1]
            else:
                maximum += ( coefficients[c]*((f[int(c/2)-1])/regional_parameter) ) / coefficients[c+1]

        if maximum > globalMaximum:
            globalMaximum = maximum

        bound = []
        for b in range(instance_dimension):
            bound.append(['g', b*(regional_parameter+1)+f[b]])
            bound.append(['l', b*(regional_parameter+1)+f[b]+1])
        boundary.append(bound)

        fIndex.append(copy.deepcopy(f))

        if f == [regional_parameter-1]*instance_dimension:
            allDefined = 1
        else:
            i = 0
            while f[i] == regional_parameter-1:
                f[i] = 0
                i = i+1
            f[i] = f[i] + 1

    if globalMinimum < 0:
        minAux = fractions.Fraction.from_float(abs(globalMinimum)).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
        for func in function:
            aux = fractions.Fraction(func[0], func[1]) + minAux
            func[0] = aux.numerator
            func[1] = aux.denominator
        globalMaximum = globalMaximum + abs(globalMinimum)
        globalMinimum = 0

    fail = 0
    if normalization is pwlNorm.minHalf:
        if globalMaximum - globalMinimum == 0:
            fail = 1
        else:
            if globalMaximum - globalMinimum > 0.25:
                maxAux = fractions.Fraction.from_float(globalMaximum-globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    for c in range(0,2*(instance_dimension+1),2):
                        func[c] = func[c]*maxAux.denominator
                        func[c+1] = 4*func[c+1]*maxAux.numerator
                globalMinimum *= 0.25*((globalMaximum-globalMinimum)**-1)
            if globalMinimum != 0.75:
                minAux = fractions.Fraction.from_float(0.75-globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    aux = fractions.Fraction(func[0], func[1]) + minAux
                    func[0] = aux.numerator
                    func[1] = aux.denominator

    elif normalization is pwlNorm.maxHalf:
        if globalMaximum == 0:
            fail = 1
        else:
            if globalMaximum != 0.5:
                maxAux = fractions.Fraction.from_float(globalMaximum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    for c in range(0,2*(instance_dimension+1),2):
                        func[c] = func[c]*maxAux.denominator
                        func[c+1] = func[c+1]*2*maxAux.numerator

    elif normalization is pwlNorm.wholeUnit:
        if globalMaximum - globalMinimum == 0:
            fail = 1
        else:
            if globalMaximum - globalMinimum != 1:
                maxAux = fractions.Fraction.from_float(globalMaximum-globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    for c in range(0,2*(instance_dimension+1),2):
                        func[c] = func[c]*maxAux.denominator
                        func[c+1] = func[c+1]*maxAux.numerator
                globalMinimum *= (globalMaximum-globalMinimum)**-1
            if globalMinimum != 0:
                minAux = fractions.Fraction.from_float(globalMinimum).limit_denominator(10**int((math.log10(MAX_INTEGER)+1)))
                for func in function:
                    aux = fractions.Fraction(func[0], func[1]) - minAux
                    func[0] = aux.numerator
                    func[1] = aux.denominator

    if not fail:
        instance_data = []
        instance_data.append(function)
        instance_data.append(boundary_prot)
        instance_data.append(boundary)
        return instance_data
    else:
        return generatePwl_cub(instance_dimension, regional_parameter, normalization)

def createReachInstance(pi, pwl_file_name):
    args = [pwl2limodsat_path, data_folder+pwl_file_name]
    subprocess.run(args, capture_output=True, text=False)
    limodsat_file_name = pwl_file_name[:-4]+".limodsat"

    with open(data_folder+limodsat_file_name, "r") as limodsat_file:
        lines = limodsat_file.readlines()

    i = 2
    while lines[i][:4] == "Unit":
        i = i + 1

    newVar = str(int(lines[0][25:])+1)
    lines.insert(i,"Unit "+str(i-1)+" :: Clause      :: "+newVar+"\n")
    lines.insert(i+1,"Unit "+str(i)+" :: Equivalence :: "+str(i-2)+" "+str(i-1)+"\n")

    lines.append("Formula")
    newVar = str(int(lines[0][25:])+2)
    lines.append("Unit 1 :: Clause      :: "+newVar+"\n")
    newUnit = "Unit 2 :: Clause      ::"
    for i in range(pi[1]-1):
        newUnit = newUnit + " " + newVar
    newUnit = newUnit + "\n"
    lines.append(newUnit)
    lines.append("Unit 3 :: Negation    :: 2\n")
    lines.append("Unit 4 :: Equivalence :: 1 3\n")

    lines.append("Formula")
    newUnit = "Unit 1 :: Clause      ::"
    for i in range(pi[0]):
        newUnit = newUnit + " " + newVar
    newUnit = newUnit + "\n"
    lines.append(newUnit)
    newVar = str(int(lines[0][25:])+1)
    lines.append("Unit 2 :: Clause      :: "+newVar+"\n")
    lines.append("Unit 3 :: Implication :: 1 2\n")

    with open(data_folder+limodsat_file_name, "w") as limodsat_file:
        limodsat_file.write("Sat\n\n")
        limodsat_file.write("f:\n")

        for line in lines:
            if line[:4] == "Unit":
                limodsat_file.write(line)
            elif line[:7] == "Formula":
                limodsat_file.write("f:\n")

        limodsat_file.close()

def createRobustInstance(dim, eps, pwl_file_name):
    args = [pwl2limodsat_path, data_folder+pwl_file_name]
    subprocess.run(args, capture_output=True, text=False)
    limodsat_file_name = pwl_file_name[:-4]+".limodsat"

    with open(data_folder+limodsat_file_name, "r") as limodsat_file:
        lines = limodsat_file.readlines()

    i = 2
    while lines[i][:4] == "Unit":
        i = i + 1

    phiVar = str(int(lines[0][25:])+1)
    lines.insert(i,"Unit "+str(i-1)+" :: Clause      :: "+phiVar+"\n")
    lines.insert(i+1,"Unit "+str(i)+" :: Equivalence :: "+str(i-2)+" "+str(i-1)+"\n")

    newLines = ["Formula"]

    for line in lines[2:]:
        cut = line.find("::")
        if line[cut:cut+9] == ":: Clause":
            cut = cut + 18
            newLine = line[:cut]
            variables = line[cut:]
            while len(variables) > 1:
                cut = variables.find(" ") if variables.find(" ") != -1 else len(variables)
                newVar = variables[:cut]
                variables = variables[cut+1:]
                litneg = 1 if int(newVar) > 0 else -1
                newVar = str( int(newVar) + litneg*int(phiVar) )
                newLine = newLine + newVar + " "
            newLines.append(newLine+"\n")
        else:
            newLines.append(line)
    lines = [*lines, *newLines]

    lines.append("Formula")
    epsVar = str(2*int(phiVar)+1)
    if eps != 1:
        lines.append("Unit 1 :: Clause      :: "+epsVar+"\n")
        newUnit = "Unit 2 :: Clause      ::"
        for i in range(eps-1):
            newUnit = newUnit + " " + epsVar
        newUnit = newUnit + "\n"
        lines.append(newUnit)
        lines.append("Unit 3 :: Negation    :: 2\n")
        lines.append("Unit 4 :: Equivalence :: 1 3\n")
    else:
        lines.append("Unit 1 :: Clause      :: "+epsVar+" -"+epsVar+"\n")

    lines.append("Formula")
    fourthVar = str(int(epsVar)+1)
    lines.append("Unit 1 :: Clause      :: "+fourthVar+"\n")
    lines.append("Unit 2 :: Clause      :: "+fourthVar+" "+fourthVar+" "+fourthVar+"\n")
    lines.append("Unit 3 :: Negation    :: 2\n")
    lines.append("Unit 4 :: Equivalence :: 1 3\n")

    newVar = str(int(fourthVar)+1)
    for i in range(1,dim+1):
        lines.append("Formula")
        lines.append("Unit 1 :: Clause      :: "+newVar+"\n")
        lines.append("Unit 2 :: Clause      :: "+epsVar+"\n")
        lines.append("Unit 3 :: Implication :: 1 2\n")
        lines.append("Formula")
        lines.append("Unit 1 :: Clause      :: "+str(i+int(phiVar))+"\n")
        lines.append("Unit 2 :: Clause      :: "+str(i)+" "+str(newVar)+"\n")
        lines.append("Unit 3 :: Equivalence :: 1 2\n")
        lines.append("Unit 4 :: Clause      :: "+str(i)+"\n")
        lines.append("Unit 5 :: Clause      :: "+str(newVar)+"\n")
        lines.append("Unit 6 :: Implication :: 4 5\n")
        lines.append("Unit 7 :: Negation    :: 6\n")
        lines.append("Unit 8 :: Equivalence :: 1 7\n")
        lines.append("Unit 9 :: Maximum     :: 3 8\n")
        newVar = str(int(newVar)+1)

    lines.append("Formula")
    lines.append("Unit 1 :: Clause      :: "+fourthVar+" "+fourthVar+" "+fourthVar+"\n")
    lines.append("Unit 2 :: Clause      :: "+phiVar+"\n")
    lines.append("Unit 3 :: Implication :: 1 2\n")

    lines.append("Consequence")
    lines.append("Unit 1 :: Clause      :: "+fourthVar+" "+fourthVar+"\n")
    lines.append("Unit 2 :: Clause      :: "+str(2*int(phiVar))+"\n")
    lines.append("Unit 3 :: Implication :: 1 2\n")

    with open(data_folder+limodsat_file_name, "w") as limodsat_file:
        limodsat_file.write("Cons\n\n")
        limodsat_file.write("f:\n")

        for line in lines:
            if line[:4] == "Unit":
                limodsat_file.write(line)
            elif line[:7] == "Formula":
                limodsat_file.write("f:\n")
            elif line[:11] == "Consequence":
                limodsat_file.write("C:\n")

        limodsat_file.close()

def runTest(pwl_file_name, dim, param, solver_type):
    limodsat_file_name = pwl_file_name[:-4]+".limodsat"

    if MODE is verificationMode.REACHABILITY:
        createReachInstance(param, pwl_file_name)
        instance_at = 77
        yes_instance = "SAT\n"
        no_instance = "unSAT\n"
    elif MODE is verificationMode.ROBUSTNESS:
        createRobustInstance(dim, param, pwl_file_name)
        instance_at = 83
        yes_instance = "VALID\n"
        no_instance = "INvalid\n"

    stats_row = []
    args = ["time", lukasol_path, solver_type, data_folder+limodsat_file_name]
    out = subprocess.run(args, capture_output=True, text=True)

    if out.stdout[instance_at:] == yes_instance:
        stats_row.append("T")
    elif out.stdout[instance_at:] == no_instance:
        stats_row.append("F")
    else:
        stats_row.append("X")

    end1 = out.stderr.find("u")
    begin2 = out.stderr.find(" ", end1) + 1
    end2 = out.stderr.find("s", begin2)
    try:
        time = float(out.stderr[0:end1]) + float(out.stderr[begin2:end2])
    except ValueError as ve:
        time = ulimit
    stats_row.append(time)

    return stats_row

#   settings   ###################################################
##################################################################
# base_path = "/home/spreto/Code/"
base_path = "/home/sandro_preto/beegfs/"
pwl2limodsat_path = base_path+"pwl2limodsat/bin/Release/pwl2limodsat"
lukasol_path = base_path+"lukasol/bin/Release/lukasol"

class pwlType(Enum):
    SIMPLE = 1
    CUBIC = 2

class verificationMode(Enum):
    REACHABILITY = 1
    ROBUSTNESS = 2

class variation(Enum):
    DIMENSION = 1
    PI = 2

TYPE = pwlType.SIMPLE
MODE = verificationMode.ROBUSTNESS
VARIATION = variation.DIMENSION
SOLVER = "-mip"
DIM_REG_INI = 1
DIM_REG = 7
TESTS_BY_CONFIG = 3
PI = 30
EPS = 1
MAX_INTEGER = 10

# cpu time limit from shell built-in command "ulimit -t"
ulimit = 46080
##################################################################

if TYPE is pwlType.SIMPLE and MODE is verificationMode.REACHABILITY and VARIATION is variation.DIMENSION:
    print("Simple-Region PWL Reachability Benchmark")
    data_folder = base_path+"lukasol/pwlTests/simp-reach_"+str(DIM_REG_INI)+"_"+str(DIM_REG)+"/"
    file_name = "simp-reach"

if TYPE is pwlType.SIMPLE and MODE is verificationMode.REACHABILITY and VARIATION is variation.PI:
    print("Simple-Region PWL Reachability Benchmark (Varying Parameter)")
    data_folder = base_path+"lukasol/pwlTests/simp-reach-varPi"+SOLVER+"_"+str(DIM_REG)+"/"
    file_name = "simp-reach"

elif TYPE is pwlType.CUBIC and MODE is verificationMode.REACHABILITY and VARIATION is variation.DIMENSION:
    print("Cubic-Region PWL Reachability Benchmark")
    data_folder = base_path+"lukasol/pwlTests/cub-reach_"+str(DIM_REG_INI)+"_"+str(DIM_REG)+"/"
    file_name = "cub-reach"

elif TYPE is pwlType.CUBIC and MODE is verificationMode.REACHABILITY and VARIATION is variation.PI:
    print("Cubic-Region PWL Reachability Benchmark (Varying Parameter)")
    data_folder = base_path+"lukasol/pwlTests/cub-reach-varPi"+SOLVER+"_"+str(DIM_REG)+"/"
    file_name = "cub-reach"

elif TYPE is pwlType.SIMPLE and MODE is verificationMode.ROBUSTNESS:
    print("Simple-Region PWL Robustness Benchmark")
    data_folder = base_path+"lukasol/pwlTests/simp-robust_"+str(DIM_REG_INI)+"_"+str(DIM_REG)
    if EPS == 1:
        data_folder += "_F/"
    else:
        data_folder += "_T/"
    file_name = "simp-robust"

elif TYPE is pwlType.CUBIC and MODE is verificationMode.ROBUSTNESS:
    print("Cubic-Region PWL Robustness Benchmark")
    data_folder = base_path+"lukasol/pwlTests/cub-robust_"+str(DIM_REG_INI)+"_"+str(DIM_REG)
    if EPS == 1:
        data_folder += "_F/"
    else:
        data_folder += "_T/"
    file_name = "cub-robust"

if not os.path.isdir(data_folder):
    os.system("mkdir "+data_folder)
else:
    os.system("rm "+data_folder+"*")

if MODE is verificationMode.ROBUSTNESS:
    stats_file = open(data_folder+"stats.csv", "w")
    stats_writer = csv.writer(stats_file)
    stats_writer.writerow(["Dim/Param","Test Num","SMT sol","SMT time","MIP sol","MIP time","Comparison"])

    stats_avg_file = open(data_folder+"stats_avg.csv", "w")
    stats_avg_writer = csv.writer(stats_avg_file)
    stats_avg_row = ["Dimension"]
    stats_avg_row.extend(range(DIM_REG_INI,DIM_REG+1))
    stats_avg_writer.writerow(stats_avg_row)

    stats_avg_row_t1 = ["SMT sol"]
    stats_avg_row_t2 = ["MIP sol"]

    if EPS == 1:
        normalization = pwlNorm.wholeUnit
    else:
        normalization = pwlNorm.minHalf

    for dr in range(DIM_REG_INI,DIM_REG+1):
        avg_t1 = 0
        avg_t2 = 0

        for tn in range(1,TESTS_BY_CONFIG*100+1):
            pwl_file_name = file_name+"_"+str(dr)+"_"+str(tn)+".pwl"

            if TYPE is pwlType.SIMPLE:
                createPwl(generatePwl_simp(dr, dr, normalization), pwl_file_name, dr)
            elif TYPE is pwlType.CUBIC:
                createPwl(generatePwl_cub(dr, 2, normalization), pwl_file_name, dr)

            os.system("cp "+data_folder+pwl_file_name+" "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_2.pwl")
            pwl_file_name = pwl_file_name[:-4]

            stats_row = [dr, tn]

            with concurrent.futures.ThreadPoolExecutor() as executor:
                t1 = executor.submit(runTest, pwl_file_name+".pwl", dr, EPS, "-smt")
                t2 = executor.submit(runTest, pwl_file_name+"_2.pwl", dr, EPS, "-mip")

            os.system("rm "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_2.pwl")
            os.system("rm "+data_folder+"*.limodsat")

            stats_row.extend(t1.result())
            stats_row.extend(t2.result())

            try:
                avg_t1 = avg_t1 + t1.result()[1]
                avg_t2 = avg_t2 + t2.result()[1]
            except Exception:
                pass

            if stats_row[2] == stats_row[4]:
                stats_row.append("eq")
            else:
                stats_row.append("diff")

            stats_writer.writerow(stats_row)

        avg_t1 = avg_t1 / (TESTS_BY_CONFIG*100)
        avg_t2 = avg_t2 / (TESTS_BY_CONFIG*100)
        stats_avg_row_t1.append(avg_t1)
        stats_avg_row_t2.append(avg_t2)

    stats_avg_writer.writerow(stats_avg_row_t1)
    stats_avg_writer.writerow(stats_avg_row_t2)

    stats_file.close()
    stats_avg_file.close()

elif VARIATION is variation.DIMENSION:
    stats_file = open(data_folder+"stats.csv", "w")
    stats_writer = csv.writer(stats_file)
    stats_row = ["Dim/Param","Test Num","SMT sol 1/4","SMT time 1/4","MIP sol 1/4","MIP time 1/4","Comparison 1/4",\
                 "SMT sol 3/4","SMT time 3/4","MIP sol 3/4","MIP time 3/4","Comparison 3/4"]
    stats_writer.writerow(stats_row)

    stats_avg_file = open(data_folder+"stats_avg.csv", "w")
    stats_avg_writer = csv.writer(stats_avg_file)
    stats_avg_row = ["Dimension"]
    stats_avg_row.extend(range(DIM_REG_INI,DIM_REG+1))
    stats_avg_writer.writerow(stats_avg_row)

    stats_avg_row_t1 = ["SMT sol 1/4"]
    stats_avg_row_t2 = ["MIP sol 1/4"]
    stats_avg_row_t3 = ["SMT sol 3/4"]
    stats_avg_row_t4 = ["MIP sol 3/4"]

    for dr in range(DIM_REG_INI,DIM_REG+1):
        avg_t1 = 0
        avg_t2 = 0
        avg_t3 = 0
        avg_t4 = 0

        for tn in range(1,TESTS_BY_CONFIG*100+1):
            pwl_file_name = file_name+"_"+str(dr)+"_"+str(tn)+".pwl"

            if TYPE is pwlType.SIMPLE:
                createPwl(generatePwl_simp(dr, dr, pwlNorm.maxHalf), pwl_file_name, dr)
            elif TYPE is pwlType.CUBIC:
                createPwl(generatePwl_cub(dr, 2, pwlNorm.maxHalf), pwl_file_name, dr)

            os.system("cp "+data_folder+pwl_file_name+" "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_2.pwl")
            os.system("cp "+data_folder+pwl_file_name+" "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_3.pwl")
            os.system("cp "+data_folder+pwl_file_name+" "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_4.pwl")
            pwl_file_name = pwl_file_name[:-4]

            stats_row = [dr, tn]

            with concurrent.futures.ThreadPoolExecutor() as executor:
                t1 = executor.submit(runTest, pwl_file_name+".pwl", dr, [1,4], "-smt")
                t2 = executor.submit(runTest, pwl_file_name+"_2.pwl", dr, [1,4], "-mip")
                t3 = executor.submit(runTest, pwl_file_name+"_3.pwl", dr, [3,4], "-smt")
                t4 = executor.submit(runTest, pwl_file_name+"_4.pwl", dr, [3,4], "-mip")

            os.system("rm "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_2.pwl")
            os.system("rm "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_3.pwl")
            os.system("rm "+data_folder+file_name+"_"+str(dr)+"_"+str(tn)+"_4.pwl")
            os.system("rm "+data_folder+"*.limodsat")

            stats_row.extend(t1.result())
            stats_row.extend(t2.result())

            try:
                avg_t1 = avg_t1 + t1.result()[1]
                avg_t2 = avg_t2 + t2.result()[1]
            except Exception:
                pass

            if stats_row[2] == stats_row[4]:
                stats_row.append("eq")
            else:
                stats_row.append("diff")

            stats_row.extend(t3.result())
            stats_row.extend(t4.result())

            try:
                avg_t3 = avg_t3 + t3.result()[1]
                avg_t4 = avg_t4 + t4.result()[1]
            except Exception:
                pass

            if stats_row[7] == stats_row[9]:
                stats_row.append("eq")
            else:
                stats_row.append("diff")

            stats_writer.writerow(stats_row)

        avg_t1 = avg_t1 / (TESTS_BY_CONFIG*100)
        avg_t2 = avg_t2 / (TESTS_BY_CONFIG*100)
        avg_t3 = avg_t3 / (TESTS_BY_CONFIG*100)
        avg_t4 = avg_t4 / (TESTS_BY_CONFIG*100)
        stats_avg_row_t1.append(avg_t1)
        stats_avg_row_t2.append(avg_t2)
        stats_avg_row_t3.append(avg_t3)
        stats_avg_row_t4.append(avg_t4)

    stats_avg_writer.writerow(stats_avg_row_t1)
    stats_avg_writer.writerow(stats_avg_row_t2)
    stats_avg_writer.writerow(stats_avg_row_t3)
    stats_avg_writer.writerow(stats_avg_row_t4)

    stats_file.close()
    stats_avg_file.close()

elif VARIATION is variation.PI:
    stats_file = open(data_folder+"stats.csv", "w")
    stats_writer = csv.writer(stats_file)
    stats_row = ["Test Num"]
    stats_row.extend(range(1,PI+1))
    stats_row.append("Transition")
    stats_writer.writerow(stats_row)

    def concurrentTests(hund):
        stats = []
        for tn in range(hund*100+1,hund*100+101):
            pwl_file_name = file_name+"_"+str(tn)+".pwl"

            if TYPE is pwlType.SIMPLE:
                createPwl(generatePwl_simp(DIM_REG, DIM_REG, pwlNorm.maxHalf), pwl_file_name, DIM_REG)
            elif TYPE is pwlType.CUBIC:
                createPwl(generatePwl_cub(DIM_REG, 2, pwlNorm.maxHalf), pwl_file_name, DIM_REG)

            transition = 0
            stats_row = [tn]
            for param in range(1,PI+1):
                testRet = runTest(pwl_file_name, DIM_REG, [param, PI], SOLVER)
                stats_row.append(testRet[1])
                if transition == 0 and testRet[0] == "F":
                    transition = param
            stats_row.append(transition)
            stats.append(stats_row)
        return stats

    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = [executor.submit(concurrentTests, t) for t in range(TESTS_BY_CONFIG)]
        for f in futures:
            for line in f.result():
                stats_writer.writerow(line)

    os.system("rm "+data_folder+"*.limodsat")

    stats_file.close()

