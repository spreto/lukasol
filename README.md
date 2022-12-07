# lukasol
Solver for satisfiability and logical consequence validity in Łukasiewicz Infinitely-valued Logic.

## Installation

You must have in your computer the compilers **gcc** and **g++**, the **SCIP Optimization Suite** [(www.scipopt.org)](https://www.scipopt.org/) and the **Yices SMT Solver** [(yices.csl.sri.com)](https://yices.csl.sri.com/).

To compile **lukasol**, type the following at the root of the distribution directory:

> $ make

To remove the compiled files, type:

> $ make clean

## Piecewise Linear Functions Benchmarks

At *pwlTests/*, you find scripts for automatic generation of instances stating properties about randomly generated rational McNaughton functions, which constitute a subclass of piecewise linear functions that are computed by a class of neural networks.

For a reference, see:

S. Preto and M. Finger, “Proving properties of binary classification
neural networks via Łukasiewicz logic,” *Logic Journal of the IGPL*, advance online publication.
[doi.org/10.1093/jigpal/jzac050](https://doi.org/10.1093/jigpal/jzac050).

### Funding

This work was supported by grant #2021/10134-0, São Paulo Research Foundation (FAPESP).

This work was carried out at the Artificial Intelligence Research Institute, Spanish National Research Council (IIIA-CSIC); and the Center for Artificial Intelligence (C4AI-USP), with support by the São Paulo Research Foundation (FAPESP), grant #2019/07665-4, and by the IBM Corporation.
