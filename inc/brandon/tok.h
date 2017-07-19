#ifndef __TOK_H__
#define __TOK_H__

//Provides a tokenizer struct
struct tokenizer_str;
typedef struct tokenizer_str Tokenizer;


/**
 * Tokenizer_ctor
 * Allocates and initializes a tokenizer
 * @param str String to tokenize
 * @param delimiters String of delimiters to delimit with
 */
void Tokenizer_ctor( Tokenizer *this, const char *str, const char *delimiters );

/**
 * Tokenizer_dtor
 * Deconstructs the tokenizer
 */
void Tokenizer_dtor( Tokenizer *this );

/**
 * Tokenizer_new
 * Allocates and initializes a tokenizer
 * @param str String to tokenize
 * @param delimiters String of delimiters to delimit with
 * @return New Tokenizer
 */
Tokenizer * Tokenizer_new(const char *str, const char *delimiters);

/**
 * Tokenizer_delete
 * Deletes a tokenizer
 */
void Tokenizer_delete(Tokenizer *this);

/**
 * Tokenizer_next
 * Get the next string in the tokenizer
 * @return null if no more tokens, else returns pointer to const string of the token
 */
const char * Tokenizer_next(Tokenizer *this);

/**
 * Tokenizer_next_cpy
 * Like Tokenizer_next, but returns a copy of the string
 */
char * Tokenizer_next_cpy(Tokenizer *this);

/**
 * Tokenizer_peek
 * Peeks at next token, doesn't advance it
 * @return null if no more tokens, else returns pointer to const string of token
 */
const char * Tokenizer_peek(Tokenizer *this);

/**
 * Tokenizer_hasTokens
 * Checks if tokens remain
 * @return 1 if tokens remain, 0 if they don't
 */
// Checks if there are tokens remaining
int Tokenizer_hasTokens(Tokenizer *this);

/**
 * Tokenizer_countTokens
 * Returns number of remaining tokens
 * @return number of remaining tokens
 */
// Returns number of remaining tokens
int Tokenizer_countTokens(Tokenizer *this);

/**
 * Tokenizer_numTokens
 * Returns total number of tokens
 * @return totla number of tokens
 */
int Tokenizer_numTokens(Tokenizer *this);

/**
 * Tokenizer_reset
 * Resets the tokenizer to beginning
 */
void Tokenizer_reset(Tokenizer *this);

/**
 * Tokenizer_contains
 * Checks if the tokenizer contains a certain string
 * @return 1 if true, 0 if false
 */
int Tokenizer_contains(Tokenizer *this, const char *str);

/**
 * Tokenizer_tokens
 * Returns the array of strings.
 * @param array Array to populate
 */
const char * const * Tokenizer_tokens(Tokenizer *this);

/**
 * Tokenizer_tokens
 * Returns a deep copy of all the tokens in an array
 * @returns deep copy array of tokens: caller must deallocate these
 */
char **Tokenizer_tokens_cpy(Tokenizer *this);

/**
 * Tokenizer_populateArray
 * Fills an array with const pointers to the tokens. Copies.
 * Pair this with numTokens to get a properly sized array.
 * @param array Array to populate
 */

void Tokenizer_populateArray(Tokenizer *this, char *array[]);

void Tokenizer_print( Tokenizer *this );

#endif
