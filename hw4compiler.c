#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

struct symbol
{
  int kind;
  char name[10];
  int value;
  int level;
  int addr;
  int mark;
};

struct code
{
  int num;
  char name[6];
  int level;
  int M;
};

/*setting enum for lex token types*/
typedef enum
{
  oddsym = 1,
  identsym,
  numbersym,
  plussym,
  minussym,
  multsym,
  slashsym,
  fisym,
  eqlsym,
  neqsym,
  lessym,
  leqsym,
  gtrsym,
  geqsym,
  lparentsym,
  rparentsym,
  commasym,
  semicolonsym,
  periodsym,
  becomessym,
  beginsym,
  endsym,
  ifsym,
  thensym,
  whilesym,
  dosym,
  callsym,
  constsym,
  varsym,
  procsym,
  writesym,
  readsym,
  elsesym
} token_type;

#define ARRAY_SIZE 1000

void getNextToken();
int determineKeyword(const char *array);
void block();
void constant_declaration();
int variable_declaration();
void statement();
void term();
void condition();
void expression();
void factor();
void program();
int symbolTableCheck(char check[10]);
void emit(char name[10], int level, int M, int num);

struct code code_table[500];
struct symbol symbolTable[500];

int numVars;
int currentToken;
int programIndex;
int symTable_iter;
int jpcIndex;
int jumpIndex;
int codeTableIndex;
int level = -1;

/*2d array for lex table & token array for printing*/
char lexArray[ARRAY_SIZE][ARRAY_SIZE];
int tokenArr[ARRAY_SIZE];
FILE *inputFile;

int main(int argc, char *argv[])
{
  /*error handle*/
  if (argc != 2)
  {
    printf("Please provide an input file.\n");
    return 1;
  }

  int i = 0;
  char tempChar;
  char *tempStr;
  int numTok = 0;

  tempStr = malloc(ARRAY_SIZE * sizeof(char));
  memset(tempStr, 0, ARRAY_SIZE * sizeof(char));

  inputFile = fopen(argv[1], "r");

  /*error handle*/
  if (inputFile == NULL)
  {
    printf("File has not been read.\n");
    return 1;
  }

  while ((tempChar = fgetc(inputFile)) != EOF)
  {
    if (isalpha(tempChar))
    {
      tempStr[i] = tempChar;

      // loop until no longer a letter or digit
      while (isalpha(tempChar) != 0 || isdigit(tempChar) != 0)
      {
        // if file ends break
        if ((tempChar = fgetc(inputFile)) == EOF)
        {
          break;
        }
        // break if no longer a letter or number
        if (isalpha(tempChar) == 0 && isdigit(tempChar) == 0)
        {
          break;
        }
        // otherwise iterate and add to character array
        i++;
        tempStr[i] = tempChar;
      }

      // put char back into stream
      ungetc(tempChar, inputFile);

      // if string is greater than 11 characters in length add to lex array and token array with 0
      if (i > 10)
      {
        printf("Error Variable is too long");
        exit(0);
        tokenArr[numTok] = 0;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      else
      {
        tokenArr[numTok] = determineKeyword(tempStr);
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }

      /*reset for next array*/
      memset(tempStr, 0, 500 * sizeof(char));
      i = 0;
    }
    else if (isdigit(tempChar))
    {
      // add to character array
      tempStr[i] = tempChar;
      // loop until no longer a number
      while (isdigit(tempChar) != 0)
      {
        // if reaches end of file break
        if ((tempChar = fgetc(inputFile)) == EOF)
        {
          break;
        }
        // if its no longer a number break
        if (isdigit(tempChar) == 0)
        {
          break;
        }
        // otherwise iterate and add to character array
        i++;
        tempStr[i] = tempChar;
      }
      // put character back in stream
      ungetc(tempChar, inputFile);
      // if number is greater than 5 digits add to token array of 0 and lex array
      if (i > 4)
      {
        printf("Error Number is too long");
        exit(0);
        tokenArr[numTok] = 0;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // else its a valid number and add to token array and lex array
      else
      {
        tokenArr[numTok] = numbersym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // reset memory and iterator to 0
      memset(tempStr, 0, 1000 * sizeof(char));
      i = 0;
    }
    else if (tempChar == ' ' || tempChar == '\r' || tempChar == '\n' || tempChar == '\t')
    {
      memset(tempStr, 0, 1000 * sizeof(char));
      i = 0;
    }
    else
    {
      /*handling symbols (including :=)*/
      tempStr[i] = tempChar;
      if (strcmp(tempStr, "+") == 0)
      {
        tokenArr[numTok] = plussym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and add to token array and lex array if equal
      else if (strcmp(tempStr, "-") == 0)
      {
        tokenArr[numTok] = minussym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and add to token array and lex array if equal
      else if (strcmp(tempStr, "*") == 0)
      {
        tokenArr[numTok] = multsym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and if equal check for comment
      else if (strcmp(tempStr, "/") == 0)
      {
        // if next character is end of file break
        if ((tempChar = fgetc(inputFile)) == EOF)
        {
          tokenArr[numTok] = slashsym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
          break;
        }
        // if next character is not an * its a slashsym add to token and lex array and put character back into stream
        else if (tempChar != '*')
        {
          ungetc(tempChar, inputFile);
          tokenArr[numTok] = slashsym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
        // if next character is an *
        else if (tempChar == '*')
        {
          // loop until comment concludes or reaches end of file
          while ((tempChar != '*' || (tempChar = fgetc(inputFile)) != '/'))
          {
            tempChar = fgetc(inputFile);
            // if character reaches end of file break
            if (tempChar == EOF)
            {
              break;
            }
          }
        }
      }
      // compare and add to token array and lex array if equal
      else if (strcmp(tempStr, "(") == 0)
      {
        tokenArr[numTok] = lparentsym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and add to token array and lex array if equal
      else if (strcmp(tempStr, ")") == 0)
      {
        tokenArr[numTok] = rparentsym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and add to token array and lex array if equal
      else if (tempChar == '=')
      {
        tokenArr[numTok] = eqlsym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and add to token array and lex array if equal
      else if (strcmp(tempStr, ",") == 0)
      {
        tokenArr[numTok] = commasym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and add to token array and lex array if equal
      else if (strcmp(tempStr, ".") == 0)
      {
        tokenArr[numTok] = periodsym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and if equal check for other cases
      else if (strcmp(tempStr, "<") == 0)
      {
        // if next character is end of file add to token and lex array and break
        if ((tempChar = fgetc(inputFile)) == EOF)
        {
          tokenArr[numTok] = lessym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
          break;
        }
        // if next character is > add to character array and token and lex array
        else if (tempChar == '>')
        {
          i++;
          tempStr[i] = tempChar;
          tokenArr[numTok] = neqsym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
        // if next character is = add to character array and token and lex array
        else if (tempChar == '=')
        {
          i++;
          tempStr[i] = tempChar;
          tokenArr[numTok] = leqsym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
        // if none of the above then put character back into stream and add to lex and token array
        else
        {
          ungetc(tempChar, inputFile);
          tokenArr[numTok] = lessym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
      }
      // compare and if equal check for other cases
      else if (strcmp(tempStr, ">") == 0)
      {
        // if next character is end of file add to token and lex array and break
        if ((tempChar = fgetc(inputFile)) == EOF)
        {
          tokenArr[numTok] = gtrsym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
          break;
        }
        // if next character is = add to character array and to token and lex array
        else if (tempChar == '=')
        {
          i++;
          tempStr[i] = tempChar;
          tokenArr[numTok] = geqsym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
        // if none of the other two cases its put character back into stream and add to token and lex array
        else
        {
          ungetc(tempChar, inputFile);
          tokenArr[numTok] = gtrsym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
      }
      // compare and add to token array and lex array if equal
      else if (strcmp(tempStr, ";") == 0)
      {
        tokenArr[numTok] = semicolonsym;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // compare and if equal check other cases
      else if (strcmp(tempStr, ":") == 0)
      {
        // if next character reaches end of file break
        if ((tempChar = fgetc(inputFile)) == EOF)
        {
          printf("Error Invalid Character");
          exit(0);
          tokenArr[numTok] = -1;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
          break;
        }
        // if next character is not = it is an invalid character put back into stream and add to token array with -1 and add to lex array
        else if (tempChar != '=')
        {
          printf("Error Invalid Character");
          exit(0);
          ungetc(tempChar, inputFile);
          tokenArr[numTok] = -1;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
        // else it is a becomesym add to character array and token and lex array
        else
        {
          i++;
          tempStr[i] = tempChar;
          tokenArr[numTok] = becomessym;
          strcpy(lexArray[numTok], tempStr);
          numTok++;
        }
      }
      // else the character is invalid and add it to lex array and token array with -1
      else
      {
        printf("Error Invalid Character");
        exit(0);
        tempStr[i] = tempChar;
        tokenArr[numTok] = -1;
        strcpy(lexArray[numTok], tempStr);
        numTok++;
      }
      // reset memory and iterator to 0
      memset(tempStr, 0, 1000 * sizeof(char));
      i = 0;
    }
  }

  free(tempStr);

  programIndex = 0;
  codeTableIndex = 0;
  currentToken = tokenArr[programIndex];
  symTable_iter = 0;

  program();

  rewind(inputFile);
  char string[ARRAY_SIZE];

  printf("\n");

  while (fgets(string, ARRAY_SIZE, inputFile))
    printf("%s", string);

  fclose(inputFile);

  printf("\n\n*No errors found. Program's syntax is correct.*\n\n");
  printf("Assembly Code:\n");

  FILE *elfFilePtr;
  elfFilePtr = fopen("elf.txt", "w");

  for (int y = 0; y < codeTableIndex; y++)
  {
    printf("%d\t%s\t%d\t%d\n", y, code_table[y].name, code_table[y].level, code_table[y].M);
    fprintf(elfFilePtr, "%d %d %d\n", code_table[y].num, code_table[y].level, code_table[y].M);
  }

  printf("\n");

  fclose(elfFilePtr);

  for (int y = 0; y < ARRAY_SIZE; y++)
    symbolTable[y].mark = 1;

  return 0;
}

int determineKeyword(const char *array)
{
  /*handling keywords*/
  if (strcmp(array, "var") == 0)
    return varsym;
  else if (strcmp(array, "const") == 0)
    return constsym;
  else if (strcmp(array, "procedure") == 0)
    return procsym;
  else if (strcmp(array, "begin") == 0)
    return beginsym;
  else if (strcmp(array, "end") == 0)
    return endsym;
  else if (strcmp(array, "if") == 0)
    return ifsym;
  else if (strcmp(array, "then") == 0)
    return thensym;
  else if (strcmp(array, "while") == 0)
    return whilesym;
  else if (strcmp(array, "do") == 0)
    return dosym;
  else if (strcmp(array, "call") == 0)
    return callsym;
  else if (strcmp(array, "write") == 0)
    return writesym;
  else if (strcmp(array, "read") == 0)
    return readsym;
  else if (strcmp(array, "else") == 0)
    return elsesym;
  else if (strcmp(array, "fi") == 0)
    return fisym;
  else if (strcmp(array, "odd") == 0)
    return oddsym;
  else
    return identsym; /*defaulting to identifier*/
}

// function to handle the procedure declaration from the input file
void procedure_declaration()
{
  while (currentToken == procsym)
  {
    getNextToken();

    if (currentToken != identsym)
    {
      printf("Error: Procedure Must Be Followed By Indentifier");
      exit(0);
    }

    struct symbol new;
    new.kind = 3;
    new.value = 0;
    strcpy(new.name, lexArray[programIndex]);
    new.level = level;
    // maybe just 2
    new.addr = codeTableIndex;
    new.mark = 0;
    symbolTable[symTable_iter] = new;
    symTable_iter++;

    getNextToken();

    if (currentToken != semicolonsym)
    {
      printf("Error: Procedure Declaration Must Be Followed By Semicolon");
      exit(0);
    }

    getNextToken();
    block();

    if (currentToken != semicolonsym)
    {
      printf("Error: Expected Semicolon After Procedure Declaration Block");
      exit(0);
    }

    getNextToken();
  }
}

// function to get the next token
void getNextToken()
{
  programIndex++;
  currentToken = tokenArr[programIndex];
}

// function to check if symbol is on the symbol table
int symbolTableCheck(char check[10])
{
  for (int i = symTable_iter - 1; i >= 0; i--)
  {
    if (strcmp(check, symbolTable[i].name) == 0)
    {
      return i;
    }
  }
  return -1;
}

void program()
{
  block();
  if (currentToken != periodsym)
  {
    printf("Error: Program Must End With Period");
    exit(0);
  }
  emit("SYS", 0, 3, 9);
}

void block()
{
  level++;
  int originalSymbolCount = symTable_iter;
  int currJMP = codeTableIndex;
  emit("JMP", 0, codeTableIndex, 7);

  constant_declaration();
  numVars = variable_declaration();

  procedure_declaration();

  code_table[currJMP].M = codeTableIndex * 3;

  emit("INC", 0, 3 + numVars, 6);
  statement();

  if (level != 0)
  {
    emit("RTN", 0, 0, 2);
  }
  symTable_iter = originalSymbolCount;
  level--;
}

// this function handles the declaration of constants, making sure that it follows proper syntax
void constant_declaration()
{
  if (currentToken == constsym)
  {
    do
    {
      getNextToken();

      if (currentToken != identsym)
      {
        printf("Error: Const Must Be Followed By Identifier");
        exit(0);
      }
      if (symbolTableCheck(lexArray[programIndex]) != -1)
      {
        printf("Error: Symbol Name Already Declared");
        exit(0);
      }

      char saveIdent[10];
      strcpy(saveIdent, lexArray[programIndex]);
      getNextToken();

      if (currentToken != eqlsym)
      {
        printf("Error: Constants Must Be Assigned With =");
        exit(0);
      }

      getNextToken();

      if (currentToken != numbersym)
      {
        printf("Error: Constant Must Be Assigned An Integer value");
        exit(0);
      }

      struct symbol new;
      new.kind = 1;
      strcpy(new.name, saveIdent);

      int test2 = atoi(lexArray[programIndex]);
      new.value = test2;
      new.level = 0;
      new.addr = 0;
      new.mark = 0;
      symbolTable[symTable_iter] = new;
      symTable_iter++;
      getNextToken();
    } while (currentToken == commasym);

    if (currentToken != semicolonsym)
    {
      printf("Error: Constant Declaration Must Be Followed By A Semicolon");
      exit(0);
    }
    getNextToken();
  }
}

// this function handles the declaration of vars, making sure that it follows proper syntax
int variable_declaration()
{
  numVars = 0;
  if (currentToken == varsym)
  {
    do
    {
      numVars++;
      getNextToken();
      if (currentToken != identsym)
      {
        printf("Error: Var Must Be Followed By Identifier");
        exit(0);
      }
      int try = symbolTableCheck(lexArray[programIndex]);
      if (try != -1 && symbolTable[try].level == level)
      {
        printf("Error: Symbol Name Already Declared");
        exit(0);
      }
      struct symbol new;
      new.kind = 2;
      new.value = 0;
      strcpy(new.name, lexArray[programIndex]);
      new.level = 0;
      new.addr = numVars + 2;
      new.mark = 0;
      symbolTable[symTable_iter] = new;
      symTable_iter++;
      getNextToken();
    } while (currentToken == commasym);
    if (currentToken != semicolonsym)
    {
      printf("Error: Variable Declaration Must Be Followed By Semicolon");
      exit(0);
    }
    getNextToken();
  }
  return numVars;
}

// this function handles the use of statements such as begin, while/do, if/then/fi, etc
void statement()
{
  int symbolIndex;
  if (currentToken == identsym)
  {
    symbolIndex = symbolTableCheck(lexArray[programIndex]);
    if (symbolIndex == -1)
    {
      printf("Error: Undeclared Identifier");
      exit(0);
    }
    if (symbolTable[symbolIndex].kind == 3)
    {
      symbolIndex = symbolTableCheck(lexArray[programIndex]);
    }
    if (symbolTable[symbolIndex].kind != 2)
    {
      printf("Error: Only Variable values May Be Altered");
      exit(0);
    }
    getNextToken();
    if (currentToken != becomessym)
    {
      printf("Error: Assignment Statements Must Use :=");
      exit(0);
    }
    getNextToken();
    expression();
    emit("STO", level - symbolTable[symbolIndex].level, symbolTable[symbolIndex].addr, 4);
    return;
  }
  if (currentToken == callsym)
  {
    int token;
    // get next token
    getNextToken();
    // if current token is not ident sym
    if (currentToken != identsym)
    {
      printf("Error: Call Must Be Followed By An Identifier");
      exit(0);
    }
    // search for programIndex
    token = symbolTableCheck(lexArray[programIndex]);
    // if programIndex doesnt exist in symbol table error
    if (token == -1)
    {
      printf("Error: Undeclared Variable");
      exit(0);
    }
    // if its a procedure
    if (symbolTable[token].kind == 3)
    {
      emit("CAL", level - symbolTable[token].level, symbolTable[token].addr * 3, 5);
    }
    // otherwise error
    else
    {
      printf("Error: Call Must Be Followed By A Procedure Indentifier");
      exit(0);
    }
    // get next token
    getNextToken();
  }
  if (currentToken == beginsym)
  {
    do
    {
      getNextToken();
      statement();
    } while (currentToken == semicolonsym);
    if (currentToken != endsym)
    {
      printf("Error: Begin Must Be Followed By End");
      exit(0);
    }
    getNextToken();
    return;
  }
  if (currentToken == ifsym)
  {
    getNextToken();
    condition();
    jpcIndex = codeTableIndex;
    emit("JPC", 0, jpcIndex, 8);
    if (currentToken != thensym)
    {
      printf("Error: If Must Be Followed By Then");
      exit(0);
    }
    getNextToken();
    statement();

    if (currentToken != fisym)
    {
      printf("Error: Then Must Be Followed By Fi");
      exit(0);
    }

    getNextToken();
    code_table[jpcIndex].M = codeTableIndex * 3;
    return;
  }
  if (currentToken == whilesym)
  {
    getNextToken();
    jumpIndex = codeTableIndex;
    condition();
    if (currentToken != dosym)
    {
      printf("Error: While Must Be Followed By Do");
      exit(0);
    }
    getNextToken();
    jpcIndex = codeTableIndex;
    emit("JPC", 0, jumpIndex, 8);
    statement();
    emit("JMP", 0, jumpIndex, 7);
    code_table[jpcIndex].M = codeTableIndex * 3;
    return;
  }
  if (currentToken == readsym)
  {
    getNextToken();
    if (currentToken != identsym)
    {
      printf("Error: Read Must Be Followed By Identifier");
      exit(0);
    }
    symbolIndex = symbolTableCheck(lexArray[programIndex]);
    if (symbolIndex == -1)
    {
      printf("Error: Undeclared Identifier");
      exit(0);
    }
    if (symbolTable[symbolIndex].kind != 2)
    {
      printf("Error: Only Variables Should Be Read");
      exit(0);
    }
    getNextToken();
    emit("READ", 0, 2, 9);
    emit("STO", level - symbolTable[symbolIndex].level, symbolTable[symbolIndex].addr, 4);
    return;
  }
  if (currentToken == writesym)
  {
    getNextToken();
    expression();
    emit("SYS", 0, 1, 9);
    return;
  }
}

// this functions handles the use of conditional operators
void condition()
{
  if (currentToken == oddsym)
  {
    getNextToken();
    expression();
    emit("ODD", 0, 11, 2);
  }
  else
  {
    expression();
    if (currentToken == eqlsym)
    {
      getNextToken();
      expression();
      emit("EQL", 0, 5, 2);
    }
    else if (currentToken == neqsym)
    {
      getNextToken();
      expression();
      emit("NEQ", 0, 6, 2);
    }
    else if (currentToken == lessym)
    {
      getNextToken();
      expression();
      emit("LSS", 0, 7, 2);
    }
    else if (currentToken == leqsym)
    {
      getNextToken();
      expression();
      emit("LEQ", 0, 8, 2);
    }
    else if (currentToken == gtrsym)
    {
      getNextToken();
      expression();
      emit("GTR", 0, 9, 2);
    }
    else if (currentToken == geqsym)
    {
      getNextToken();
      expression();
      emit("GEQ", 0, 10, 2);
    }
    else
    {
      printf("Error: Condition Must Contain Comparison Operator");
      exit(0);
    }
  }
}

// this functions handles the use of ADD and SUB, making sure it follows syntax
void expression()
{
  term();
  while (currentToken == plussym || currentToken == minussym)
  {
    if (currentToken == minussym)
    {
      getNextToken();
      term();
      emit("SUB", 0, 2, 2);
    }
    else
    {
      getNextToken();
      term();
      emit("ADD", 0, 1, 2);
    }
  }
}

// this functions handles the use of MUL and DIV, making sure it follows syntax
void term()
{
  factor();
  while (currentToken == multsym || currentToken == slashsym)
  {
    if (currentToken == multsym)
    {
      getNextToken();
      factor();
      emit("MUL", 0, 3, 2);
    }
    else
    {
      getNextToken();
      factor();
      emit("DIV", 0, 4, 2);
    }
  }
}

// this function handles the use of parenthesis in arithmetic operations, making sure that if a prenthesis is opened, it has to be closed otherwise it throws an error
void factor()
{
  int symbolIndex;
  if (currentToken == identsym)
  {
    symbolIndex = symbolTableCheck(lexArray[programIndex]);
    if (symbolIndex == -1)
    {
      printf("Error: Undeclared Identifier");
      exit(0);
    }
    if (symbolTable[symbolIndex].kind == 1)
    {
      emit("LIT", 0, symbolTable[symbolIndex].value, 1);
    }
    else
    {
      emit("LOD", level - symbolTable[symbolIndex].level, symbolTable[symbolIndex].addr, 3);
    }
    getNextToken();
  }
  else if (currentToken == numbersym)
  {
    int test2 = atoi(lexArray[programIndex]);
    emit("LIT", 0, test2, 1);
    getNextToken();
  }
  else if (currentToken == lparentsym)
  {
    getNextToken();
    expression();
    if (currentToken != rparentsym)
    {
      printf("Error: Right Parenthesis Must Follow Left Parenthesis");
      exit(0);
    }
    getNextToken();
  }
  else
  {
    printf("Error: Arithmetic Equations Must Contain Operands, Parenthesis, Numbers, or Symbols");
    printf("%d", currentToken);
    exit(0);
  }
}

// this function stores a line in assembly code in order to properly use it later
void emit(char name[10], int level, int M, int num)
{
  struct code newCode;
  strcpy(newCode.name, name);
  newCode.level = level;

  if (strcmp(name, "JMP") == 0)
    newCode.M = M * 3;
  else
    newCode.M = M;

  newCode.num = num;
  code_table[codeTableIndex] = newCode;
  codeTableIndex++;
}
