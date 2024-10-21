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
    T_ID,
    T_NUM,
    T_IF,
    T_ELSE,
    T_RETURN,
    T_ASSIGN,
    T_PLUS,
    T_MINUS,
    T_MUL,
    T_DIV,
    T_LPAREN,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_SEMICOLON,
    T_EOF,
    T_GT,
    T_LT,
    T_CHAR_LITERAL,
    T_FLOAT_LITERAL,
    T_FOR,
    T_WHILE,
};

struct Token
{
    TokenType type;
    string value;
};

class Lexer
{
private:
    string src;
    size_t pos;

    unordered_map<string, TokenType> keywords = {
        {"if", T_IF},
        {"else", T_ELSE},
        {"return", T_RETURN},
        {"int", T_INT},
        {"char", T_CHAR},
        {"float", T_FLOAT},
        {"double", T_DOUBLE},
        {"for", T_FOR},
        {"while", T_WHILE},
    };

    unordered_map<char, TokenType> symbols = {
        {'=', T_ASSIGN},
        {'+', T_PLUS},
        {'-', T_MINUS},
        {'*', T_MUL},
        {'/', T_DIV},
        {'(', T_LPAREN},
        {')', T_RPAREN},
        {'{', T_LBRACE},
        {'}', T_RBRACE},
        {';', T_SEMICOLON},
        {'>', T_GT},
        {'<', T_LT},
    };

public:
    Lexer(const string &src)
    {
        this->src = src;
        this->pos = 0;
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
            cout << "Invalid character literal" << endl;
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

    vector<Token> tokenize()
    {
        vector<Token> tokens;
        while (pos < src.size())
        {
            char c = src[pos];
            if (isspace(c))
            {
                pos++;
                continue;
            }

            if (isdigit(c) || (c == '.' && pos + 1 < src.size() && isdigit(src[pos + 1])))
            {
                tokens.push_back({T_NUM, consumeNumber()});
                continue;
            }

            if (isalpha(c))
            {
                string word = consumeWord();
                if (keywords.find(word) != keywords.end())
                {
                    tokens.push_back({keywords[word], word});
                }
                else
                {
                    tokens.push_back({T_ID, word});
                }
                continue;
            }

            if (c == '\'')
            {
                tokens.push_back({T_CHAR_LITERAL, consumeCharLiteral()});
                continue;
            }

            if (symbols.find(c) != symbols.end())
            {
                tokens.push_back({symbols[c], string(1, c)});
                pos++;
                continue;
            }

            cout << "Unexpected character: " << c << endl;
            pos++;
        }
        tokens.push_back({T_EOF, ""});
        return tokens;
    }
};

 

class Parser
{
private:
    vector<Token> tokens;
    size_t pos;

public:
    Parser(const vector<Token> &tokens)
    {
        this->tokens = tokens;
        this->pos = 0;
    }

    void parseProgram()
    {
        while (tokens[pos].type != T_EOF)
        {
            parseStatement();
        }
        cout << "Parsing completed successfully" << endl;
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

        if (tokens[pos].type == T_ELSE)
        {
            expect(T_ELSE);

            if (tokens[pos].type == T_LBRACE)
            {
                parseBlock();
            }
            else
            {
                parseStatement();
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
            tokens[pos].type == T_FLOAT || tokens[pos].type == T_DOUBLE)
        {
            parseDeclarationAndAssignment();
        }
        else if (tokens[pos].type == T_ID)
        {
            parseAssignment();
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
        else if (tokens[pos].type == T_LBRACE)
        {
            parseBlock();
        }
    }

    void parseDeclarationAndAssignment()
    {
        expectType();
        expect(T_ID);
        if (tokens[pos].type == T_ASSIGN)
        {
            pos++;
            parseExpression();
        }
        expect(T_SEMICOLON);
    }

    void parseForLoop()
    {
        expect(T_FOR);
        expect(T_LPAREN);
        parseAssignment();
        parseExpression();
        expect(T_SEMICOLON);
        parseAssignment();
        expect(T_RPAREN);
        parseBlock();
    }

    void parseWhileLoop()
    {
        expect(T_WHILE);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseBlock();
    }

    void parseAssignment()
    {
        expect(T_ID);
        expect(T_ASSIGN);
        parseExpression();
        expect(T_SEMICOLON);
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

    void parseExpression()
    {
        parseTerm();
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS)
        {
            pos++;
            parseTerm();
        }
    }

    void parseTerm()
    {
        parseFactor();
        while (tokens[pos].type == T_MUL || tokens[pos].type == T_DIV)
        {
            pos++;
            parseFactor();
        }
    }

    void parseFactor()
    {
        if (tokens[pos].type == T_LPAREN)
        {
            expect(T_LPAREN);
            parseExpression();
            expect(T_RPAREN);
        }
        else if (tokens[pos].type == T_NUM)
        {
            pos++;
        }
        else if (tokens[pos].type == T_ID)
        {
            pos++;
        }
        else
        {
            cout << "Unexpected token: " << tokens[pos].value << endl;
            exit(1);
        }
    }

    void expect(TokenType type)
    {
        if (tokens[pos].type == type)
        {
            pos++;
        }
        else
        {
            cout << "Expected token " << type << ", but got " << tokens[pos].type << endl;
            exit(1);
        }
    }

    void expectType()
    {
        if (tokens[pos].type == T_INT || tokens[pos].type == T_CHAR ||
            tokens[pos].type == T_FLOAT || tokens[pos].type == T_DOUBLE)
        {
            pos++;
        }
        else
        {
            cout << "Expected a type, but got " << tokens[pos].type << endl;
            exit(1);
        }
    }
};

int main(int argc ,char* argv[])
{

    string code = "";


    if (argc != 2) {
        cerr << "Usage: mycompiler <filename.txt>\n";
        return 1;
    }

    string filename = argv[1];
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << '\n';
        return 1;
    }

    string line;
    while (getline(file, line)) {
        code+= line;
    }

    file.close();


    Lexer lexer(code);
    vector<Token> tokens = lexer.tokenize();

    for (const auto &token : tokens)
    {
        cout << "Token: " << token.value << ", Type: " << token.type << endl;
    }

    return 0;
}