#include "markov_chain.h"
#include "linked_list.h"
#include <stdlib.h>
#include <unistd.h>
#include "stdio.h"
#include "stdbool.h"
#include "string.h"

#define ARGC_ERROR_MSG "USAGE: Insufficient program arguments. Please enter"\
" seed value, number of tweets, path of text corpus and number of words to"\
" read (optional).\n"
#define MAX_NUMBER_OF_WORDS_IN_TWEET 20
#define TWEET_PRINT_STRING "Tweet %i: "
#define INVALID_FILE_ERROR_MSG "ERROR: Invalid file."
#define VALID_ARGC 4
#define LINE_LENGTH_BUFFER 1000
#define BASE_10 10
#define ARGC_IS_FIVE 5
#define ALLOC_FAIL 0
#define ALLOC_SUCCESS 1

// Function declarations start here:
static bool test_command_line_input (const int *argc);
static bool test_argv_txt_file (const char **arg3);
static bool run_input_tests (const int *argc, const char **arg3);
static bool chain_allocation_is_unsuccessful (void *object);
static unsigned int convert_arg_to_unsigned_int (const char **argv);
static int init_words_to_read (int argc, const char **arg4);
static void init_markov_chain (MarkovChain *markov_chain);
static void print_function (void *data);
static int comp_function (void *first, void *second);
static void free_data_function (void *data);
static void *copy_function (void *src);
static bool is_last_function (void *data);
static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain);
static void remove_newline_chars (char *data_ptr);
static bool init_linked_list (MarkovChain *markov_chain);
static bool create_counter_list (Node *word_node, Node *next_word_node,
                                 MarkovChain *markov_chain);
static bool add_last_word_of_line (MarkovChain *markov_chain, char *word);
static bool add_words_to_linked_list_and_create_counter_list (MarkovChain
                                                              *markov_chain,
                                                              char *word,
                                                              char *next_word);
static bool
fill_database_alloc_unsuccessful (int result, MarkovChain *markov_chain);
static void
generate_tweets (const char **arg1, MarkovChain *markov_chain, const
char **arg2);
// Function declarations ends here.

/**
 * Program driver. Does several tasks:
 * 1. Tests for CLI validity.
 * 2. Creates and fills the Markov Chain database which includes a
 * Linked List of words, each with their own Markov Nodes. The Markov Nodes
 * hold more data structures that contain all the words that follow said
 * word and their frequencies.
 * 3. Generates random tweets using the Markov Chain Database.
 * @return EXIT-SUCCESS if program runs successfully, EXIT_FAILURE - if any
 * allocation issues arise. In the latter case an allocation error is printed.
 */
int main (const int argc, const char *argv[])
{
  if (run_input_tests (&argc, &argv[3]) == false)
  { return EXIT_FAILURE; }
  int words_to_read = init_words_to_read (argc, &argv[4]);
  MarkovChain *markov_chain = malloc (sizeof (MarkovChain));
  init_markov_chain (markov_chain);
  if (chain_allocation_is_unsuccessful (markov_chain))
  { return EXIT_FAILURE; }
  FILE *input_file;
  input_file = fopen (argv[3], "r");
  int result = fill_database (input_file, words_to_read, markov_chain);
//  print_linked_list (markov_chain);
  if (fill_database_alloc_unsuccessful (result, markov_chain) == true)
  { return EXIT_FAILURE; }
  generate_tweets (&argv[1], markov_chain, &argv[2]);
  free_markov_chain (&markov_chain);
  return EXIT_SUCCESS;
}

/**
 * If fill_database function has an allocation issue, this function will
 * free all relevant databases, and print the allocation error.
 * @return true if allocation was a failure, false if all allocations
 * succeeded.
 */
static bool
fill_database_alloc_unsuccessful (int result, MarkovChain *markov_chain)
{
  if (result == ALLOC_FAIL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    free_markov_chain (&markov_chain);
    return true;
  }
  return false;
}

static bool chain_allocation_is_unsuccessful (void *object)
{
  if (!object)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    return true;
  }
  return false;
}

static bool run_input_tests (const int *argc, const char **arg3)
{
  if (test_command_line_input (&*argc) == false || test_argv_txt_file
                                                       (&*arg3) == false)
  { return false; }
  return true;
}

/**
 * CLI is valid if there are 3 or 4 arguments; arg1 - seed, arg2 - number of
 * tweets to print, arg3 - input file, arg4 - number of words to read from
 * input file. The last argument is optional. When omitted, program will
 * read entire file.
 * @return true if test passes, false if test fails.
 */
static bool test_command_line_input (const int *argc)
{
  if (*argc != VALID_ARGC && *argc != VALID_ARGC + 1)
  {
    printf (ARGC_ERROR_MSG);
    return false;
  }
  return true;
}

/**
 * Tests file validity for two issues:
 *      1. file exists but cannot be read.
 *      2. file does not exist.
 * @return true if test passes, false if test fails.
 */
static bool test_argv_txt_file (const char **arg3)
{
  if (access (*arg3, R_OK) == -1)
    // file exists but can't be read
  {
    printf (INVALID_FILE_ERROR_MSG);
    return false;
  }
  if (access (*arg3, F_OK) == -1) // file doesn't exist
  {
    printf (INVALID_FILE_ERROR_MSG);
    return false;
  }
  return true;
}

static unsigned int convert_arg_to_unsigned_int (const char **argv)
{
  unsigned int argv_int = (int) strtol (*argv, NULL, BASE_10);
  return argv_int;
}

/**
 * Determines the number of words to read.
 * @return 0 if no argument was given and the entire file is to be read.
 * any other integer returned specifies how many words to read.
 */
static int init_words_to_read (int argc, const char **arg4)
{
  int words_to_read = 0;
  if (argc == ARGC_IS_FIVE)
  { words_to_read = (int) convert_arg_to_unsigned_int (&*arg4); }
  return words_to_read;
}

static void init_markov_chain (MarkovChain *markov_chain)
{
  markov_chain->print_func = &print_function;
  markov_chain->comp_func = &comp_function;
  markov_chain->free_data = &free_data_function;
  markov_chain->copy_func = &copy_function;
  markov_chain->is_last = &is_last_function;
}

static void print_function (void *data)
{
  char *data_char = data;
  if (data_char[strlen (data_char) - 1] == '.')
  {
    printf ("%s", data_char);
    return;
  }
  printf ("%s ", data_char);
}

static int comp_function (void *first, void *second)
{
  if (!first || !second)
  { return 1; }
  return strcmp (first, second);
}

static void free_data_function (void *data)
{
  free ((char *) data);
}

static void *copy_function (void *src)
{
  if (!src)
  { return NULL; }
  char *src_char = src;
  char *str;
  char *p;
  int len = 0;
  while (src_char[len] != '\0')
  {
    len++;
  }
  str = malloc (len + 1);
  p = str;
  while (*src_char)
  {
    *p++ = *src_char++;
  }
  *p = '\0';
  return str;
}

static bool is_last_function (void *data)
{
  char *word = data;
  if (word[strlen (word) - 1] == '.')
  { return true; }
  return false;
}

/**
 * Fills database by calling two functions, add_words_to_linked_list_
 * and_create_counter_list, and add_to_database.
 * @return ALLOC_FAIL if there is an allocation fail, ALLOC_SUCCESS otherwise.
 */
static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  if (init_linked_list (markov_chain) == false)
  { return ALLOC_FAIL; }
  char line[LINE_LENGTH_BUFFER];
  int words_read = 0;
  while (fgets (line, LINE_LENGTH_BUFFER, fp))
  {
    char *word = strtok (line, " "), *next_word = strtok (NULL, " ");
    remove_newline_chars (word);
    remove_newline_chars (next_word);
    if (words_to_read - words_read == 1 && words_to_read != 0)
    {
      if (add_last_word_of_line (markov_chain, word) == false)
      { return ALLOC_FAIL; }
      return ALLOC_SUCCESS;
    }
    if (add_words_to_linked_list_and_create_counter_list (markov_chain, word,
                                                          next_word) == false)
    { return ALLOC_FAIL; }
    words_read += 2;
    while (word != NULL && next_word != NULL)
    {
      if (words_read >= words_to_read && words_to_read != 0)
        // words to read = 0 when entire file must be read.
      { return true; }
      word = next_word, next_word = strtok (NULL, " ");
      remove_newline_chars (next_word);
      if (next_word != NULL)
      {
        if (add_words_to_linked_list_and_create_counter_list (markov_chain,
                                                              word, next_word)
            == false)
        { return ALLOC_FAIL; }
      }
      else
      {
        if (add_last_word_of_line (markov_chain, word) == false)
        { return ALLOC_FAIL; }
      }
      words_read += 1;
    }
  }
  return ALLOC_SUCCESS;
}

/**
 * Adds two words to the linked list at a time, and builds the counter list
 * of the first word by calling the function create_counter_list.
 */
static bool add_words_to_linked_list_and_create_counter_list (MarkovChain
                                                         *markov_chain,
                                                         char *word,
                                                         char *next_word)
{
  Node *word_node = add_to_database (markov_chain, word);
  Node *next_word_node = add_to_database (markov_chain, next_word);
  if (word_node == NULL || next_word_node == NULL)
  { return false; }
  if (create_counter_list (word_node, next_word_node, markov_chain) == false)
  {
    return false;
  }
  return true;
}

static bool add_last_word_of_line (MarkovChain *markov_chain, char *word)
{
  Node *word_node = add_to_database (markov_chain, word);
  if (word_node == NULL)
  { return false; }
  return true;
}

static void remove_newline_chars (char *data_ptr)
{
  if (!data_ptr)
  { return; }
  if (data_ptr[strlen (data_ptr) - 1] == '\n')
  { data_ptr[strlen (data_ptr) - 1] = '\0'; }
}

static bool init_linked_list (MarkovChain *markov_chain)
{
  LinkedList *linked_list = malloc (sizeof (LinkedList));
  if (linked_list == NULL)
  { return ALLOC_FAIL; }
  linked_list->first = NULL;
  linked_list->last = NULL;
  linked_list->size = 0;
  markov_chain->database = linked_list;
  return ALLOC_SUCCESS;
}

static bool create_counter_list (Node *word_node, Node *next_word_node,
                                 MarkovChain *markov_chain)
{
  if (add_node_to_counter_list (word_node->data, next_word_node->data,
                                markov_chain) == false)
  { return false; }
  return true;
}

/**
 * Randomly selects first word, and builds and prints all tweets.
 */
static void generate_tweets (const char **arg1, MarkovChain *markov_chain,
                             const
                             char **arg2)
{
  srand (convert_arg_to_unsigned_int (&*arg1));
  int tweet_number = 1, num_of_tweets_to_create =
      (int) convert_arg_to_unsigned_int (&*arg2);
  MarkovNode *first_random_node = get_first_random_node (markov_chain);
  while (tweet_number != num_of_tweets_to_create + 1)
  {
    printf (TWEET_PRINT_STRING, tweet_number);
    generate_random_sequence (markov_chain, first_random_node,
                              MAX_NUMBER_OF_WORDS_IN_TWEET);
    printf ("\n");
    tweet_number++;
    first_random_node = NULL;
  }
}