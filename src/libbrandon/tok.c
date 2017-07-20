#include <stdlib.h>
#include <string.h>

#include <brandon/tok.h>
#include <brandon/vvector.h>

struct tokenizer_str
{
    int pos;            //Current position: sits on the next token
    int length;         //Number of elements
    char **elements;    //Array of strings
};

static void Tokenizer_ctor_custom( Tokenizer *thiz,
                                   const char *str,
                                   VVector * (* func)(const char *, void *),
                                   void * param)
{
    if (thiz == NULL)
        return;

    VVector * vec = func( str, param );

    thiz->pos = 0;
    thiz->length = VVector_length(vec);                 //get the size of the array returned
    thiz->elements = (char**)VVector_toArray_cpy(vec);  //get the array of strings
    VVector_delete(vec);    //free the vector
}

static VVector * strtok_wrapper( const char * str, void * param )
{
    const char * del = (const char *) param;
    VVector* vec = VVector_new(1);
    char *buffer = strdup(str);     // have a working buffer
    char* tok = strtok(buffer,del);
    while(tok != NULL)
    {
        VVector_push(vec,strdup(tok));  //Push the string into the vector
        tok = strtok(NULL,del);          //next token
    }
    return vec;
}

/**
 * Tokenizer_ctor
 * Allocates and initializes a tokenizer
 * @param str String to tokenize
 * @param delimiters String of delimiters to delimit with
 */
static void Tokenizer_ctor( Tokenizer *thiz, const char *str, const char *del)
{
    // uses strtok as a base: TODO: implement own strtok
    // that uses a context so it's re-entrant
    Tokenizer_ctor_custom( thiz, str, strtok_wrapper, del );
}

/**
 * Tokenizer_dtor
 * Deconstructs the tokenizer
 */
static void Tokenizer_dtor( Tokenizer *thiz )
{
    if( thiz == NULL )
        return;
    //Free each string
    int size = thiz->length;
    char **strs = thiz->elements;
    for(int i = 0; i < size; i++)
    {
        free(strs[i]);
    }

    //Free the array itself;
    free(strs);
}

Tokenizer* Tokenizer_new(const char *str, const char *del)
{
    Tokenizer *tokenizer = malloc(sizeof(Tokenizer));

    Tokenizer_ctor( tokenizer, str, del );

    return tokenizer;
}

Tokenizer * Tokenizer_new_custom(const char *str,
                                 VVector * (* func)(const char *, void *),
                                 void * param)
{
    Tokenizer *tokenizer = malloc(sizeof(Tokenizer));

    Tokenizer_ctor_custom( tokenizer, str, func, param );

    return tokenizer;
}

void Tokenizer_delete(Tokenizer *thiz)
{
    if( thiz == NULL )
        return;
    Tokenizer_dtor( thiz );
    //Free the struct
    free(thiz);
}

// Get the next string in the tokenizer
// Returns null if no more tokens
// Returns pointer to newly allocated string otherwise
const char* Tokenizer_next(Tokenizer* thiz)
{
    if( thiz == NULL )
        return NULL;
    if(thiz->pos >= thiz->length)
    {
        return NULL;
    }
    char* output = thiz->elements[thiz->pos];
    thiz->pos++;
    return output;
}

char* Tokenizer_next_cpy(Tokenizer* thiz)
{
    if( thiz == NULL )
        return NULL;
    char *next = Tokenizer_next(thiz);
    return (next==NULL)? NULL : strdup(next);
}

// Peeks at next token, doesn't advance it
const char* Tokenizer_peek(Tokenizer* tokenizer)
{
    int len = tokenizer->length;
    if(tokenizer->pos >= len)
    {
        return 0x0;
    }
    char* output = tokenizer->elements[tokenizer->pos];
    return output;
}

char* Tokenizer_peek_cpy(Tokenizer* thiz)
{
    char *peek = Tokenizer_peek(thiz);
    return (peek==NULL)? NULL : strdup(peek);
}

// Checks if there are tokens remaining
int Tokenizer_hasTokens(Tokenizer* thiz)
{
    if(thiz->pos < thiz->length)
    {
        return 1;
    }
    return 0;
}

// Returns number of remaining tokens
int Tokenizer_countTokens(Tokenizer* thiz)
{
    return thiz->length - thiz->pos;
}

// Returns total number of tokens
int Tokenizer_numTokens(Tokenizer* thiz)
{
    return thiz->length;
}

// Resets the tokenizer to beginning
void Tokenizer_reset(Tokenizer* thiz)
{
    thiz->pos = 0;
}

// Checks if the thiz contains a certain string
// Returns 1 if true, 0 if false
int Tokenizer_contains(Tokenizer* thiz, const char* str)
{
    int len = thiz->length;
    for(int i = 0; i < len; i++)
    {
        if(strcmp(str,thiz->elements[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

const char * const * Tokenizer_tokens(Tokenizer *thiz)
{
    return thiz->elements;
}

// Returns a deep copy of all the tokens in an array
char** Tokenizer_tokens_cpy(Tokenizer* thiz)
{
    char** output = malloc(sizeof(char*) * thiz->length);
    int len = thiz->length;
    for(int i = 0; i < len; i++)
    {
        output[i] = strdup(thiz->elements[i]);
    }
    return output;
}

// Returns a deep copy of all the tokens in an array
void Tokenizer_populateArray(Tokenizer* thiz, char *array[])
{
    int len = thiz->length;
    for(int i = 0; i < len; i++)
    {
        array[i] = strdup(thiz->elements[i]);
    }
}

void Tokenizer_print(Tokenizer *thiz)
{
    int len = thiz->length;
    for(int i = 0; i < len; i++)
    {
        printf("Token #%d: %s\n", i, thiz->elements[i]);
    }
}
