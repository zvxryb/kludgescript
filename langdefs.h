#ifndef KL_LANGDEFS_H
#define KL_LANGDEFS_H

/* IDs used for tokens, parsing, opcodes, etc. */

/* binary operators */
#define KL_FLAG_BINOP 0x0100
#define KL_BINOP(x) \
  ((x) | KL_FLAG_BINOP)
/* unary operators */
#define KL_FLAG_UNOP  0x0200
#define KL_UNOP(x) \
  ((x) | KL_FLAG_UNOP)
/* grouping operators */
#define KL_FLAG_GROUP 0x0400
#define KL_GROUP(x) \
  ((x) | KL_FLAG_GROUP)

#define KL_NONE     -1
#define KL_LABEL  0x01
#define KL_NUMBER 0x02
#define KL_PRINT  0x03 /* print */
#define KL_END    0x04 /* ; */

/* lexer->parser->compiler->opcodes */
#define KL_ADD    KL_BINOP(0x20) /* + */
#define KL_UADD   KL_UNOP(0x20)  /* + (unary) */
#define KL_SUB    KL_BINOP(0x21) /* - */
#define KL_USUB   KL_UNOP(0x21)  /* - (unary) */
#define KL_MUL    KL_BINOP(0x22) /* * */
#define KL_DIV    KL_BINOP(0x23) /* / */
#define KL_FDIV   KL_BINOP(0x24) /* // (python style "floor" division) */
#define KL_MOD    KL_BINOP(0x25) /* % */
#define KL_ASHFTL KL_BINOP(0x26) /* <<< (arithmetic shift left) */
#define KL_ASHFTR KL_BINOP(0x27) /* >>> (arithmetic shift right) */
#define KL_LSHFTL KL_BINOP(0x28) /* << (logical shift left) */
#define KL_LSHFTR KL_BINOP(0x29) /* >> (logical shift right) */
#define KL_BITAND KL_BINOP(0x2A) /* & */
#define KL_BITOR  KL_BINOP(0x2B) /* | */
#define KL_BITXOR KL_BINOP(0x2C) /* ^ */
#define KL_BITNOT 0x2D /* ~ */

/* lexer->parser */
#define KL_LPAREN KL_GROUP(0x40) /* ( */
#define KL_RPAREN KL_GROUP(0x41) /* ) */

/* lexer->parser->compiler->opcodes */
#define KL_CMP    KL_BINOP(0x50) /* <=> (ruby style general comparison operator) */
#define KL_EQ     KL_BINOP(0x51)
#define KL_LT     KL_BINOP(0x52)
#define KL_GT     KL_BINOP(0x53)
#define KL_LEQ    KL_BINOP(0x54)
#define KL_GEQ    KL_BINOP(0x55)

/* lexer->parser->compiler */
#define KL_ASSIGN 0x60

/* compiler->opcode */
#define KL_PUSH 0x80

#endif /* KL_LANGDEFS_H */