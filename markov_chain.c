#include <stdlib.h>
#include "markov_chain.h"
#include <string.h>
#include <assert.h>

void free_single_markov_node (const MarkovChain *markov_chain, Node *cur_node);
// See full documentation in header file
int get_random_number (int max_number)
{
  return rand () % max_number;
}

// See full documentation in header file
MarkovNode *get_first_random_node (MarkovChain *markov_chain)
{
  Node *random_node = get_node (markov_chain);
  while (markov_chain->is_last (random_node->data->data) == true)
  {
    random_node = get_node (markov_chain);
  }
  return random_node->data;
}

Node *get_node (MarkovChain *markov_chain)
{
  Node *cur = markov_chain->database->first;
  int random_number = get_random_number (markov_chain->database->size);
  for (int i = 0; i < random_number; i++)
  {
    cur = cur->next;
  }
  return cur;
}

// See full documentation in header file
MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr)
{
  int random_number = get_random_number (state_struct_ptr->sum_of_frequencies);
  int i = 0;
  NextNodeCounter *cur = state_struct_ptr->counter_list[i];
  int number_of_occurences_left =
      state_struct_ptr->counter_list[i]->frequency;
  while (random_number > 0)
  {
    number_of_occurences_left--;
    if (number_of_occurences_left == 0)
    {
      i++;
      cur = state_struct_ptr->counter_list[i];
      number_of_occurences_left =
          state_struct_ptr->counter_list[i]->frequency;
    }
    random_number--;
  }
  return cur->markov_node;
}

void generate_random_sequence (MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  print_first_node (markov_chain, &first_node, &max_length);
  MarkovNode *next_node = get_next_random_node (first_node);
  max_length--;
  print_next_nodes (max_length, next_node, markov_chain);
}

void print_first_node (MarkovChain *markov_chain, MarkovNode **first_node,
                       int *max_length)
{
  if ((*first_node) != NULL)
  {
    markov_chain->print_func ((*first_node)->data);
    (*max_length)--;
  }
  else
  {
    (*first_node) = get_first_random_node (markov_chain);
    markov_chain->print_func ((*first_node)->data);
    (*max_length)--;
  }
}

void print_next_nodes (int max_length, MarkovNode *next_node, MarkovChain
*markov_chain)
{
  while (max_length >= 0)
  {
    if (markov_chain->is_last (next_node->data) == true)
    {
      markov_chain->print_func (next_node->data);
      return;
    }
    markov_chain->print_func (next_node->data);
    next_node = get_next_random_node (next_node);
    max_length--;
  }
}

// See full documentation in header file
void free_markov_chain (MarkovChain **ptr_chain)
{
  free_markov_nodes (*ptr_chain);
  free ((*ptr_chain)->database);
  free (*ptr_chain);
}

// See full documentation in header file
bool add_node_to_counter_list (MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain)
{
  if (first_node->counter_list == NULL)
  {
    if (init_counter_list (first_node) == false)
    { return false; }
  }
  NextNodeCounter **counter_list_array = first_node->counter_list;
  NextNodeCounter *node = update_if_object_in_counter_list
      (first_node, second_node, markov_chain);
  if (node == NULL)
  {
    NextNodeCounter *next_object_node = init_next_object_node (second_node);
    if (next_object_node == NULL)
    { return false; }
    counter_list_array[first_node->length_of_counter_list] = next_object_node;
    update_node_counter_list_fields (first_node);
    first_node->counter_list = realloc (first_node->counter_list,
                                        (first_node
                                             ->length_of_counter_list
                                         + 1)
                                        * sizeof (NextNodeCounter *));
    if (first_node->counter_list == NULL)
    { return false; }
  }
  return true;
}

bool init_counter_list (MarkovNode *markov_node)
{
  NextNodeCounter **counter_list_array = malloc (sizeof (NextNodeCounter *));
  if (counter_list_array == NULL)
  { return false; }
  markov_node->counter_list = counter_list_array;
  markov_node->length_of_counter_list = 0;
  markov_node->sum_of_frequencies = 0;
  return true;
}

// See full documentation in header file
NextNodeCounter *update_if_object_in_counter_list (MarkovNode *node_to_search_in,
                                                   MarkovNode *node_to_search_for
                                                 , MarkovChain *markov_chain)
{
  int i = 0;
  if (node_to_search_in->length_of_counter_list == 0)
  { return NULL; }
  while (i != node_to_search_in->length_of_counter_list)
  {
    NextNodeCounter *cur_node = node_to_search_in->counter_list[i];
    if (markov_chain->comp_func (node_to_search_for->data,
                                 cur_node->markov_node->data)
        == 0)
    {
      cur_node->frequency++;
      node_to_search_in->sum_of_frequencies++;
      return cur_node;
    }
    i++;
  }
  return NULL;
}

NextNodeCounter *init_next_object_node (MarkovNode *next_node)
{
  NextNodeCounter
      *new_node_in_counter_list = malloc (sizeof (NextNodeCounter));
  if (new_node_in_counter_list == NULL)
  { return NULL; }
  new_node_in_counter_list->markov_node = next_node;
  new_node_in_counter_list->frequency = 1;
  return new_node_in_counter_list;
}

void update_node_counter_list_fields (MarkovNode *first_node)
{
  first_node->sum_of_frequencies++;
  first_node->length_of_counter_list++;
}

// See full documentation in header file
Node *add_to_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *existing_node = get_existing_node (markov_chain, data_ptr);
  if (existing_node != NULL)
  { return existing_node; }
  void *data_ptr_cpy = markov_chain->copy_func (data_ptr);
  MarkovNode *markov_node = init_markov_node (data_ptr_cpy);
  if (markov_node == NULL)
  { return NULL; }
  if (add (markov_chain->database, markov_node) == 1)
  { return NULL; }
  return markov_chain->database->last;
}

Node *get_existing_node (MarkovChain *markov_chain, void *data_ptr)
{
  Node *cur = markov_chain->database->first;
  int counter = 0;
  while (counter != markov_chain->database->size)
  {
    if (markov_chain->comp_func (data_ptr, cur->data->data) == 0)
    { return cur; }
    cur = cur->next;
    counter++;
  }
  return NULL;
}

MarkovNode *init_markov_node (void *data_ptr)
{
  MarkovNode *markov_node = malloc (sizeof (MarkovNode));
  if (markov_node == NULL)
  { return NULL; }
  markov_node->data = data_ptr;
  markov_node->length_of_counter_list = 0;
  markov_node->sum_of_frequencies = 0;
  markov_node->counter_list = NULL;
  return markov_node;
}

void free_markov_nodes (MarkovChain *markov_chain)
{
  Node *cur_node = markov_chain->database->first;
  Node *next_node = markov_chain->database->first->next;
  while (next_node != NULL)
  {
    free_single_markov_node (markov_chain, cur_node);
    cur_node = next_node;
    next_node = next_node->next;
  }
  free_single_markov_node (markov_chain, cur_node);
}
void free_single_markov_node (const MarkovChain *markov_chain, Node *cur_node)
{
  MarkovNode *cur_markov_node = cur_node->data;
  markov_chain->free_data (cur_markov_node->data);
  for (int i = 0; i < cur_markov_node->length_of_counter_list; i++)
  {
    free (cur_markov_node->counter_list[i]);
  }
  free (cur_markov_node->counter_list);
  free (cur_markov_node);
  free (cur_node);
}

// See full documentation in header file
Node *get_node_from_database (MarkovChain *markov_chain, void *data_ptr)
{
  int i = 0;
  Node *cur = markov_chain->database->first;
  while (i != markov_chain->database->size)
  {
    if (markov_chain->comp_func (data_ptr, cur->data->data) == 0)
    { return cur; }
    cur = cur->next;
    i++;
  }
  return NULL;
}