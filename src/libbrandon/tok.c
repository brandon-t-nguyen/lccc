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


void Tokenizer_ctor( Tokenizer *this, const char *str, const char *del)
{
    if( this == NULL )
        return;
    VVector* vec = VVector_new(1);
    char *buffer = strdup(str);     // have a working buffer
    char* tok = strtok(buffer,del);
    while(tok != NULL)
    {
        VVector_push(vec,strdup(tok));  //Push the string into the vector
        tok = strtok(NULL,del);          //next token
    }

    this->pos = 0;
    this->length = VVector_length(vec);                 //get the size of the array returned
    this->elements = (char**)VVector_toArray_cpy(vec);  //get the array of strings
    VVector_delete(vec);    //free the vector
    free(buffer);
}

void Tokenizer_dtor( Tokenizer *this )
{
    if( this == NULL )
        return;
    //Free each string
    int size = this->length;
    char **strs = this->elements;
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

void Tokenizer_delete(Tokenizer *this)
{
    if( this == NULL )
        return;
    Tokenizer_dtor( this );
    //Free the struct
    free(this);
}

// Get the next string in the tokenizer
// Returns null if no more tokens
// Returns pointer to newly allocated string otherwise
const char* Tokenizer_next(Tokenizer* this)
{
    if( this == NULL )
        return NULL;
    if(this->pos >= this->length)
    {
        return NULL;
    }
    char* output = this->elements[this->pos];
    this->pos++;
    return output;
}

char* Tokenizer_next_cpy(Tokenizer* this)
{
    if( this == NULL )
        return NULL;
    char *next = Tokenizer_next(this);
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

char* Tokenizer_peek_cpy(Tokenizer* this)
{
    char *peek = Tokenizer_peek(this);
    return (peek==NULL)? NULL : strdup(peek);
}

// Checks if there are tokens remaining
int Tokenizer_hasTokens(Tokenizer* this)
{
    if(this->pos < this->length)
    {
        return 1;
    }
    return 0;
}

// Returns number of remaining tokens
int Tokenizer_countTokens(Tokenizer* this)
{
    return this->length - this->pos;
}

// Returns total number of tokens
int Tokenizer_numTokens(Tokenizer* this)
{
    return this->length;
}

// Resets the tokenizer to beginning
void Tokenizer_reset(Tokenizer* this)
{
    this->pos = 0;
}

// Checks if the this contains a certain string
// Returns 1 if true, 0 if false
int Tokenizer_contains(Tokenizer* this, const char* str)
{
    int len = this->length;
    for(int i = 0; i < len; i++)
    {
        if(strcmp(str,this->elements[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

const char * const * Tokenizer_tokens(Tokenizer *this)
{
    return this->elements;
}

// Returns a deep copy of all the tokens in an array
char** Tokenizer_tokens_cpy(Tokenizer* this)
{
    char** output = malloc(sizeof(char*) * this->length);
    int len = this->length;
    for(int i = 0; i < len; i++)
    {
        output[i] = strdup(this->elements[i]);
    }
    return output;
}

// Returns a deep copy of all the tokens in an array
void Tokenizer_populateArray(Tokenizer* this, char *array[])
{
    int len = this->length;
    for(int i = 0; i < len; i++)
    {
        array[i] = strdup(this->elements[i]);
    }
}

void Tokenizer_print(Tokenizer *this)
{
    int len = this->length;
    for(int i = 0; i < len; i++)
    {
        printf("Token #%d: %s\n", i, this->elements[i]);
    }
}
