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



// class SymbolTableEntry {  
// public:  
//   string identifier, scope, dataType; 
//     int lineNo; 
  
// SymbolTableEntry(const string& n, const string& dt, const string&sc,  const int&ln)  
//         : identifier(n), dataType(dt), scope(sc), lineNo(ln) {}  
// };  


// class SymbolTable {  
// public:  
//     void insert(const SymbolTableEntry& entry) {  
// entries.push_back(entry);  
//     }  
  
// SymbolTableEntry* lookup(const string& identifier) {  
//         for (auto&entry : entries) {  
//             if (entry.identifier == identifier) {  
//                 return &entry;
//             }  
//         }  
//         return nullptr;  
//     }  
  
// private:  
// vector<SymbolTableEntry> entries;  
// };  



struct Token
{
    TokenType type;
    string value;
    int line;
};

class Lexer
{
private:
    string src;
    size_t pos;
    int lineNumber;


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
            cerr << "Invalid character literal at line number "<< lineNumber <<  endl;
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
            if(c == '\n')
            {
                lineNumber++;
                pos++;
            }
            if (isspace(c))
            {
                pos++;
                continue;
            }

            if (isdigit(c) || (c == '.' && pos + 1 < src.size() && isdigit(src[pos + 1])))
            {
                tokens.push_back({T_NUM, consumeNumber(),lineNumber});
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
                    tokens.push_back({T_ID, word,lineNumber});
                }
                continue;
            }

            if (c == '\'')
            {
                tokens.push_back({T_CHAR_LITERAL, consumeCharLiteral(),lineNumber});
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
        tokens.push_back({T_EOF, "",lineNumber});
        return tokens;
    }
};

 

class Parser
{
private:
    vector<Token> tokens;
    size_t pos;
    int lineNumber;


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
            
            cerr << "Unexpected token: " << tokens[pos].value <<" at line number " << tokens[pos].line  << endl;
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
            cerr << "Expected token " << type << ", but got " << tokens[pos].type <<" at line number " << tokens[pos].line << endl;
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
            cerr << " Expected a type, but got " << tokens[pos].type <<" at line number " << tokens[pos].line << endl;
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
        code+= '\n';
    }

    file.close();


    Lexer lexer(code);
    vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    parser.parseProgram();
    return 0;
}