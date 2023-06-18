#include "trie.h"
#include "code.h"

#include <stdlib.h>

/*
    Creates a trie node with code: index.
    This allocates the memory required for the children pointer array.
*/
TrieNode *trie_node_create(uint16_t index) {
    TrieNode *node = (TrieNode *) calloc(1, sizeof(TrieNode));

    if (node == NULL) {
        return NULL;
    }

    node->code = index;
    return node;
}

/*
    Frees node n and sets pointer to NULL
*/
void trie_node_delete(TrieNode *n) {
    free(n);
}

/*
    Creates a new trie with EMPTY_CODE root.
*/
TrieNode *trie_create(void) {
    return trie_node_create(EMPTY_CODE);
}

/*
    Deletes trie from root
*/
void trie_reset(TrieNode *root) {
    for (int i = 0; i < ALPHABET; i++) {
        if (root->children[i] != NULL) {
            trie_delete(root->children[i]);
            root->children[i] = NULL;
        }
    }
}

/*
    Deletes each child in the subtree by using recursive call.
    Then will delete node n. 
*/
void trie_delete(TrieNode *n) {
    for (int i = 0; i < ALPHABET; i++) {
        if (n->children[i] != NULL) {
            trie_delete(n->children[i]);
            n->children[i] = NULL;
        }
    }
    trie_node_delete(n);
}

/*
    Steps down the tree to symbol
*/
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    return n->children[sym];
}
