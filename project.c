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

#define MINNING_DIFFUCULTY 1
#define MAX_SIZE 1024
#define MAX_HASH_SIZE 65
#define MAX_CONCATINATED_SIZE 2000

typedef struct Block {
  int index; // the height of the block
  char timestamps[20];
  char data[MAX_SIZE];
  char current_hash[MAX_HASH_SIZE];
  char prev_hash[MAX_HASH_SIZE];
  int nonce;
  struct Block *next_block;
} Block;

// Block mining
void mine_block(Block *block);

// Output :
void print_chain(Block *genesis_block);
void spinning_loading_minning();

// Extra
void spinning_loading();
void spinning_loading_util(char spin_char);

// Block structure
void free_chain(Block **genesis_block);
void test_invalid_chain(Block *genesis_block); // for debugging
bool validate_block_chain(Block **block);
void delete_invalid_block(Block **genesis_block);

void set_time_stamps(Block *myblock) {
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  strftime(myblock->timestamps, 20, "%H:%M:%S", timeinfo);
}

void hash_func(Block *myblock) {
  // Concatinate all the fields in a single string
  char con_data[MAX_CONCATINATED_SIZE];
  int char_written = snprintf(
      con_data, sizeof(con_data), "%d%s%s%s%d", myblock->index,
      myblock->timestamps, myblock->prev_hash, myblock->data, myblock->nonce);
  if (char_written >= MAX_CONCATINATED_SIZE) {
    perror("Buffer overflow");
    exit(1);
  }
  // Hashing
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char *)con_data, strlen(con_data), digest);

  // Convert the raw hash into hexa string
  char hex[65];
  for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
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
  new_block->index = index + 1;

  // now link the blocks and fill the "prev hash" field
  strcpy(new_block->prev_hash, temp->current_hash);
  mine_block(new_block);
  temp->next_block = new_block;
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
      printf("Enter the datj : ");
      char data[MAX_SIZE];
      fgets(data, MAX_SIZE, stdin);
      create_new_block(genesis_block, data);
      if (!validate_block_chain(genesis_block)) {
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

bool validate_block_chain(Block **genesis_block) {
  /* 1. Recompute the hash.
   *  2. Check : curr -> hash = prev -> hash
   *  3. Check : curr -> index = prev -> index + 1
   *  4. Verify the PoW.
   */

  if (*genesis_block == NULL) {
    return false;
  }

  Block *curr = *genesis_block;
  char hash_buffer[MAX_HASH_SIZE];

  while (curr->next_block) {
    strncpy(hash_buffer, curr->current_hash, MAX_HASH_SIZE);
    hash_func(curr);

    if (strcmp(hash_buffer, curr->current_hash) != 0) {
      fprintf(stderr, "Block Error : Block hash mismatch");
      return false;
    }

    if (curr->index + 1 != curr->next_block->index) {
      fprintf(stderr, "Block Error : Block height not continuous");
      return false;
    }

    if (strcmp(curr->current_hash, curr->next_block->prev_hash) != 0) {
      fprintf(stderr, "Block Error : Linking hash mismatch");
      return false;
    }

    for (int i = 0; i < MINNING_DIFFUCULTY; i++) {
      if (curr->current_hash[i] != '0') {
        fprintf(stderr,
                "Block Error : Not matching the correct PoW difficulty");
        return false;
      }
    }

    curr = curr->next_block;
  }

  // We still need to check for the latest block:
  for (int i = 0; i < MINNING_DIFFUCULTY; i++) {
    if (curr->current_hash[i] != '0') {
      fprintf(stderr, "Block Error : Not matching the correct PoW difficulty");
      return false;
    }
  }

  return true;
}

void delete_invalid_block(Block **genesis_block) {
  // 1. Loop through the linked list till the block before the end
  // 2. Set the next_block == NULL
  if (genesis_block == NULL || (*genesis_block)->next_block == NULL) {
    // nothing to delete here
    return;
  }

  // Case 2 : More than two blocks
  Block *temp = *genesis_block;
  while (temp->next_block->next_block != NULL) {
    temp = temp->next_block;
  }
  free(temp->next_block);
  temp->next_block = NULL;
}

void spinning_loading() {
  // const char *theme_1 = "⣾⣽⣻⢿⡿⣟⣯⣷";
  const char *theme_1 = "|/-\\|/-\\";
  for (int cycle = 0; cycle < 2; cycle++) { // 3 rotations
    for (unsigned short int i = 0; i < 4;
         i++) { // experimenting with different data types  :)
      spinning_loading_util(theme_1[i]);
    }
  }
}
void spinning_loading_util(char spin_char) {
  printf("\r%c ", spin_char);
  fflush(stdout);
  usleep(200000);
}
void test_invalid_chain(Block *genesis_block) {
  Block *chain = genesis_block;
  create_new_block(&chain, "A");
  create_new_block(&chain, "B");
  print_chain(genesis_block);

  // Test 1: Corrupt hash
  strcpy(chain->next_block->current_hash, "0000000000");
  assert(validate_block_chain(&chain) == false);

  // Test 2: Break link
  strcpy(chain->next_block->prev_hash, "broken_link");
  assert(validate_block_chain(&chain) == false);
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

void print_chain(Block *genesis_block) {
  if (!genesis_block) {
    printf("No blockchain to display!\n");
    return;
  }

  printf("\n BLOCKCHAIN VISUALIZATION \n");
  printf("================================\n\n");

  Block *curr = genesis_block;
  while (curr != NULL) {
    // Use your spinning loader
    printf("  Loading block %d ", curr->index);
    spinning_loading();
    printf(" ✓\n");

    // Print block info
    if (curr->index == 0) {
      printf(" -------- GENESIS BLOCK --------\n");
    } else {
      printf(" -------- Block %d --------\n", curr->index);
    }

    printf("Timestamp : %s\n", curr->timestamps);
    printf("Data      : %s\n", curr->data);
    printf("Prev Hash : %.16s...\n", curr->prev_hash);
    printf("Hash      : %.16s...\n", curr->current_hash);
    printf("------------------------------\n");

    // Connection arrow to next block (if not last)
    if (curr->next_block != NULL) {
      printf("              |\n");
      printf("              ↓\n");
      usleep(300000); // 0.3 seconds pause
    }

    printf("\n");
    curr = curr->next_block;
  }

  printf("End of blockchain!\n\n");
}

void mine_block(Block *block) {

  block->nonce = 0;
  printf("Mining Block : %d...\n", block->index);

  while (true) {

    hash_func(block);

    bool is_valid = true;
    for (int i = 0; i < MINNING_DIFFUCULTY; i++) {
      if (block->current_hash[i] != '0') {
        is_valid = false;
        break;
      }
    }
    spinning_loading_minning();

    if (block->nonce % 10000 == 0) {
      printf("Tried nonce %d -> hash : %.15s", block->nonce,
             block->current_hash);
    }

    if (is_valid) {
      printf("Block mined !: None = %d", block->nonce);
      printf("Final hash : %s", block->current_hash);
      break;
    }

    block->nonce++;
  }
}
void spinning_loading_minning() {
  // const char *theme_1 = "⣾⣽⣻⢿⡿⣟⣯⣷";
  const char *theme_1 = "|/-\\|/-\\";
  for (int cycle = 0; cycle < 2; cycle++) { // 3 rotations
    for (unsigned short int i = 0; i < 4;
         i++) { // experimenting with different data types  :)
      spinning_loading_util(theme_1[i]);
    }
  }
}
