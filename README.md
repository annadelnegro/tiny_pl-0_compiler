# **Tiny PL/0 Compiler**

This project implements a basic compiler written in C. It is designed to process source files written in a simplified high-level language, performing lexical analysis, syntax parsing, and intermediate code generation. The program provides robust error handling and generates machine-readable instructions or bytecode for further execution.

---

## **Overview**

A compiler is a fundamental tool in computer science, translating human-readable high-level code into machine-level instructions. This project demonstrates the key stages of compilation, including:
- **Lexical Analysis**: Tokenizing the input source file.
- **Syntax Analysis**: Validating the structure of the input code.
- **Intermediate Code Generation**: Producing an intermediate representation for execution.
- **Error Reporting**: Identifying and reporting syntax and semantic issues in the source file.

---

## **Features**
- **Lexical Analysis**:
  - Breaks input into tokens such as keywords, identifiers, literals, and operators.
  - Validates token formats (e.g., variable names, numbers).
  
- **Syntax Analysis**:
  - Implements a grammar-based parser to ensure code follows language rules.
  - Detects and reports errors with line numbers for better debugging.

- **Intermediate Code Generation**:
  - Produces bytecode or pseudo-assembly instructions.
  - Handles variable assignments, arithmetic operations, conditional statements, and loops.

- **Error Handling**:
  - Provides detailed messages for syntax and semantic errors.
  - Reports the exact line and column of each error.

- **Supported Constructs**:
  - Variable declarations (`int`, `float`, etc.).
  - Arithmetic operations (`+`, `-`, `*`, `/`).
  - Conditional statements (`if`, `else`).
  - Loops (`while`, `for`).

---

## **Getting Started**

### **Prerequisites**
- A C compiler such as `gcc` or `clang`.
- A text editor for creating source code files.

### **Compilation**
Compile the program using the following command:
```bash
gcc -o hw4compiler hw4compiler.c
