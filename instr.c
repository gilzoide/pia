#include "debug.h"
#include "instr.h"
#include "jit.h"

/// Create operations for `push`ing a value into the stack
static void add_push(pia_jit *jit, gcc_jit_location *loc, gcc_jit_block *block, gcc_jit_rvalue *rvalue) {
	gcc_jit_rvalue *v = gcc_jit_lvalue_as_rvalue(jit->v);
	gcc_jit_lvalue *s = gcc_jit_rvalue_dereference(gcc_jit_lvalue_as_rvalue(jit->s), loc);
	// v[s] = rvalue
	gcc_jit_block_add_assignment(
		block,
		loc,
		// v[s]
		gcc_jit_context_new_array_access(
			jit->ctx,
			loc,
			v,
			gcc_jit_lvalue_as_rvalue(s)),
		rvalue);

	gcc_jit_block_add_assignment_op(
		block,
		loc,
		s,
		GCC_JIT_BINARY_OP_PLUS,
		jit->int_one);
}

/// Create operation for `peek`ing the top value from the stack
static gcc_jit_rvalue *add_peek(pia_jit *jit, gcc_jit_location *loc) {
	gcc_jit_rvalue *v = gcc_jit_lvalue_as_rvalue(jit->v);
	gcc_jit_rvalue *s = gcc_jit_lvalue_as_rvalue(gcc_jit_rvalue_dereference(gcc_jit_lvalue_as_rvalue(jit->s), loc));
	return gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(
		jit->ctx,
		loc,
		v,
		s));
}

/// Create operations for `pop`ing the top value from the stack
static gcc_jit_rvalue *add_pop(pia_jit *jit, gcc_jit_location *loc, gcc_jit_block *block) {
	gcc_jit_rvalue *top = add_peek(jit, loc);
	gcc_jit_lvalue *s = gcc_jit_rvalue_dereference(gcc_jit_lvalue_as_rvalue(jit->s), loc);
	gcc_jit_block_add_assignment_op(
		block,
		loc,
		s,
		GCC_JIT_BINARY_OP_MINUS,
		jit->int_one);
	return top;
}

static void add_binary_op(pia_jit *jit, gcc_jit_location *loc, gcc_jit_block *block, enum gcc_jit_binary_op op) {
	gcc_jit_rvalue *b = add_pop(jit, loc, block);
	gcc_jit_rvalue *a = add_pop(jit, loc, block);
	add_push(
		jit,
		loc,
		block,
		gcc_jit_context_new_binary_op(
			jit->ctx,
			loc,
			op,
			jit->double_type,
			a,
			b));
}

int pia_instr_populate(pia_jit *jit, gcc_jit_function *func, gcc_jit_block *block, pia_instr *instr) {
	jit->v = gcc_jit_param_as_lvalue(gcc_jit_function_get_param(func, 0));
	jit->s = gcc_jit_param_as_lvalue(gcc_jit_function_get_param(func, 1));
	gcc_jit_location *loc = gcc_jit_context_new_location(jit->ctx, jit->filename, instr->line_on_file, 0);
	switch(instr->opcode) {
		case DUP:
			add_push(jit, loc, block, add_peek(jit, loc));
			break;
		case ROT: {
				gcc_jit_rvalue *b = add_pop(jit, loc, block);
				gcc_jit_rvalue *a = add_pop(jit, loc, block);
				add_push(jit, loc, block, a);
				add_push(jit, loc, block, b);
			}
			break;
		case ADD:
			add_binary_op(jit, loc, block, GCC_JIT_BINARY_OP_PLUS);
			break;
		case SUB:
			add_binary_op(jit, loc, block, GCC_JIT_BINARY_OP_MINUS);
			break;
		case MUL:
			add_binary_op(jit, loc, block, GCC_JIT_BINARY_OP_MULT);
			break;
		case DIV:
			add_binary_op(jit, loc, block, GCC_JIT_BINARY_OP_DIVIDE);
			break;
		case MOD:
			add_binary_op(jit, loc, block, GCC_JIT_BINARY_OP_MODULO);
			break;
		case PUSH: {
				gcc_jit_rvalue *pushed = gcc_jit_context_new_rvalue_from_double(
						jit->ctx, jit->double_type, instr->r1.d);
				add_push(jit, loc, block, pushed);
			}
			break;
		case CALL: {
				gcc_jit_function **ptr;
				JSLI(ptr, jit->func_map, (uint8_t *) instr->r1.s);
				if(*ptr == NULL) {
					FERROR("pia_instr_populate", "Function \"%s\" is undefined", instr->r1.s);
					return 0;
				}
				gcc_jit_rvalue *args[] = {
					gcc_jit_lvalue_as_rvalue(jit->v),
					gcc_jit_lvalue_as_rvalue(jit->s)
				};
				gcc_jit_block_add_eval(
					block,
					loc,
					gcc_jit_context_new_call(
						jit->ctx,
						loc,
						*ptr,
						2,
						args));
			}
			break;
		case PRINT: {
				const char *fmt;
				gcc_jit_rvalue *arg;
				// print string
				if(instr->r1.s) {
					fmt = "%s";
					arg = gcc_jit_context_new_string_literal(jit->ctx, instr->r1.s);
				}
				// print float from top of stack
				else {
					fmt = "%g";
					arg = add_pop(jit, loc, block);
				}
				gcc_jit_rvalue *args[] = {
					gcc_jit_context_new_string_literal(jit->ctx, fmt),
					arg,
				};
				gcc_jit_block_add_eval(
					block,
					loc,
					gcc_jit_context_new_call(
						jit->ctx,
						loc,
						jit->printf,
						2,
						args));
			}
			break;
	}
	return 1;
}
