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

#define MINNING_DIFFUCULTY 3
#define MAX_SIZE 1024
#define MAX_HASH_SIZE 65
#define MAX_CONCATINATED_SIZE 10000
#define MAX_USERNAME_SIZE 50
#define MAX_TX 10

typedef struct Transaction {
  char sender[MAX_USERNAME_SIZE];
  char receiver[MAX_USERNAME_SIZE];
  int amount;
} Transaction;

typedef struct Block {
  int index;
  char timestamps[20];
  Transaction transactions[MAX_TX];
  int tx_count;
  char current_hash[MAX_HASH_SIZE];
  char prev_hash[MAX_HASH_SIZE];
  int nonce;
  struct Block *next_block;
} Block;

void add_transaction(Block *block, char *from, char *to, int amount);
void mine_block(Block *block);
void mine_latest_block(Block *block);
void print_chain(Block *genesis_block);
void spinning_loading_minning();
void spinning_loading();
void spinning_loading_util(char spin_char);
void free_chain(Block **genesis_block);
void test_invalid_chain(Block *genesis_block);
bool validate_block_chain(Block **block);
void delete_invalid_block(Block **genesis_block);

void set_time_stamps(Block *myblock) {
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  strftime(myblock->timestamps, 20, "%H:%M:%S", timeinfo);
}

void hash_func(Block *myblock) {
  if (!myblock) {
    return;
  }

  char con_data[MAX_CONCATINATED_SIZE];
  int char_written =
      snprintf(con_data, sizeof(con_data), "%d%s%s%d", myblock->index,
               myblock->timestamps, myblock->prev_hash, myblock->nonce);

  if (char_written >= MAX_CONCATINATED_SIZE) {
    perror("Buffer overflow");
    exit(1);
  }

  for (int i = 0; i < myblock->tx_count; i++) {
    int remaining = MAX_CONCATINATED_SIZE - char_written;
    int tx_written = snprintf(con_data + char_written, remaining, "%s%s%d",
                              myblock->transactions[i].sender,
                              myblock->transactions[i].receiver,
                              myblock->transactions[i].amount);

    if (tx_written >= remaining) {
      printf("Transaction Error : Buffer overflow");
      exit(1);
    }
    char_written += tx_written;
  }

  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char *)con_data, strlen(con_data), digest);

  char hex[65];
  for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    sprintf(hex + (i * 2), "%02x", digest[i]);
  }
  hex[64] = '\0';

  strcpy(myblock->current_hash, hex);
}

void init_new_block(Block *genesis_block) {
  set_time_stamps(genesis_block);
  genesis_block->index = 0;
  genesis_block->tx_count = 0;
  genesis_block->next_block = NULL;
  strcpy(genesis_block->prev_hash, "0");
  strcpy(genesis_block->current_hash, "0");
  memset(genesis_block->transactions, 0, sizeof(genesis_block->transactions));
}

void create_new_block(Block **genesis_block) {
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

  Block *new_block = (Block *)malloc(sizeof(Block));
  if (!new_block) {
    perror("Error Allocating data for the new block");
    exit(EXIT_FAILURE);
  }
  init_new_block(new_block);
  new_block->index = index + 1;

  strcpy(new_block->prev_hash, temp->current_hash);
  temp->next_block = new_block;
}

void create_new_block_wrapper(Block **genesis_block) {
  while (1) {
    system("clear");
    char answer;
    printf("Do you want to Create a new block  ? (Y/n)");
    scanf(" %c", &answer);
    if (tolower(answer) == 'n') {
      printf("Have a good day");
      return;
    } else if (tolower(answer) == 'y') {
      printf("Enter the datj : ");
      char data[MAX_SIZE];
      fgets(data, MAX_SIZE, stdin);
      create_new_block(genesis_block);
      if (!validate_block_chain(genesis_block)) {
        printf("New block invalid !");
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
  Block *genesis_block;
  genesis_block = malloc(sizeof(Block));
  if (!genesis_block) {
    perror("Error allocating memory");
    return 1;
  }

  init_new_block(genesis_block);
  hash_func(genesis_block);

  test_invalid_chain(genesis_block);
  free_chain(&genesis_block);

  return 0;
}

bool validate_block_chain(Block **genesis_block) {
  if (*genesis_block == NULL) {
    return false;
  }

  Block *curr = *genesis_block;
  char hash_buffer[MAX_HASH_SIZE];

  while (curr->next_block) {
    strncpy(hash_buffer, curr->current_hash, MAX_HASH_SIZE);
    hash_func(curr);

    if (strcmp(hash_buffer, curr->current_hash) != 0) {
      fprintf(stderr, "Block Error : Block hash mismatch\n");
      return false;
    }

    if (curr->index + 1 != curr->next_block->index) {
      fprintf(stderr, "Block Error : Block height not continuous\n");
      return false;
    }

    if (strcmp(curr->current_hash, curr->next_block->prev_hash) != 0) {
      fprintf(stderr, "Block Error : Linking hash mismatch\n");
      return false;
    }

    for (int i = 0; i < MINNING_DIFFUCULTY; i++) {
      if (curr->current_hash[i] != '0') {
        fprintf(stderr,
                "Block Error : Not matching the correct PoW difficulty\n");
        return false;
      }
    }

    curr = curr->next_block;
  }

  for (int i = 0; i < MINNING_DIFFUCULTY; i++) {
    if (curr->current_hash[i] != '0') {
      fprintf(stderr, "Block Error : Not matching the correct PoW difficulty");
      return false;
    }
  }

  return true;
}

void delete_invalid_block(Block **genesis_block) {
  if (genesis_block == NULL || (*genesis_block)->next_block == NULL) {
    return;
  }

  Block *temp = *genesis_block;
  while (temp->next_block->next_block != NULL) {
    temp = temp->next_block;
  }
  free(temp->next_block);
  temp->next_block = NULL;
}

void spinning_loading() {
  const char *theme_1 = "|/-\\|/-\\";
  for (int cycle = 0; cycle < 2; cycle++) {
    for (unsigned short int i = 0; i < 4; i++) {
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

  create_new_block(&chain);

  Block *latest_block = chain;
  while (latest_block->next_block != NULL) {
    latest_block = latest_block->next_block;
  }

  add_transaction(latest_block, "Alice", "Bob", 10000);
  add_transaction(latest_block, "Alice", "Dave", 250);
  add_transaction(latest_block, "Alice", "john", 100);

  mine_latest_block(chain);

  create_new_block(&chain);
  while (latest_block->next_block != NULL) {
    latest_block = latest_block->next_block;
  }
  add_transaction(latest_block, "Rida", "Mortada", 9999);
  add_transaction(latest_block, "Rida", "Didine", 125);
  add_transaction(latest_block, "Rida", "Connor", 50);

  mine_latest_block(chain);

  create_new_block(&chain);

  while (latest_block->next_block != NULL) {
    latest_block = latest_block->next_block;
  }

  add_transaction(latest_block, "jamel", "Mortada", 9999);
  add_transaction(latest_block, "jamel", "Didine", 125);
  add_transaction(latest_block, "jamel", "Connor", 50);

  print_chain(genesis_block);

  strcpy(chain->next_block->current_hash, "0000000000");
  assert(validate_block_chain(&chain) == false);

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
  *genesis_block = NULL;
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
    printf("    Loading block %d ", curr->index);
    spinning_loading();
    printf(" ✓ \n");

    if (curr->index == 0) {
      printf(" -------- GENESIS BLOCK --------\n");
    } else {
      printf(" -------- Block %d --------\n", curr->index);
    }

    printf("Timestamp : %s\n", curr->timestamps);
    printf("Prev Hash : %.16s...\n", curr->prev_hash);
    printf("Hash      : %.16s...\n", curr->current_hash);

    if (curr->index > 0) {
      printf("Transactions :\n");
      for (int i = 0; i < curr->tx_count; i++) {
        printf("%s->%s: %d\n", curr->transactions[i].sender,
               curr->transactions[i].receiver, curr->transactions[i].amount);
      }
    }
    printf("------------------------------\n");

    if (curr->next_block != NULL) {
      printf("              |\n");
      printf("              ↓\n");
      usleep(300000);
    }

    printf("\n");
    fflush(stdout);
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
      printf("\nBlock mined !: None = %d\n", block->nonce);
      printf("Final hash : %s\n", block->current_hash);
      break;
    }

    block->nonce++;
  }
}

void spinning_loading_minning() {
  static int i = 0;
  const char *theme = "|/-\\";

  printf("\r%c Mining... ", theme[i]);
  fflush(stdout);

  // keep changing the spinner character in a circle
  i = (i + 1) % 4;
}

void add_transaction(Block *block, char *from, char *to, int amount) {
  if (!block || block->tx_count >= MAX_TX) {
    fprintf(stderr,
            "Transaction error : maximum number of transactions reached\n");
    return;
  }
  Transaction *tx = &block->transactions[block->tx_count];

  strncpy(tx->receiver, to, MAX_USERNAME_SIZE - 1);
  tx->receiver[MAX_USERNAME_SIZE - 1] = '\0';

  strncpy(tx->sender, from, MAX_USERNAME_SIZE - 1);
  tx->sender[MAX_USERNAME_SIZE - 1] = '\0';

  tx->amount = amount;
  block->tx_count++;
}

void mine_latest_block(Block *block) {
  if (!block) {
    printf("Block Error : block doesn't exist or is corrupt");
    return;
  }

  Block *temp = block;
  while (temp->next_block != NULL) {
    temp = temp->next_block;
  }
  mine_block(temp);
}
