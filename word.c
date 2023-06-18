#include "word.h"
#include "code.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
    Creates a word by:
        - Allocating memory for word
        - Allocating memory for symbols (with length len)
        - Copying syms array into word->syms
        - Setting length
*/
Word *word_create(uint8_t *syms, uint32_t len) {
    Word *word = (Word *) calloc(1, sizeof(Word));

    if (word == NULL) {
        return NULL;
    }

    word->syms = (uint8_t *) calloc(len, sizeof(uint8_t));
    if (word->syms == NULL) {
        free(word);
        return NULL;
    }

    memcpy(word->syms, syms, len);

    word->len = len;
    return word;
}

/*
    Creates a new word that appends the symbol given by using word_create and setting the last character to the new symbol->
*/
Word *word_append_sym(Word *w, uint8_t sym) {
    if (w == NULL) {
        return NULL;
    }

    Word *word = (Word *) calloc(1, sizeof(Word));

    if (word == NULL) {
        return NULL;
    }

    word->syms = (uint8_t *) calloc(w->len + 1, sizeof(uint8_t));
    if (word->syms == NULL) {
        free(word);
        return NULL;
    }

    memcpy(word->syms, w->syms, w->len);

    word->syms[w->len] = sym;

    word->len = w->len + 1;

    return word;
}

/*
    Frees and NULLs w
*/
void word_delete(Word *w) {
    if (w == NULL) {
        return;
    }
    if (w->syms != NULL) {
        free(w->syms);
    }
    free(w);
}

/*
    Creates new WordTable with size MAX_CODE
*/
WordTable *wt_create(void) {
    WordTable *wt = (WordTable *) calloc(MAX_CODE, sizeof(Word *));
    uint8_t *syms = (uint8_t *) calloc(1, sizeof(uint8_t));
    Word *word = word_create(syms, 0);
    wt[EMPTY_CODE] = word;
    free(syms);
    return wt;
}

/*
    Resets wordtable by iteratively traversing word list and deleting all words in table (until null word)
*/
void wt_reset(WordTable *wt) {
    for (int i = 1; i < MAX_CODE; i++) {
        if (i == EMPTY_CODE || wt[i] == NULL) {
            continue;
        }
        word_delete(wt[i]);
        wt[i] = NULL;
    }
    return;
}

/*
    Frees all words in WordTable, then frees wordtable->
*/
void wt_delete(WordTable *wt) {
    wt_reset(wt);
    word_delete(wt[EMPTY_CODE]);
    free(wt);
    return;
}
