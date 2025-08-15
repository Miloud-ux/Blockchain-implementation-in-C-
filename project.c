#include <ctype.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#define MAX_SIZE 1024
#define MAX_HASH_SIZE                                                          \
  32 // MAX chars for SHA-256 is MAX_HASH_SIZE it's the size of the digest
     // (output)
#define MAX_CONCATINATED_SIZE 2000

typedef struct Block {
  int index; // the height of the block
  char timestamps[20];
  char data[MAX_SIZE];
  char current_hash[MAX_HASH_SIZE];
  char prev_hash[MAX_HASH_SIZE];
  struct Block *next_block;
} Block;

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
    } else {
      printf("Unknown character");
      return;
    }
  }
}

bool valiate_block(Block *block);
bool validate_block_components(Block *block, int *height);
bool validate_block_chain(Block *block);

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
  create_new_block_wrapper(&genesis_block);

  return 0;
}

bool validate_block_chain(Block *genesis_block) {
  // Traverse the linked list and compare each hash with the one after
  // We use "fast and slow pointers"
  if (genesis_block == NULL) {
    printf(stderr, "Chain Error : Genesis block doesn't exist or is NULL");
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
      printf(stderr, "Chain Error : Hashes don't match ");
      return false;
    }
  }
  return true;
}

bool validate_block_components(Block *block, int *height) {
  // Validate the hash of the current block
  // 1. Recompute the hash
  // 2. Compare with the provided hash
  // note: instead of creating a new function to calculate the hash and somehow
  // "return" the string we can simply save the current hash in a seperate
  // string variable and then write the freshly computed hash into the block and
  // then compare it to the string var :
  // - If they match : correct hash
  // - Else : modified hash (fake)
}

bool validate_block(Block *block) {

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
}
