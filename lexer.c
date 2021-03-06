#include "lexer.h"

#include <stdio.h>
#include <string.h>

#include "number.h"
#include "langdefs.h"


#define ISALPHA(c)\
  (((c) >= 'a' && (c) <= 'z') || ((c) >='A' && (c) <= 'Z'))

#define ISDECIMAL(c)\
  ((c) >= '0' && (c) <= '9')

#define ISALPHANUM(c)\
  (ISALPHA(c) || ISDECIMAL(c))

static void kl_lexer_alphanum(kl_lexer_t *source, char* buf, int *n, int max);
static void kl_lexer_number(kl_lexer_t *source, kl_token_num_t *token, char* buf, int n);
static int  kl_lexer_keyword(char *buf, int n);
static void kl_lexer_error(char *msg);
static int  peek(kl_lexer_t *source);
static void next(kl_lexer_t *source);

void kl_lexer_init(kl_lexer_t *source, kl_lexer_read_cb read, kl_lexer_err_cb error) {
  memset(source, 0, sizeof(kl_lexer_t));
  source->read  = read;
  source->error = error != NULL ? error : kl_lexer_error;
  source->cur   = read(); /* load first char */
  source->line  = 1;
  source->last  = KL_NONE;
}

#define KL_LEXER_BUFSIZE 0x0100
void kl_lexer_next(kl_lexer_t *s, kl_token_t *token) {
  kl_token_header_t* h = &token->header;
  h->type = KL_NONE;
  h->line = s->line;

  char c;
  char buf[KL_LEXER_BUFSIZE];
  while ((c = peek(s)) >= 0) {
    next(s);
    switch(c) {
      case '\x04': /* End of Transmission */
        h->type = KL_NONE;
        s->last = KL_NONE;
        return;
      case '#':    /* single-line comment */
        while (peek(s) != '\n') { next(s); }
        break;
      case '\n':
        s->line++;
        break;
      case ';':
        h->type = KL_END;
        s->last = KL_END;
        return;
      case '+':
        if (s->last == KL_NUMBER || s->last & KL_FLAG_VAR || s->last == KL_RPAREN) {
          h->type = KL_ADD;
          s->last = KL_ADD;
          return;
        }
        h->type = KL_UADD;
        s->last = KL_UADD;
        return;
      case '-':
        if (s->last == KL_NUMBER || s->last & KL_FLAG_VAR || s->last == KL_RPAREN) {
          h->type = KL_SUB;
          s->last = KL_SUB;
          return;
        }
        h->type = KL_USUB;
        s->last = KL_USUB;
        return;
      case '*':
        h->type = KL_MUL;
        s->last = KL_MUL;
        return;
      case '/':
        if (peek(s) == '/') {
          next(s);
          h->type = KL_FDIV;
          s->last = KL_FDIV;
          return;
        }
        h->type = KL_DIV;
        s->last = KL_DIV;
        return;
      case '%':
        h->type = KL_MOD;
        s->last = KL_MOD;
        return;
      case '<':
        if (peek(s) == '<') {
          next(s);
          if (peek(s) == '<') {
            next(s);
            h->type = KL_ASHFTL;
            s->last = KL_ASHFTL;
            return;
          }
          h->type = KL_LSHFTL;
          s->last = KL_LSHFTL;
          return;
        } else if (peek(s) == '=') {
          next(s);
          if (peek(s) == '>') {
            next(s);
            h->type = KL_CMP;
            s->last = KL_CMP;
            return;
          }
          h->type = KL_LEQ;
          s->last = KL_LEQ;
          return;
        }
        h->type = KL_LT;
        s->last = KL_LT;
        return;
      case '>':
        if (peek(s) == '>') {
          next(s);
          if (peek(s) == '>') {
            next(s);
            h->type = KL_ASHFTR;
            s->last = KL_ASHFTR;
            return;
          }
          h->type = KL_LSHFTR;
          s->last = KL_LSHFTR;
          return;
        } else if (peek(s) == '=') {
          next(s);
          h->type = KL_GEQ;
          s->last = KL_GEQ;
          return;
        }
        h->type = KL_GT;
        s->last = KL_GT;
        return;
      case '=':
        if (peek(s) == '=') {
          next(s);
          h->type = KL_EQ;
          s->last = KL_EQ;
          return;
        }
        h->type = KL_ASSIGN;
        s->last = KL_ASSIGN;
        return;
      case '&':
        if (peek(s) == '&') {
          next(s);
          h->type = KL_LOGAND;
          s->last = KL_LOGAND;
          return;
        }
        h->type = KL_BITAND;
        s->last = KL_BITAND;
        return;
      case '|':
        if (peek(s) == '|') {
          next(s);
          h->type = KL_LOGOR;
          s->last = KL_LOGOR;
          return;
        }
        h->type = KL_BITOR;
        s->last = KL_BITOR;
        return;
      case '^':
        h->type = KL_BITXOR;
        s->last = KL_BITXOR;
        return;
      case '~':
        h->type = KL_BITNOT;
        s->last = KL_BITNOT;
        return;
      case '!':
        if (peek(s) == '=') {
          next(s);
          h->type = KL_NEQ;
          s->last = KL_NEQ;
          return;
        }
        h->type = KL_LOGNOT;
        s->last = KL_LOGNOT;
        return;
      case '(':
        h->type = KL_LPAREN;
        s->last = KL_LPAREN;
        return;
      case ')':
        h->type = KL_RPAREN;
        s->last = KL_RPAREN;
        return;
      default:
        if (ISALPHA(c)) {
          buf[0] = c;
          int n  = 1;
          kl_lexer_alphanum(s, buf, &n, KL_LEXER_BUFSIZE);

          int kw = kl_lexer_keyword(buf, n);
          if (kw != KL_NONE) {
            h->type = kw;
            s->last = kw;
            return;
          }

          kl_token_str_t* stoken = &token->str;
          if (n > KL_TOKEN_STRLEN) {
            s->error("Variable name exceeds maximum length!");
            return;
          }
          stoken->header.type = KL_LOCAL;
          memcpy(stoken->str, buf, n);

          s->last            = KL_LOCAL;
          return;
        }

        if (ISDECIMAL(c)) {
          buf[0] = c;
          kl_lexer_number(s, (kl_token_num_t*)token, buf, 1);

          s->last = KL_NUMBER;
          return;
        }
    }
  }
}

/* continues reading a string of alphanumeric characters */
static void kl_lexer_alphanum(kl_lexer_t *source, char* buf, int *n, int max) {
  int c;
  for (int i=*n; i < max; i++) {
    c = peek(source);
    if (!(ISALPHANUM(c) || c == '_')) {
      *n = i;
      return;
    }
    buf[i] = c;
    next(source);
  }
  source->error("Label exceeds maximum length!");
}

/* continues reading and parses a numeric value -- partial number string (up to decimal) may be loaded into buf */
static void kl_lexer_number(kl_lexer_t *source, kl_token_num_t *token, char* buf, int n) {
  int c;
  kl_number_t number;

  for(int i=n; i < KL_LEXER_BUFSIZE; i++) {
    c = peek(source);
    if (!ISDECIMAL(c)) {
      number = kl_strtoinum(buf, i);
      break;
    }
    buf[i] = c;
    next(source);
  }
  if (c == '.') {
    next(source);
    for(int i=0; i < KL_LEXER_BUFSIZE; i++) {
      c = peek(source);
      if (!ISDECIMAL(c)) {
        number += kl_strtofnum(buf, i);
        break;
      }
      buf[i] = c;
      next(source);
    }
  }

  token->header.type = KL_NUMBER;
  token->val        = number;
}

static int kl_lexer_keyword(char *buf, int n) {
  if (strncmp(buf, "print", n) == 0) { return KL_PRINT; }
  if (strncmp(buf, "sin", n) == 0) { return KL_SINE; }
  if (strncmp(buf, "cos", n) == 0) { return KL_COSINE; }
  if (strncmp(buf, "ln", n) == 0) { return KL_LOG_E; }
  if (strncmp(buf, "lb", n) == 0) { return KL_LOG_2; }
  if (strncmp(buf, "lg", n) == 0) { return KL_LOG_10; }
  return KL_NONE;
}
static void kl_lexer_error(char *msg) {
  fprintf(stderr, "KludgeScript -> Lexical Analysis Error: %s\n", msg);
}
static int  peek(kl_lexer_t *source) {
  return source->cur;
}
static void next(kl_lexer_t *source) {
  source->cur = source->read();
}
