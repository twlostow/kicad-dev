/*
    This file is part of libeval, a simple math expression evaluator

    Copyright (C) 2017 Michael Geselbracht, mgeselbracht3@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <set>
#include <vector>

#ifdef DEBUG
#include <stdarg.h>
#endif

#include "libeval_compiler.h"

/* The (generated) lemon parser is written in C.
 * In order to keep its symbol from the global namespace include the parser code with
 * a C++ namespace.
 */
namespace LIBEVAL
{

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#include "grammar.c"
#include "grammar.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static void libeval_dbg( const char *fmt, ... )
{
#ifdef DEBUG
    va_list ap;
    va_start( ap, fmt );
    fprintf(stderr, "libeval: ");
    vfprintf(stderr, fmt, ap );
    va_end( ap );
#endif
}

std::string UCODE::UOP::Format() const
{
    char str[1024];
    switch(m_op)
    {
        case TR_UOP_PUSH_VAR:
            sprintf( str, "PUSH VAR [%p]", m_arg );
            break;
        case TR_UOP_PUSH_VALUE:
            sprintf(str, "PUSH VAL [%p]", m_arg );
            break;
        case TR_OP_ADD:
            sprintf(str,"ADD\n");
            break;
        case TR_OP_MUL:
            sprintf(str,"MUL\n");
            break;

    }
    return str;
}

void UCODE::Dump()
    {
        printf( "ops: %lu\n", m_ucode.size() );
        for( auto op : m_ucode )
        {
            printf( " %s\n", (const char*) op->Format().c_str() );
        }
    };

std::string TOKENIZER::GetChars( std::function<bool( int )> cond ) const
    {
        std::string rv;
        size_t         p = m_pos;
        //   printf("p %d len %d\n", p, str.length() );
        while( p < m_str.length() && cond( m_str[p] ) )
        {
            rv.append( 1, m_str[p] );
            p++;
        }
        return rv;
    }

    bool TOKENIZER::MatchAhead( std::string match, std::function<bool( int )> stopCond ) const
    {
        int remaining = m_str.length() - m_pos;
        if( remaining < (int)match.length() )
            return false;

        if( m_str.substr( m_pos, match.length() ) == match )
        {
            return ( remaining == (int)match.length() || stopCond( m_str[m_pos + match.length()] ) );
        }
        return false;
    }


COMPILER::COMPILER()
{
    m_localeDecimalSeparator = '.';
    m_parseError = false;
    m_parseFinished = false;
    m_unitResolver.reset(new UNIT_RESOLVER);
    m_parser = LIBEVAL::ParseAlloc( malloc );
}


COMPILER::~COMPILER()
{
    LIBEVAL::ParseFree( m_parser, free );

    // Allow explicit call to destructor
    m_parser = nullptr;

    Clear();
}


void COMPILER::Clear()
{
    //free( current.token );
    m_tokenizer.Clear();
    m_parseError = true;
}


void COMPILER::parseError( const char* s )
{
    m_parseError = true;
}


void COMPILER::parseOk()
{
    m_parseFinished = true;
}



bool COMPILER::Compile( const std::string& aString, UCODE *aCode )
{
    // Feed parser token after token until end of input.

    newString( aString );
    m_tree = nullptr;
    m_parseError = false;
    m_parseFinished = false;
    T_TOKEN tok;

    libeval_dbg("str: '%s' empty: %d\n", aString.c_str(), !!aString.empty() );

    if( aString.empty() )
    {
        m_parseFinished = true;
        return generateUCode(aCode);
    }

    do
    {
        tok = getToken();
        libeval_dbg("Token: '%d'\n", tok.token );

        Parse( m_parser, tok.token, tok.value, this );

        if( m_parseFinished || tok.token == G_ENDS )
        {
            // Reset parser by passing zero as token ID, value is ignored.
            Parse( m_parser, 0, tok.value, this );
            break;
        }
    } while( tok.token );

    return generateUCode(aCode);
}



void COMPILER::newString( const std::string& aString )
{
    Clear();

    m_lexerState = LS_DEFAULT;
    m_tokenizer.Restart( aString );
    m_parseFinished = false;
}

COMPILER::T_TOKEN COMPILER::getToken()
{
    T_TOKEN rv;
    bool done = false;
    do {
        //printf("-> lstate %d\n", m_lexerState);
        switch( m_lexerState )
        {
            case LS_DEFAULT: done = lexDefault( rv ); break;
            case LS_STRING: done = lexString( rv ); break;

        }
    } while( !done );

    return rv;
}



bool COMPILER::lexString( COMPILER::T_TOKEN& aToken )
{
    auto str = m_tokenizer.GetChars( [] ( int c ) -> bool { return c != '"'; } );
    //printf("STR LIT '%s'\n", (const char *)str.c_str() );
    
    aToken.token = G_STRING;
    strcpy( aToken.value.value.str, str.c_str() );

    m_tokenizer.NextChar( str.length() + 1 );
    m_lexerState = LS_DEFAULT;
    return true;
}


int COMPILER::resolveUnits()
{
    int unitId = 0;
    for ( auto unitName : m_unitResolver->GetSupportedUnits() )
    {
        if( m_tokenizer.MatchAhead(unitName, [] ( int c ) -> bool { return !isalnum( c ); } ) )
        {
            libeval_dbg("Match unit '%s'\n", unitName.c_str() );
            m_tokenizer.NextChar(unitName.length());
            return unitId;
        }

        unitId ++;
    }

    return -1;
}

bool COMPILER::lexDefault( COMPILER::T_TOKEN& aToken )
{
    T_TOKEN retval;
    std::string current;
    size_t idx;
    int convertFrom;

    retval.token = G_ENDS;

    if( m_tokenizer.Done() )
    {
        aToken = retval;
        return true;
    }

    auto isDecimalSeparator = [ & ]( char ch ) -> bool {
        return ( ch == m_localeDecimalSeparator || ch == '.' || ch == ',' );
    };

    // Lambda: get value as string, store into clToken.token and update current index.
    auto extractNumber = [ & ]() {
        bool haveSeparator = false;
        idx = 0;
        auto ch = m_tokenizer.GetChar(); 

        do
        {
            if( isDecimalSeparator( ch ) && haveSeparator )
                break;

            current.append(1, ch);

            if( isDecimalSeparator( ch ))
                haveSeparator = true;

            m_tokenizer.NextChar();
            ch = m_tokenizer.GetChar(); 
        } while( isdigit( ch ) || isDecimalSeparator( ch ));

        // Ensure that the systems decimal separator is used
        for( int i = current.length(); i; i-- )
            if( isDecimalSeparator( current[ i - 1 ] ))
                current[ i - 1 ] = m_localeDecimalSeparator;


        //printf("-> NUM: '%s'\n", (const char *) current.c_str() );
    };


    int ch;

    // Start processing of first/next token: Remove whitespace
    for( ;; )
    {
        ch = m_tokenizer.GetChar();

        if( ch == ' ' )
            m_tokenizer.NextChar();
        else
            break;
    }

    //printf("LEX ch '%c' pos %d\n", ch, m_tokenizer.pos );

    if( ch == 0 )
    {
        /* End of input */
    }
    else if( isdigit( ch ) )
    {
        // VALUE
        extractNumber();
        retval.token = G_VALUE;
        strcpy(retval.value.value.str, current.c_str() );
    }
    else if(( convertFrom = resolveUnits()) >= 0 )
    {
        //printf("unit\n");
        // UNIT
        // Units are appended to a VALUE.
        // Determine factor to default unit if unit for value is given.
        // Example: Default is mm, unit is inch: factor is 25.4
        // The factor is assigned to the terminal UNIT. The actual
        // conversion is done within a parser action.
        retval.token = G_UNIT;
        retval.value.value.type = convertFrom;
    }
    else if( ch == '\"') // string literal
    {
        //printf("MATCH STRING LITERAL\n");
        m_lexerState = LS_STRING;
        m_tokenizer.NextChar();
        return false;
    }
    else if( isalpha( ch ) )
    {
        //printf("ALPHA\n");
        current = m_tokenizer.GetChars( [] ( int c ) -> bool { return isalnum( c ); } );
        //printf("Len: %d\n", current.length() );
        //printf("id '%s'\n", (const char *) current.c_str() );
        fflush(stdout);
        retval.token = G_IDENTIFIER;
        strcpy(retval.value.value.str, current.c_str());
        m_tokenizer.NextChar( current.length() );
    }
    else
    {
        if( m_tokenizer.MatchAhead("==", [] ( int c ) -> bool { return c != '='; } ) )
        {
            retval.token = G_EQUAL;
            m_tokenizer.NextChar(2);
        }
        else if( m_tokenizer.MatchAhead("&&", [] ( int c ) -> bool { return c != '&'; } ) )
        {
            retval.token = G_BOOL_AND;
            m_tokenizer.NextChar(2);
        }
        else if( m_tokenizer.MatchAhead("||", [] ( int c ) -> bool { return c != '|'; } ) )
        {
            retval.token = G_BOOL_OR;
            //printf("             GOT OR\n");
            m_tokenizer.NextChar(2);
        } else {

            // Single char tokens
            switch( ch )
            {
            case '+' :retval.token = G_PLUS;   break;
            case '!' :retval.token = G_BOOL_NOT;  break;
            case '-' :retval.token = G_MINUS;  break;
            case '*' :retval.token = G_MULT;   break;
            case '/' :retval.token = G_DIVIDE; break;
            case '<' :retval.token = G_LESS_THAN;   break;
            case '>' :retval.token = G_GREATER_THAN;   break;
            case '(' :retval.token = G_PARENL; break;
            case ')' :retval.token = G_PARENR; break;
            case ';' :retval.token = G_SEMCOL; break;
            case '.' :retval.token = G_STRUCT_REF; break;
            default  :m_parseError = true;   break;   /* invalid character */
            }

            m_tokenizer.NextChar();
        }
    }

    aToken = retval;
    return true;
}

const std::string formatNode(TREE_NODE *tok)
{
    char str[1024];
 //   printf("fmt tok %p v %p ", tok, tok->value.v );
    fflush(stdout);
    sprintf(str, "%s", (const char *) tok->value.str );
    return str;
}


const std::string formatOpName( int op )
{
    switch(op)
    {
        case TR_OP_ADD: return "ADD";
        case TR_OP_MUL: return "MUL";
        case TR_OP_LESS: return "LT";
        case TR_OP_GREATER: return "GT";
        case TR_OP_EQUAL: return "EQ";
        case TR_OP_BOOL_AND: return "AND";
        case TR_OP_BOOL_OR: return "OR";
    }
    return "???";
}

void dumpNode( TREE_NODE *tok, int depth = 0 )
{

    printf("\n[%p] ", tok); //[tok %p] ", tok);
    for(int i = 0; i < 2*depth; i++ )
        printf("  ");

    switch(tok->op)
    {
        case TR_OP_ADD:
        case TR_OP_MUL:
        case TR_OP_LESS:
        case TR_OP_GREATER:
        case TR_OP_EQUAL:
        case TR_OP_BOOL_AND:
        case TR_OP_BOOL_OR:
            printf("%s", (const char *) formatOpName( tok->op ).c_str() ); 
            dumpNode( tok->leaf[0], depth + 1 );
            dumpNode( tok->leaf[1], depth + 1 );
            break;
        case TR_NUMBER:
            printf("NUMERIC: ");
            printf("%s", formatNode( tok ).c_str() );
            if( tok->leaf[0] )
                dumpNode( tok->leaf[0], depth + 1 );
            break;
        case TR_STRING:
            printf("STRING: ");
            printf("%s", formatNode( tok ).c_str() );
            break;
        case TR_IDENTIFIER:
            printf("ID: ");
            printf("%s", formatNode( tok ).c_str() );
            break;
        case TR_STRUCT_REF:
            printf("SREF: ");
            dumpNode( tok->leaf[0], depth + 1 );
            dumpNode( tok->leaf[1], depth + 1 );
            break;
        case TR_UNIT:
            printf("UNIT: %d ", tok->value.type );
            break;
        default:
            printf("OP %d", tok->op);
            break;
    }
}

void COMPILER::setRoot( TREE_NODE root )
{
    dumpNode(&root);
    printf("\n");
    m_tree = copyNode( root );
}


bool COMPILER::generateUCode( UCODE *aCode )
{
    std::vector<TREE_NODE*> stack;
    std::set<TREE_NODE*> visitedNodes;

    auto visited = [&] ( TREE_NODE* node ) -> bool { return visitedNodes.find( node ) != visitedNodes.end(); };

    UCODE code;

    assert( m_tree );

    stack.push_back( m_tree );

    //printf("compile: tree %p\n", m_tree);

    while(!stack.empty())
    {
        auto node = stack.back();
        bool isTerminalNode = true;

        printf("process node %p [op %d] [stack %d]\n", node, node->op, stack.size() );

        // process terminal nodes first
        switch(node->op)
        {
            case TR_STRUCT_REF:
            {
                assert(node->leaf[0]->op == TR_IDENTIFIER );
                assert(node->leaf[1]->op == TR_IDENTIFIER );

                auto vref = aCode->createVarRef( node->leaf[0]->value.str, node->leaf[1]->value.str );
                aCode->AddOp( TR_UOP_PUSH_VAR, vref );
                break;
            }

            case TR_NUMBER:
            {
                auto son = node->leaf[0];

                double value = atof( node->value.str ); // fixme: locale

                if( son && son->op == TR_UNIT )
                {
                    printf("HandleUnit: %s unit %d\n", node->value.str, son->value.type );

                    value = m_unitResolver->Convert( node->value.str, son->value.type );
                    visitedNodes.insert( son );
                }

                aCode->AddOp( TR_UOP_PUSH_VALUE, value );

                break;
            }   
            case TR_STRING:
            {
                aCode->AddOp( TR_UOP_PUSH_VALUE, node->value.str );
                break;
            }
            default:
                isTerminalNode = false;
            break;
        }

        visitedNodes.insert( node );
        stack.pop_back();

        if ( !isTerminalNode && node->leaf[0] && !visited( node->leaf[0] ) )
        {
            //printf("push: %p\n",node->leaf[0] );
            stack.push_back(node->leaf[0] );
        }
        if ( !isTerminalNode && node->leaf[1] && !visited( node->leaf[1] ) )
        {
//            printf("push: %p\n",node->leaf[1] );
           stack.push_back(node->leaf[1] );
        }

        if( !isTerminalNode )
            aCode->AddOp( node->op );
    }


    return true;
}


void UCODE::UOP::Exec( CONTEXT* ctx )
{
    switch( m_op )
    {
        case TR_UOP_PUSH_VAR:
//            ctx->Push(  );
            break;
        case TR_UOP_PUSH_VALUE:
            //ctx->Push( reinterpret_cast<OPERAND> )
            break;
        /*case TR_OP_ADD:
            ctx->Push( arg2 + arg1 );
            break;
        case TR_OP_GREATER:
            ctx->Push( arg2 > arg1 ? 1.0 : 0.0 );
            break;
        case TR_OP_LESS:
            ctx->Push( arg2 < arg1 ? 1.0 : 0.0 );
            break;*/

    }
}

VALUE* UCODE::Run()
{
    CONTEXT ctx;
    for( const auto op : m_ucode )
        op->Exec( &ctx );

    assert ( ctx.SP() == 1 );
    return ctx.Pop();
}

}
