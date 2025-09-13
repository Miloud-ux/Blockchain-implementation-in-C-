#include <assert.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#define MAX_SIZE 1024
#define MAX_HASH_SIZE 65

// MAX chars for SHA-256 is MAX_HASH_SIZE it's the size of the digest (output)
#define MAX_CONCATINATED_SIZE 2000
typedef struct Block {
  int index; // the height of the block
  char timestamps[20];
  char data[MAX_SIZE];
  char current_hash[MAX_HASH_SIZE];
  char prev_hash[MAX_HASH_SIZE];
  struct Block *next_block;
} Block;

void free_chain(Block **genesis_block);
void test_invalid_chain(Block *genesis_block); // for debugging
void spinning_loading();
void spinning_loading_util(char spin_char);

bool validate_block(Block **genesis_block);
bool validate_block_components(Block **block);
bool validate_block_chain(Block *block);
void delete_invalid_block(Block **genesis_block);

void set_time_stamps(Block *myblock) {
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  strftime(myblock->timestamps, 20, "%H:%M:%S", timeinfo);
}

void hash_func(Block *myblock) {
  // Concatinate all the fields in a single string
  char con_data[MAX_CONCATINATED_SIZE];
  int char_written =
      snprintf(con_data, sizeof(con_data), "%d%s%s%s", myblock->index,
               myblock->timestamps, myblock->prev_hash, myblock->data);
  if (char_written >= MAX_CONCATINATED_SIZE) {
    perror("Buffer overflow");
    exit(1);
  }
  // Hashing
  unsigned char digest[MAX_HASH_SIZE];
  SHA256((unsigned char *)con_data, strlen(con_data), digest);

  // Convert the raw hash into hexa string
  char hex[65];
  for (size_t i = 0; i < MAX_HASH_SIZE; i++) {
    sprintf(hex + (i * 2), "%02x", digest[i]);
  }
  hex[64] = '\0';

  // copy the hash into the block
  strcpy(myblock->current_hash, hex);
}

void init_new_block(Block *genesis_block) {
  set_time_stamps(genesis_block);
  genesis_block->index = 0;
  genesis_block->next_block = NULL; // 0 = NULL
  strcpy(genesis_block->prev_hash, "0");
  strcpy(genesis_block->current_hash, "0");
  strcpy(genesis_block->data, "0");
}

void create_new_block(Block **genesis_block, char data[]) {
  // we only need the data to be passed as a parameter
  // Block** pass the genensis block by reference since it's local to the main

  if (*genesis_block == NULL) {
    perror("The genesis block doesn't exist");
    return;
  }

  Block *temp = *genesis_block;
  int index = 0;
  while (temp->next_block != NULL) {
    temp = temp->next_block;
    index++;
  }
  // temp is at the end of the list
  // we create our block and initialize.
  Block *new_block = (Block *)malloc(sizeof(Block));
  if (!new_block) {
    perror("Error Allocating data for the new block");
    exit(EXIT_FAILURE);
  }
  init_new_block(new_block); // to avoid garbage values
  strcpy(new_block->data, data);
  hash_func(new_block);
  new_block->index = index + 1;

  // now link the blocks and fill the "prev hash" field
  temp->next_block = new_block;
  strcpy(new_block->prev_hash, temp->current_hash);
}

void create_new_block_wrapper(Block **genesis_block) {
  while (1) {
    system("clear");
    char answer;
    printf("Do you want to Create a new block  ? (Y/n)");
    scanf(" %c", &answer); // leave space to clear the buffer
    if (tolower(answer) == 'n') {
      printf("Have a good day");
      return;
    } else if (tolower(answer) == 'y') {
      printf("Enter the data : ");
      char data[MAX_SIZE];
      fgets(data, MAX_SIZE, stdin);
      create_new_block(genesis_block, data);
      if (!validate_block(genesis_block)) {
        printf("New block invalid !");
        // Add an ASCII spinner
      }
      delete_invalid_block(genesis_block);
      printf("Done");
    } else {
      fprintf(stderr, "Syntax Error :Unknown character");
      return;
    }
  }
}
int main() {
  // Create the first block (Genesis block)
  Block *genesis_block;
  genesis_block = malloc(sizeof(Block));
  if (!genesis_block) {
    perror("Error allocating memory");
    return 1;
  }

  // init the genesis block
  init_new_block(genesis_block);

  // Assign the hash for the genesis block
  hash_func(genesis_block);

  // Interactive block creation:
  test_invalid_chain(genesis_block);
  free_chain(&genesis_block);
  // create_new_block_wrapper(&genesis_block);

  // free the whole chain

  return 0;
}

bool validate_block_chain(Block *genesis_block) {
  // Traverse the linked list and compare each hash with the one after
  // We use "fast and slow pointers"
  if (genesis_block == NULL) {
    fprintf(stderr, "Chain Error : Genesis block doesn't exist or is NULL");
    return false;
  }
  Block *slow = genesis_block;
  Block *fast = genesis_block->next_block;

  // Edge case (1 block)
  if (genesis_block->next_block == NULL) {
    // Sicne it's the genesis block there's no way to manipulate it
    return true;
  }
  // Edge case2 : (2 blocks)
  if (fast && fast->next_block == NULL) {
    return strcmp(slow->current_hash, fast->prev_hash) == 0;
  }

  while (slow->next_block->next_block != NULL && fast->next_block != NULL) {
    if (strcmp(slow->current_hash, fast->prev_hash) != 0) {
      fprintf(stderr, "Chain Error : Hashes don't match ");
      return false;
    }
    slow = slow->next_block;
    fast = fast->next_block;
  }
  return true;
}

bool validate_block_components(Block **genesis_block) {
  // Validate the hash of the current block
  // 1. Recompute the hash
  // 2. Compare with the provided hash
  // note: instead of creating a new function to calculate the hash and somehow
  // "return" the string we can simply save the current hash in a seperate
  // string variable and then write the freshly computed hash into the block and
  // then compare it to the string var :
  // - If they match : correct hash
  // - Else : modified hash (fake)

  if (genesis_block == NULL) {
    return false;
  }

  // traverse to the last block (last added block)
  Block *temp = *genesis_block;
  while (temp->next_block != NULL) {
    temp = temp->next_block;
  }

  char temp_hash[MAX_HASH_SIZE] = {0};
  strcpy(temp_hash, temp->current_hash);

  hash_func(temp);
  if (strcmp(temp_hash, temp->current_hash) != 0) {
    fprintf(stderr,
            "Component Error: provided hash doesn't match computed hash");
    return false;
  }

  // Step2 :  compare the height of the blocks
  int temp_index = temp->index;
  temp = *genesis_block;
  while (temp->next_block->next_block) {
    temp = temp->next_block;
  }

  if (temp_index <= temp->index) {
    fprintf(stderr, "Component Error : Height of the current block doesn't  ");
    return false;
  }

  return true;
}

bool validate_block(Block **genesis_block) {

  // == validate_block_components ==
  // "current hash" of current block = "current hash saved in the struct"
  // (compare strings)
  //  "index" of current block = "index" of next block - 1

  // == validate_block_chain ==
  // "current hash" of current block = "prev hash" of the next block

  // == Log errors ==
  // To log errors we are using pass by reference to detect where the erros is
  // instead of a general error message

  // == note ==
  // we don't need to track the  index of the block to be deleted since we are
  // checking everytime a block gets added so obv it's always the last block.

  Block *latest_block = *genesis_block;
  while (latest_block->next_block != NULL) {
    latest_block = latest_block->next_block;
  }

  if (!validate_block_components(&latest_block)) {
    return false;
  } else if (!validate_block_chain(*genesis_block)) {
    return false;
  }
  printf("Log : >>Valid block \n");
  return true;
}

void delete_invalid_block(Block **genesis_block) {
  // 1. Loop through the linked list till the block before the end
  // 2. Set the next_block == NULL
  if (genesis_block == NULL || (*genesis_block)->next_block == NULL) {
    // nothing to delete here
    return;
  }

  // Case 2 : two blocks only
  Block *temp_block = *genesis_block;
  if (temp_block->next_block->next_block == NULL) {
    free(temp_block);
    temp_block->next_block = NULL;
    return;
  }

  if (temp_block->next_block->next_block == NULL) {
    temp_block = temp_block->next_block;
  }
  temp_block->next_block = NULL;
}

void spinning_loading() {
  const char *theme_1 = "⣾⣽⣻⢿⡿⣟⣯⣷";
  // const char* theme_2 = ['◴','◷','◶','◵'];
  for (int i = 0; i < strlen(theme_1); i++) {
    spinning_loading_util(theme_1[i]);
  }
}
void spinning_loading_util(char spin_char) {
  printf("%c\r", spin_char);
  fflush(stdout);
  usleep(40000);
}
void test_invalid_chain(Block *genesis_block) {
  Block *chain = genesis_block;
  create_new_block(&chain, "A");
  create_new_block(&chain, "B");

  // Test 1: Corrupt hash
  strcpy(chain->next_block->current_hash, "0000000000");
  assert(validate_block(&chain) == false);

  // Test 2: Break link
  strcpy(chain->next_block->prev_hash, "broken_link");
  assert(validate_block_chain(chain) == false);
}

void free_chain(Block **genesis_block) {
  if (*genesis_block == NULL) {
    printf("Chain Error : Genesis block doesn't exist or is NULL");
    return;
  }
  Block *current = *genesis_block;
  Block *temp;
  while (current != NULL) {
    temp = current;
    current = current->next_block;
    free(temp);
  }
  printf("Chain freed");
}
