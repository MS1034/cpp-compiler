#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <map>
#include <fstream>

using namespace std;

enum TokenType
{
    T_INT,
    T_CHAR,
    T_FLOAT,
    T_DOUBLE,
    T_BOOLEAN,
    T_ID,
    T_NUM,
    T_IF,
    T_AND,
    T_OR,
    T_EQ,
    T_NEQ,
    T_GTE,
    T_LTE,
    T_TRUE,
    T_FALSE,
    T_ELSE,
    T_RETURN,
    T_ASSIGN,
    T_PLUS_ASSIGN,
    T_MINUS_ASSIGN,
    T_MUL_ASSIGN,
    T_DIV_ASSIGN,
    T_INCREMENT,
    T_DECREMENT,
    T_PLUS,
    T_MINUS,
    T_MUL,
    T_DIV,
    T_MOD,
    T_LPAREN,
    T_RPAREN,
    T_COMMA,
    T_LBRACE,
    T_RBRACE,
    T_SEMICOLON,
    T_COLON,
    T_QUESTION,
    T_GT,
    T_LT,
    T_CHAR_LITERAL,
    T_FLOAT_LITERAL,
    T_FOR,
    T_WHILE,
    T_SWITCH,
    T_CASE,
    T_BREAK,
    T_CONTINUE,
    T_DEFAULT,

    T_EOL,
    T_EOF,
};

struct Token
{
    TokenType type;
    string value;
    int line;
};

class SymbolTable
{
private:
    unordered_map<string, TokenType> table;

public:
    void insert(const string &identifier, TokenType type)
    {
        table[identifier] = type;
    }

    bool exists(const string &identifier)
    {
        return table.find(identifier) != table.end();
    }

    TokenType getType(const string &identifier)
    {
        return table.at(identifier);
    }

    void printTable()
    {
        for (const auto &entry : table)
        {
            cout << "Identifier: " << entry.first << ", Type: " << entry.second << endl;
        }
    }
};
class Lexer
{
private:
    string src;
    size_t pos;
    int lineNumber;

    unordered_map<string, TokenType> keywords = {
        {"if", T_IF},
        {"agar", T_IF},
        {"else", T_ELSE},
        {"return", T_RETURN},
        {"int", T_INT},
        {"char", T_CHAR},
        {"float", T_FLOAT},
        {"double", T_DOUBLE},
        {"bool", T_BOOLEAN},
        {"for", T_FOR},
        {"while", T_WHILE},
        {"true", T_TRUE},
        {"false", T_FALSE},
        {"switch", T_SWITCH},
        {"case", T_CASE},
        {"break", T_BREAK},
        {"continue", T_CONTINUE},
        {"default", T_DEFAULT}};

    unordered_map<char, TokenType> symbols = {
        {'=', T_ASSIGN},
        {'+', T_PLUS},
        {'-', T_MINUS},
        {'*', T_MUL},
        {'/', T_DIV},
        {'%', T_MOD},
        {',', T_COMMA},
        {'(', T_LPAREN},
        {')', T_RPAREN},
        {'{', T_LBRACE},
        {'}', T_RBRACE},
        {';', T_SEMICOLON},
        {':', T_COLON},
        {'?', T_QUESTION},
        {'>', T_GT},
        {'<', T_LT},
        {'\n', T_EOL},
    };

    unordered_map<string, TokenType> multiCharSymbols = {
        {"&&", T_AND},
        {"||", T_OR},
        {"==", T_EQ},
        {"!=", T_NEQ},
        {">=", T_GTE},
        {"<=", T_LTE},
        {"+=", T_PLUS_ASSIGN},
        {"-=", T_MINUS_ASSIGN},
        {"*=", T_MUL_ASSIGN},
        {"/=", T_DIV_ASSIGN},
        {"++", T_INCREMENT},
        {"--", T_DECREMENT},
    };

public:
    Lexer(const string &src)
    {
        this->src = src;
        this->pos = 0;
        this->lineNumber = 1;
    }

    string consumeCharLiteral()
    {
        size_t start = this->pos;
        this->pos++;

        if (this->src[this->pos] == '\\')
        {
            this->pos += 2;
        }
        else
        {
            this->pos++;
        }

        if (this->pos >= this->src.size() || this->src[this->pos] != '\'')
        {
            cerr << "Invalid character literal at line number " << lineNumber << endl;
            exit(1);
        }
        this->pos++;

        return src.substr(start, this->pos - start);
    }

    string consumeNumber()
    {
        size_t start = this->pos;
        bool hasDecimalPoint = false;

        while (this->pos < this->src.size() && (isdigit(this->src[this->pos]) || this->src[this->pos] == '.'))
        {
            if (this->src[this->pos] == '.')
            {
                if (hasDecimalPoint)
                    break;
                hasDecimalPoint = true;
            }
            pos++;
        }

        return src.substr(start, pos - start);
    }

    string consumeWord()
    {
        size_t start = this->pos;
        while (this->pos < this->src.size() && isalnum(this->src[this->pos]))
            pos++;
        return src.substr(start, pos - start);
    }

    vector<Token> tokenize(SymbolTable &symbolTable)
    {
        vector<Token> tokens;
        while (pos < src.size())
        {
            char c = src[pos];
            if (c == '\n')
            {
                lineNumber++;
            }
            if (isspace(c))
            {
                pos++;
                continue;
            }

            if (isdigit(c) || (c == '.' && pos + 1 < src.size() && isdigit(src[pos + 1])))
            {
                tokens.push_back({T_NUM, consumeNumber(), lineNumber});
                continue;
            }

            if (isalpha(c))
            {
                string word = consumeWord();
                if (keywords.find(word) != keywords.end())
                {
                    tokens.push_back({keywords[word], word, lineNumber});
                }
                else
                {
                    tokens.push_back({T_ID, word, lineNumber});

                    symbolTable.insert(word, T_ID);
                }
                continue;
            }

            if (c == '\'')
            {
                tokens.push_back({T_CHAR_LITERAL, consumeCharLiteral(), lineNumber});
                continue;
            }

            string potentialMultiChar = src.substr(pos, 2);
            if (multiCharSymbols.find(potentialMultiChar) != multiCharSymbols.end())
            {
                tokens.push_back({multiCharSymbols[potentialMultiChar], potentialMultiChar, lineNumber});
                pos += 2;
                continue;
            }

            if (symbols.find(c) != symbols.end())
            {
                tokens.push_back({symbols[c], string(1, c), lineNumber});
                pos++;
                continue;
            }

            cout << "Unexpected character at line number " << lineNumber << ": " << c << endl;
            pos++;
        }
        tokens.push_back({T_EOF, "", lineNumber});

        return tokens;
    }
};
class Parser
{
private:
    vector<Token> tokens;
    size_t pos;
    int lineNumber;
    SymbolTable symbolTable;

public:
    Parser(const vector<Token> &tokens)
    {
        this->tokens = tokens;
        this->pos = 0;
        this->lineNumber = 1;
    }

    void parseProgram()
    {
        while (tokens[pos].type != T_EOF)
        {
            parseStatement();
        }
        cout << "Parsing completed successfully" << endl;
        symbolTable.printTable();
    }

    void parseIfStatement()
    {
        expect(T_IF);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);

        if (tokens[pos].type == T_LBRACE)
        {
            parseBlock();
        }
        else
        {
            parseStatement();
        }

        while (tokens[pos].type == T_ELSE)
        {
            pos++;
            if (tokens[pos].type == T_IF)
            {
                pos++;
                expect(T_LPAREN);
                parseExpression();
                expect(T_RPAREN);

                if (tokens[pos].type == T_LBRACE)
                {
                    parseBlock();
                }
                else
                {
                    parseStatement();
                }
            }
            else
            {
                if (tokens[pos].type == T_LBRACE)
                {
                    parseBlock();
                }
                else
                {
                    parseStatement();
                }
                break;
            }
        }
    }

    void parseReturnStatement()
    {
        expect(T_RETURN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseStatement()
    {
        if (tokens[pos].type == T_INT || tokens[pos].type == T_CHAR ||
            tokens[pos].type == T_FLOAT || tokens[pos].type == T_DOUBLE || tokens[pos].type == T_BOOLEAN)
        {
            parseDeclarationAndAssignment();
        }
        else if (tokens[pos].type == T_ID)
        {
            parseAssignment();
            expect(T_SEMICOLON);
        }
        else if (tokens[pos].type == T_IF)
        {
            parseIfStatement();
        }
        else if (tokens[pos].type == T_FOR)
        {
            parseForLoop();
        }
        else if (tokens[pos].type == T_WHILE)
        {
            parseWhileLoop();
        }
        else if (tokens[pos].type == T_RETURN)
        {
            parseReturnStatement();
        }
        else if (tokens[pos].type == T_BREAK)
        {
            parseBreakStatement();
        }
        else if (tokens[pos].type == T_CONTINUE)
        {
            parseContinueStatement();
        }
        else if (tokens[pos].type == T_LBRACE)
        {
            parseBlock();
        }
        else
        {
            cerr << "Unexpected token at line " << lineNumber << ": " << tokens[pos].value << endl;
            exit(1);
        }
    }

    void parseDeclarationAndAssignment()
    {
        TokenType varType = expectType();
        do
        {
            expect(T_ID);
            string identifier = tokens[pos - 1].value;

            if (symbolTable.exists(identifier))
            {
                cerr << "Error: Identifier " << identifier << " already declared at line " << lineNumber << endl;
                exit(1);
            }
            symbolTable.insert(identifier, varType);

            if (tokens[pos].type == T_ASSIGN)
            {
                pos++;
                parseExpression();
            }

            if (tokens[pos].type == T_COMMA)
            {
                pos++;
            }
            else
            {
                expect(T_SEMICOLON);
            }
        } while (tokens[pos].type == T_COMMA);
    }

    void parseForLoop()
    {
        expect(T_FOR);
        expect(T_LPAREN);

        if (tokens[pos].type != T_SEMICOLON)
        {
            parseAssignment();
        }

        expect(T_SEMICOLON);

        if (tokens[pos].type != T_SEMICOLON)
        {
            parseExpression();
        }

        expect(T_SEMICOLON);

        if (tokens[pos].type != T_RPAREN)
        {
            while (tokens[pos].type != T_RPAREN)
            {
                parseAssignment();
                if (tokens[pos].type == T_COMMA)
                {
                    pos++;
                }
            }
        }

        expect(T_RPAREN);

        parseStatement();
    }

    void parseWhileLoop()
    {
        expect(T_WHILE);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);

        if (tokens[pos].type == T_LBRACE)
        {
            parseBlock();
        }
        else
        {
            parseStatement();
        }
    }

    void parseAssignment()
    {
        expect(T_ID);
        string identifier = tokens[pos - 1].value;

        if (!symbolTable.exists(identifier))
        {
            cerr << "Error: Identifier " << identifier << " not declared at line " << lineNumber << endl;
            exit(1);
        }

        if (tokens[pos].type == T_INCREMENT || tokens[pos].type == T_DECREMENT)
        {
            pos++;
        }
        else if (tokens[pos].type == T_ASSIGN || tokens[pos].type == T_PLUS_ASSIGN ||
                 tokens[pos].type == T_MINUS_ASSIGN || tokens[pos].type == T_MUL_ASSIGN ||
                 tokens[pos].type == T_DIV_ASSIGN)
        {
            pos++;
            parseExpression();
        }
        else
        {
            cerr << "Expected assignment or increment/decrement operator at line " << lineNumber << endl;
        }

        if (tokens[pos].type == T_INCREMENT || tokens[pos].type == T_DECREMENT)
        {
            pos++;
        }
    }

    void parseBlock()
    {
        expect(T_LBRACE);
        while (tokens[pos].type != T_RBRACE && tokens[pos].type != T_EOF)
        {
            parseStatement();
        }
        expect(T_RBRACE);
    }

    void parseBreakStatement()
    {
        expect(T_BREAK);
        expect(T_SEMICOLON);
    }

    void parseContinueStatement()
    {
        expect(T_CONTINUE);
        expect(T_SEMICOLON);
    }

    void parseExpression()
    {
        if (tokens[pos].type == T_ID || tokens[pos].type == T_NUM || tokens[pos].type == T_TRUE || tokens[pos].type == T_FALSE)
        {
            parseTernaryExpression();
        }
        else
        {
            parseLogicalOr();
        }
    }

    void parseTernaryExpression()
    {
        parseLogicalOr();

        if (tokens[pos].type == T_QUESTION)
        {
            pos++;
            parseExpression();

            expect(T_COLON);
            parseExpression();
        }
    }

    void parseLogicalOr()
    {
        parseLogicalAnd();
        while (tokens[pos].type == T_OR)
        {
            pos++;
            parseLogicalAnd();
        }
    }

    void parseLogicalAnd()
    {
        parseEquality();
        while (tokens[pos].type == T_AND)
        {
            pos++;
            parseEquality();
        }
    }

    void parseEquality()
    {
        parseRelational();
        while (tokens[pos].type == T_EQ || tokens[pos].type == T_NEQ)
        {
            pos++;
            parseRelational();
        }
    }

    void parseRelational()
    {
        parseAdditive();
        while (tokens[pos].type == T_GT || tokens[pos].type == T_LT || tokens[pos].type == T_GTE || tokens[pos].type == T_LTE)
        {
            pos++;
            parseAdditive();
        }
    }

    void parseAdditive()
    {
        parseMultiplicative();
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS)
        {
            pos++;
            parseMultiplicative();
        }
    }

    void parseMultiplicative()
    {
        parseUnary();
        while (tokens[pos].type == T_MUL || tokens[pos].type == T_DIV || tokens[pos].type == T_MOD)
        {
            pos++;
            parseUnary();
        }
    }

    void parseUnary()
    {
        if (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS || tokens[pos].type == T_INCREMENT || tokens[pos].type == T_DECREMENT)
        {
            pos++;
        }

        if (tokens[pos].type == T_ID || tokens[pos].type == T_NUM || tokens[pos].type == T_TRUE || tokens[pos].type == T_FALSE || tokens[pos].type == T_CHAR_LITERAL || tokens[pos].type == T_FLOAT_LITERAL)
        {
            pos++;
        }
        else if (tokens[pos].type == T_LPAREN)
        {
            pos++;
            parseExpression();
            expect(T_RPAREN);
        }
        else
        {
            cerr << "Expected a valid expression at line " << lineNumber << endl;
        }
    }

    TokenType expectType()
    {
        TokenType type = tokens[pos].type;
        if (type == T_INT || type == T_CHAR || type == T_FLOAT || type == T_DOUBLE || type == T_BOOLEAN)
        {
            pos++;
            return type;
        }
        cerr << "Expected a type at line " << lineNumber << endl;
        exit(1);
    }

    Token expect(TokenType expectedType)
    {
        if (tokens[pos].type == expectedType)
        {
            return tokens[pos++];
        }
        cerr << "Expected token type " << expectedType << " at line " << lineNumber << endl;
        exit(1);
    }
};

int main(int argc, char *argv[])
{

    string code = "";

    if (argc != 2)
    {
        cerr << "Usage: mycompiler <filename.txt>\n";
        return 1;
    }

    string filename = argv[1];
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filename << '\n';
        return 1;
    }

    string line;
    while (getline(file, line))
    {
        code += line;
        code += '\n';
    }
    cout << code;
    file.close();

    Lexer lexer(code);

    SymbolTable symbolTable;
    vector<Token> tokens = lexer.tokenize(symbolTable);

    Parser parser(tokens);
    parser.parseProgram();
    return 0;
}