# Simple Blockchain Implementation in C

A minimalist blockchain implementation written in C that demonstrates core blockchain concepts including proof-of-work mining, transaction handling, and chain validation.

## Features

- **Genesis Block Creation**: automatic initialization of the first block
- **Block Mining**: proof-of-work algorithm with configurable difficulty
- **Transaction Management**: add and store transactions within blocks
- **Chain Validation**: comprehensive blockchain integrity checking
- **Hash Verification**: SHA-256 hashing for block security
- **Interactive Display**: visual blockchain representation with loading animations
- **Memory Management**: proper allocation and cleanup of blockchain data

## Technical Specifications

- **Hashing Algorithm**: SHA-256
- **Mining Difficulty**: 3 leading zeros (configurable)
- **Max Transactions per Block**: 10
- **Block Structure**: index, timestamp, transactions, hash, previous hash, nonce
- **Transaction Structure**: sender, receiver, amount

## Prerequisites

- GCC compiler
- OpenSSL development libraries

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libssl-dev
```

**macOS:**
```bash
brew install openssl
```

**Fedora/RHEL:**
```bash
sudo dnf install openssl-devel
```

## Compilation

```bash
gcc -o blockchain main.c -lssl -lcrypto
```

## Usage

Run the compiled program:
```bash
./blockchain
```

The program will:
1. Create a genesis block
2. Add new blocks with sample transactions
3. Mine each block using proof-of-work
4. Display the complete blockchain with visual animations
5. Validate the entire chain integrity

## Code Structure

### Core Data Structures

```c
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
```

### Key Functions

- `hash_func()`: generates SHA-256 hash for blocks
- `mine_block()`: implements proof-of-work mining
- `add_transaction()`: adds transactions to blocks
- `validate_block_chain()`: verifies chain integrity
- `print_chain()`: visual blockchain display

## Example Output

```
 BLOCKCHAIN VISUALIZATION 
================================

    Loading block 0 | ✓ 
 -------- GENESIS BLOCK --------
Timestamp : 14:23:17
Prev Hash : 0...
Hash      : 0001a4f2b8c9d3e7...
------------------------------
              |
              ↓

    Loading block 1 / ✓ 
 -------- Block 1 --------
Timestamp : 14:23:45
Prev Hash : 0001a4f2b8c9d3e7...
Hash      : 0002f7a1c5d8e9b3...
Transactions :
Alice->Bob: 10000
Alice->Dave: 250
Alice->john: 100
------------------------------
```

## Validation Features

The blockchain checks:
- Hash integrity (recomputes and compares hashes)
- Block linking (previous hash matches)
- Sequential indexing (continuous block heights)
- Proof-of-work (mining difficulty compliance)

## Configuration

Modify these constants in the code:

```c
#define MINNING_DIFFUCULTY 3     // number of leading zeros required
#define MAX_TX 10                // maximum transactions per block
#define MAX_USERNAME_SIZE 50     // maximum username length
```

## Limitations & Future Improvements

**Current Limitations:**
- No network functionality
- No mempool implementation or transaction fees
- Fixed mining difficulty and in-memory storage only

**Potential Enhancements:**
- Mempool for pending transactions
- Dynamic difficulty adjustment
- Transaction fees and rewards
- Merkle tree for transaction verification
- Persistent file-based storage
- Network protocol implementation
- UTXO (Unspent Transaction Output) model
- Digital signatures for transaction security

## Learning Objectives

This project demonstrates:
- Linked list implementation for blockchain structure
- Cryptographic hashing (SHA-256)
- Proof-of-work consensus mechanism
- Memory management in C
- Data validation and integrity checking

## Contributing

Feel free to fork this project and submit pull requests for improvements. Some areas that could use enhancement:
- Better error handling
- More comprehensive testing
- Performance optimizations
- Additional validation checks

## License

This project is open source and available under the MIT License.

## Acknowledgments

- Built using OpenSSL for SHA-256 hashing
- Inspired by Bitcoin's blockchain architecture
- Educational implementation for learning blockchain concepts