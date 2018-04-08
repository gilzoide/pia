#include "debug.h"
#include "instr.h"
#include "jit.h"

/// Create operations for `push`ing a value into the stack
static void add_push(pia_jit *jit, jit_function_t func, jit_value_t value) {
	// (*s)++
	jit_value_t s = jit_insn_load_relative(func, jit->s, 0, jit_type_int);
	jit_value_t one = jit_value_create_nint_constant(func, jit_type_int, 1);
	s = jit_insn_add(func, s, one);
	jit_insn_store_relative(func, jit->s, 0, s);
	// v[s] = value
	jit_insn_store_elem(func, jit->v, s, value);
}

/// Create operation for `peek`ing the top value from the stack
static jit_value_t add_peek(pia_jit *jit, jit_function_t func) {
	jit_value_t s = jit_insn_load_relative(func, jit->s, 0, jit_type_int);
	return jit_insn_load_elem(func, jit->v, s, jit_type_float64);
}

/// Create operations for `pop`ing the top value from the stack
static jit_value_t add_pop(pia_jit *jit, jit_function_t func) {
	// top = v[s]
	jit_value_t s = jit_insn_load_relative(func, jit->s, 0, jit_type_int);
	jit_value_t top = jit_insn_load_elem(func, jit->v, s, jit_type_float64);
	// (*s)--
	jit_value_t one = jit_value_create_nint_constant(func, jit_type_int, 1);
	s = jit_insn_sub(func, s, one);
	jit_insn_store_relative(func, jit->s, 0, s);
	
	return top;
}

typedef jit_value_t(*_binary_op_function)(jit_function_t, jit_value_t, jit_value_t);

static void add_binary_op(pia_jit *jit, jit_function_t func, pia_opcode op) {
	jit_value_t b = add_pop(jit, func);
	jit_value_t a = add_pop(jit, func);
	_binary_op_function apply_binary;
	switch(op) {
		case ADD: apply_binary = jit_insn_add; break;
		case SUB: apply_binary = jit_insn_sub; break;
		case MUL: apply_binary = jit_insn_mul; break;
		case DIV: apply_binary = jit_insn_div; break;
		case MOD: apply_binary = jit_insn_rem; break;
		default: return;
	}
	add_push(jit, func, apply_binary(func, a, b));
}

int pia_instr_populate(pia_jit *jit, jit_function_t func, pia_instr *instr) {
	jit->v = jit_value_get_param(func, 0);
	jit->s = jit_value_get_param(func, 1);
	switch(instr->opcode) {
		case NOP:
			jit_insn_nop(func);
			break;
		case DUP:
			add_push(jit, func, add_peek(jit, func));
			break;
		case ROT: {
				jit_value_t a = add_pop(jit, func);
				jit_value_t b = add_pop(jit, func);
				add_push(jit, func, a);
				add_push(jit, func, b);
			}
			break;
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOD:
			add_binary_op(jit, func, instr->opcode);
			break;
		case PUSH: {
				jit_value_t pushed = jit_value_create_float64_constant(func, jit_type_float64, instr->r1.d);
				add_push(jit, func, pushed);
			}
			break;
		case POP:
			add_pop(jit, func);
			break;
		case CALL: {
				const char *name = instr->r1.s;
				jit_function_t f = pia_find_function(jit, name);
				if(f == NULL) {
					FERROR("pia_instr_populate", "Function \"%s\" is undefined", name);
					return 0;
				}
				jit_insn_call(func, name, f, jit->func_signature, (jit_value_t[]){ jit->v, jit->s }, 2, 0);
			}
			break;
		case PRINT: {
				const char *fmt;
				jit_value_t arg;
				jit_type_t sig;
				// print string
				if(instr->r1.s) {
					sig = jit->printf_s_signature;
					fmt = "%s";
					arg = jit_value_create_nint_constant(func, jit->_charp, (intptr_t)instr->r1.s);
				}
				// print float from top of stack
				else {
					sig = jit->printf_d_signature;
					fmt = "%g";
					arg = add_peek(jit, func);
				}
				jit_value_t params[] = {
					jit_value_create_nint_constant(func, jit->_charp, (intptr_t)fmt),
					arg,
				};
				jit_insn_call_native(func, "printf", printf, sig, params, 2, 0);
			}
			break;
	}
	return 1;
}
