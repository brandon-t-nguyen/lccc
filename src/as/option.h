#ifndef __OPTION_H__
#define __OPTION_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct option
{
    const char * s_arg; // short arg
    const char * l_arg; // long form
    bool assign;        // is an assignment: looks for =
    // function to run when this option is used
    // has control to increment arg_idx to consume additional spaced arguments
    // if NULL, this option becomes annotation text for --help
    //      if assign is true, it starts a section of arguments or ends one if
    //      it is already in a section. In the case a section has already started,
    //      d_str will be irrelevant
    int (* func)(void * aux, const char * arg, const char * assign);
    const char * p_str; // help parameter for option
    const char * pd_str;// default option (NULL if not relevant)
    const char * d_str; // help details for option
} option;

void print_option(const option * opt);
void print_options(const option * options, size_t num_options);

/**
 * Parses the list of options, running the option function if there's a match
 * @param[in]   src         String to indicate the source of any errors on a print.
 *                          See print.h's msg functions
 * @param[in]   options     Array of options to check against
 * @param[in]   num_options Number of options in the array
 * @param[in]   aux         Auxillary info (e.g. driver params) that are passed
 *                          onto the option function
 * @param[in]   argc        Number of command line arguments
 * @param[in]   argv        Command line arguments
 */
void parse_options(const char * msg_src,
                   const option * options, size_t num_options,
                   void * aux, vector * non_options, int argc, char ** argv);

#ifdef __cplusplus
}
#endif

#endif//__OPTION_H__
