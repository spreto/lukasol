#!/usr/bin/python3

from enum import Enum

import os
import subprocess
import csv
import math
import fractions
import random
import copy

def createRothenberg(n, mode):
    rothenberg_file = open(data_folder+"rothenberg"+mode+"_"+str(n)+".txt", "w")
    rothenberg_file.write("Cons\n\n")
    rothenberg_file.write("C:\n")

    rothenberg_file.write("Unit 1 :: Clause      :: 1\n")
    unit = 2
    unitClause1 = 1

    for i in range(n-1):
        rothenberg_file.write("Unit "+str(unit)+" :: Conjunction :: "+str(unitClause1)+" "+str(unit-1)+"\n")
        unit = unit + 1
    conjunction1 = unit - 1

    rothenberg_file.write("Unit "+str(unit)+" :: Clause      :: 2\n")
    unitClause2 = unit
    unit = unit + 1

    for i in range(n-1):
        rothenberg_file.write("Unit "+str(unit)+" :: Conjunction :: "+str(unitClause2)+" "+str(unit-1)+"\n")
        unit = unit + 1
    conjunction2 = unit - 1

    rothenberg_file.write("Unit "+str(unit)+" :: Maximum     :: "+str(conjunction1)+" "+str(conjunction2)+"\n")
    maximum1 = unit
    unit = unit + 1
    rothenberg_file.write("Unit "+str(unit)+" :: Maximum     :: "+str(unitClause1)+" "+str(unitClause2)+"\n")
    maximum2 = unit
    unit = unit + 1

    for i in range(n-1):
        rothenberg_file.write("Unit "+str(unit)+" :: Conjunction :: "+str(maximum2)+" "+str(unit-1)+"\n")
        unit = unit + 1

    if mode == "A":
        rothenberg_file.write("Unit "+str(unit)+" :: Implication :: "+str(maximum1)+" "+str(unit-1)+"\n")
    elif mode == "B":
        rothenberg_file.write("Unit "+str(unit)+" :: Implication :: "+str(unit-1)+" "+str(maximum1)+"\n")

    rothenberg_file.close()

def testRunnerRothenberg(n, mode):
    csv_file = open(data_folder+"rothenberg"+mode+"-stats.csv", "w")
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(["n","SMT sol","SMT time","MIP sol","MIP time"])

    for i in range(2,n+1):
        csv_row = [str(i)]
        createRothenberg(i, mode)

        args = ["time", lukasol_path, "-smt", data_folder+"rothenberg"+mode+"_"+str(i)+".txt"]
        out = subprocess.run(args, capture_output=True, text=True)

        if out.stdout[83:] == "VALID\n":
            csv_row.append("T")
        elif out.stdout[83:] == "INvalid\n":
            csv_row.append("F")

        time = float(out.stderr[0:4]) + float(out.stderr[9:13])
        csv_row.append(time)

        args[2] = "-mip"
        out = subprocess.run(args, capture_output=True, text=True)

        if out.stdout[83:] == "VALID\n":
            csv_row.append("T")
        elif out.stdout[83:] == "INvalid\n":
            csv_row.append("F")

        time = float(out.stderr[0:4]) + float(out.stderr[9:13])
        csv_row.append(time)

        csv_writer.writerow(csv_row)

    csv_file.close()

def createBofill(varNum, clausesNum, fileName):
    bofill_file = open(data_folder+fileName, "w")
    bofill_file.write("Sat\n\n")

    for cl in range(clausesNum):
        bofill_file.write("f:\n")

        var1 = random.randrange(1, varNum)
        var2 = random.randrange(1, varNum)
        var3 = random.randrange(1, varNum)
        clType = random.randrange(1, 11)

        if clType == 1:
            bofill_file.write("Unit 1 :: Clause      :: "+str(var1)+" "+str(var2)+" "+str(var3)+"\n")
        elif clType == 2:
            bofill_file.write("Unit 1 :: Clause      :: -"+str(var1)+" "+str(var2)+" "+str(var3)+"\n")
        elif clType == 3:
            bofill_file.write("Unit 1 :: Clause      :: "+str(var1)+" -"+str(var2)+" "+str(var3)+"\n")
        elif clType == 4:
            bofill_file.write("Unit 1 :: Clause      :: "+str(var1)+" "+str(var2)+" -"+str(var3)+"\n")
        elif clType == 5:
            bofill_file.write("Unit 1 :: Clause      :: -"+str(var1)+" -"+str(var2)+" "+str(var3)+"\n")
        elif clType == 6:
            bofill_file.write("Unit 1 :: Clause      :: -"+str(var1)+" "+str(var2)+" -"+str(var3)+"\n")
        elif clType == 7:
            bofill_file.write("Unit 1 :: Clause      :: "+str(var1)+" -"+str(var2)+" -"+str(var3)+"\n")
        elif clType == 8:
            bofill_file.write("Unit 1 :: Clause      :: -"+str(var1)+" -"+str(var2)+" -"+str(var3)+"\n")
        elif clType == 9:
            bofill_file.write("Unit 1 :: Clause      :: "+str(var1)+" "+str(var2)+"\n")
            bofill_file.write("Unit 2 :: Negation    :: 1\n")
            bofill_file.write("Unit 3 :: Clause      :: "+str(var3)+"\n")
            bofill_file.write("Unit 4 :: Disjunction :: 2 3\n")
        elif clType == 10:
            bofill_file.write("Unit 1 :: Clause      :: "+str(var1)+" "+str(var3)+"\n")
            bofill_file.write("Unit 2 :: Negation    :: 1\n")
            bofill_file.write("Unit 3 :: Clause      :: "+str(var2)+"\n")
            bofill_file.write("Unit 4 :: Disjunction :: 2 3\n")
        elif clType == 11:
            bofill_file.write("Unit 1 :: Clause      :: "+str(var1)+"\n")
            bofill_file.write("Unit 2 :: Clause      :: "+str(var2)+" "+str(var3)+"\n")
            bofill_file.write("Unit 3 :: Negation    :: 2\n")
            bofill_file.write("Unit 4 :: Disjunction :: 1 3\n")

        bofill_file.write("\n")

    bofill_file.close()

def testRunnerBofill(varnum, clmax, clstep, tnum):
    stats_file = open(data_folder+"stats.csv", "w")
    stats_writer = csv.writer(stats_file)
    stats_writer.writerow(["Clauses Num","Test Num","SMT sol","SMT time","MIP sol","MIP time","Comparison"])

    avg_file = open(data_folder+"avg.csv", "w")
    avg_writer = csv.writer(avg_file)
    avg_writer.writerow(["Clauses Num","SMT true","SMT time","MIP true","MIP time"])

    for cl in range(clstep, clmax+1, clstep):
        avg_row = [cl,0,0,0,0]

        for tn in range(1, tnum+1):
            fileName = "bofill_"+str(cl)+"_"+str(tn)+".txt"
            createBofill(varnum, cl, fileName)

            stats_row = [cl, tn]
            args = ["time", lukasol_path, "-smt", data_folder+fileName]
            out = subprocess.run(args, capture_output=True, text=True)

            if out.stdout[77:] == "SAT\n":
                stats_row.append("T")
                avg_row[1] = avg_row[1] + 1
            elif out.stdout[77:] == "unSAT\n":
                stats_row.append("F")
            else:
                stats_row.append("X")

            end1 = out.stderr.find("u")
            begin2 = out.stderr.find(" ", end1) + 1
            end2 = out.stderr.find("s", begin2)
            try:
                time = float(out.stderr[0:end1]) + float(out.stderr[begin2:end2])
            except ValueError as ve:
                time = "-"
            stats_row.append(time)
            avg_row[2] = avg_row[2] + time

            args[2] = "-mip"
            out = subprocess.run(args, capture_output=True, text=True)

            if out.stdout[77:] == "SAT\n":
                stats_row.append("T")
                avg_row[3] = avg_row[3] + 1
            elif out.stdout[77:] == "unSAT\n":
                stats_row.append("F")
            else:
                stats_row.append("X")

            end1 = out.stderr.find("u")
            begin2 = out.stderr.find(" ", end1) + 1
            end2 = out.stderr.find("s", begin2)
            try:
                time = float(out.stderr[0:end1]) + float(out.stderr[begin2:end2])
            except ValueError as ve:
                time = "-"
            stats_row.append(time)
            avg_row[4] = avg_row[4] + time

            if stats_row[2] == stats_row[4]:
                stats_row.append("eq")
            else:
                stats_row.append("diff")

            stats_writer.writerow(stats_row)

        avg_row[1] = avg_row[1] / tnum
        avg_row[2] = avg_row[2] / tnum
        avg_row[3] = avg_row[3] / tnum
        avg_row[4] = avg_row[4] / tnum

        avg_writer.writerow(avg_row)

    stats_file.close()
    avg_file.close()

#   settings   ###################################################
##################################################################
base_path = "/home/spreto/Code/lukasol/"
# base_path = "/home/sandro_preto/beegfs/lukasol/"
lukasol_path = base_path+"bin/Release/lukasol"

class TestMode(Enum):
    ROTHENBERG_A = 1
    ROTHENBERG_B = 2
    BOFILL = 3

TEST_MODE = TestMode.BOFILL
##################################################################

if TEST_MODE is TestMode.ROTHENBERG_A:
    ROTHENBERG_N = 10

    print("Rothenberg Benchmark A")
    data_folder = base_path+"tests/rothenbergA/"
    if not os.path.isdir(data_folder):
        os.system("mkdir "+data_folder)
    else:
        os.system("rm "+data_folder+"*")
    testRunnerRothenberg(ROTHENBERG_N, "A")

elif TEST_MODE is TestMode.ROTHENBERG_B:
    ROTHENBERG_N = 10

    print("Rothenberg Benchmark B")
    data_folder = base_path+"tests/rothenbergB/"
    if not os.path.isdir(data_folder):
        os.system("mkdir "+data_folder)
    else:
        os.system("rm "+data_folder+"*")
    testRunnerRothenberg(ROTHENBERG_N, "B")

elif TEST_MODE is TestMode.BOFILL:
    VARIABLES_NUM = 1500
    CLAUSES_MAX = 6000
    CLAUSES_STEP = 100
    TESTS_NUM = 100

    print("Bofill et al. Benchmark")
    data_folder = base_path+"tests/bofill_"+str(VARIABLES_NUM)+"_"+str(CLAUSES_MAX)+"/"
    if not os.path.isdir(data_folder):
        os.system("mkdir "+data_folder)
    else:
        os.system("rm "+data_folder+"*")
    testRunnerBofill(VARIABLES_NUM, CLAUSES_MAX, CLAUSES_STEP, TESTS_NUM)

else:
    print("There is no such test."+"\n\n")
