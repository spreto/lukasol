# lukasol
Solver for satisfiability and logical consequence validity in Łukasiewicz Infinitely-valued Logic.

## Installation

You must have in your computer the compilers **gcc** and **g++**, the **SCIP Optimization Suite** callable library [(www.scipopt.org)](https://www.scipopt.org/) and the Yices SMT Solver [(yices.csl.sri.com)](https://yices.csl.sri.com/).

To compile **lukasol**, type the following at the root of the distribution directory:

> $ make

To remove the compiled files, type:

> $ make clean

## Piecewise Linear Functions Benchmarks

At *pwlTests/*, you find scripts for automatic generation of instances stating properties about randomly generated rational McNaughton functions, which constitute a subclass of piecewise linear functions that are computed by a class of neural networks. See S. Preto and M. Finger, “Proving properties of binary classification
neural networks via Łukasiewicz logic,” *Logic Journal of the IGPL*, advance online publication. [doi.org/10.1093/jigpal/jzac050](https://doi.org/10.1093/jigpal/jzac050).
