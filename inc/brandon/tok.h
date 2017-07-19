#ifndef __TOK_H__
#define __TOK_H__

#ifdef __cplusplus
extern "C"{
#endif

//Provides a tokenizer struct
struct tokenizer_str;
typedef struct tokenizer_str Tokenizer;


/**
 * Tokenizer_ctor
 * Allocates and initializes a tokenizer
 * @param str String to tokenize
 * @param delimiters String of delimiters to delimit with
 */
void Tokenizer_ctor( Tokenizer *thiz, const char *str, const char *delimiters );

/**
 * Tokenizer_dtor
 * Deconstructs the tokenizer
 */
void Tokenizer_dtor( Tokenizer *thiz );

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
void Tokenizer_delete(Tokenizer *thiz);

/**
 * Tokenizer_next
 * Get the next string in the tokenizer
 * The returned string one owned by the tokenizer
 * Use Tokenizer_next_cpy for caller's copy
 * @return null if no more tokens, else returns pointer to
 *         const string of the token owned by the Tokenizer
 */
const char * Tokenizer_next(Tokenizer *thiz);

/**
 * Tokenizer_next_cpy
 * Like Tokenizer_next, but returns a copy of the string
 */
char * Tokenizer_next_cpy(Tokenizer *thiz);

/**
 * Tokenizer_peek
 * Peeks at next token, doesn't advance it
 * The returned string is owned by the Tokenizer
 * @return null if no more tokens, else returns pointer to const string of token
 */
const char * Tokenizer_peek(Tokenizer *thiz);

/**
 * Tokenizer_hasTokens
 * Checks if tokens remain
 * @return 1 if tokens remain, 0 if they don't
 */
// Checks if there are tokens remaining
int Tokenizer_hasTokens(Tokenizer *thiz);

/**
 * Tokenizer_countTokens
 * Returns number of remaining tokens
 * @return number of remaining tokens
 */
// Returns number of remaining tokens
int Tokenizer_countTokens(Tokenizer *thiz);

/**
 * Tokenizer_numTokens
 * Returns total number of tokens
 * @return totla number of tokens
 */
int Tokenizer_numTokens(Tokenizer *thiz);

/**
 * Tokenizer_reset
 * Resets the tokenizer to beginning
 */
void Tokenizer_reset(Tokenizer *thiz);

/**
 * Tokenizer_contains
 * Checks if the tokenizer contains a certain string
 * @return 1 if true, 0 if false
 */
int Tokenizer_contains(Tokenizer *thiz, const char *str);

/**
 * Tokenizer_tokens
 * Returns the array of strings.
 * @param array Array to populate
 */
const char * const * Tokenizer_tokens(Tokenizer *thiz);

/**
 * Tokenizer_tokens
 * Returns a deep copy of all the tokens in an array
 * @returns deep copy array of tokens: caller must deallocate these
 */
char **Tokenizer_tokens_cpy(Tokenizer *thiz);

/**
 * Tokenizer_populateArray
 * Fills an array with const pointers to the tokens. Copies.
 * Pair this with numTokens to get a properly sized array.
 * @param array Array to populate
 */

void Tokenizer_populateArray(Tokenizer *thiz, char *array[]);

void Tokenizer_print( Tokenizer *thiz );

#ifdef __cplusplus
}
#endif
#endif
