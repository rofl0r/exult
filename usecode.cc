/**
 **	Usecode.cc - Interpreter for usecode.
 **
 **	Written: 8/12/99 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>			/* Debugging.			*/
#include <fstream.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "usecode.h"
#include "gamewin.h"
#include "objs.h"

/*
 *	Get array size.
 */

int Usecode_value::count_array
	(
	const Usecode_value& val
	)
	{
	int i;
	for (i = 0; val.value.array[i].type != (int) end_of_array_type; i++)
		;
	return (i);
	}

/*
 *	Resize array.  (Assumes this is an array.)
 */

void Usecode_value::resize
	(
	int new_size
	)
	{
	int size = count_array(*this);	// Get current size.
	Usecode_value *newvals = new Usecode_value[new_size + 1];
	newvals[new_size].type = (unsigned char) end_of_array_type;
					// Move old values over.
	int cnt = new_size < size ? new_size : size;
	for (int i = 0; i < cnt; i++)
		newvals[i] = value.array[i];
	delete [] value.array;		// Delete old list.
	value.array = newvals;		// Store new.
	}

/*
 *	Comparator.
 *
 *	Output:	1 if they match, else 0.
 */

int Usecode_value::operator==
	(
	const Usecode_value& v2
	)
	{
	if (&v2 == this)
		return (1);		// Same object.
	if (v2.type != type)
		return (0);		// Wrong type.
	if (type == (int) int_type)
		return (value.intval == v2.value.intval);
	else if (type == (int) string_type)
		return (strcmp(value.str, v2.value.str) == 0);
	else
		return (0);
	}

/*
 *	Search an array for a given value.
 *
 *	Output:	Index if found, else -1.
 */

int Usecode_value::find_elem
	(
	const Usecode_value& val
	)
	{
	if (type != array_type)
		return (-1);		// Not an array.
	int i;
	for (i = 0; val.value.array[i].type != (int) end_of_array_type; i++)
		if (value.array[i] == val)
			return (i);
	return (-1);
	}

/*
 *	Concatenate two values.
 */

Usecode_value& Usecode_value::concat
	(
	Usecode_value& val2		// Concat. val2 onto end.
	)
	{
	Usecode_value *result;
	int size;			// Size of result.
	if (type != array_type)		// Not an array?  Create one.
		{
		result = new Usecode_value(1, this);
		size = 1;
		}
	else
		{
		result = this;
		size = get_array_size();
		}
	if (val2.type != array_type)	// Appending a single value?
		{
		result->resize(size + 1);
		result->put_elem(size, val2);
		}
	else				// Appending an array.
		{
		int size2 = val2.get_array_size();
		result->resize(size + size2);
		for (int i = 0; i < size2; i++)
			result->put_elem(size + i, val2.get_elem(i));
		}
	return (*result);
	}

/*
 *	Print in ASCII.
 */

void Usecode_value::print
	(
	ostream& out
	)
	{
	switch ((Val_type) type)
		{
	case int_type:
		out << value.intval; break;
	case string_type:
		out << '"' << value.str << '"'; break;
	case array_type:
		{
		for (int i = 0; value.array[i].type != (int) end_of_array_type;
									i++)
			{
			if (i)
				out << ", ";
			value.array[i].print(out);
			}
		}
		break;
	default:
		break;
		}
	}

/*
 *	Read in a function.
 */

Usecode_function::Usecode_function
	(
	istream& file
	)
	{
	id = Read2(file);
	len = Read2(file);
	code = new unsigned char[len];	// Allocate buffer & read it in.
	file.read(code, len);
	}

/*
 *	Append a string.
 */

void Usecode_machine::append_string
	(
	char *str
	)
	{
	if (!str)
		return;
					// Figure new length.
	int len = string ? strlen(string) : 0;
	len += strlen(str);
	char *newstr = new char[len + 1];
	if (string)
		{
		strcpy(newstr, string);
		delete string;
		string = strcat(newstr, str);
		}
	string = strcpy(newstr, str);
	}

/*
 *	Say the current string and empty it.
 */

void Usecode_machine::say_string
	(
	)
	{
	if (string)
		cout << "\nSaying:  " << string << '\n';//+++++++Testing.
	delete string;
	string = 0;
	user_choice = 0;		// Clear user's response.
	}

/*
 *	Stack error.
 */

void Usecode_machine::stack_error
	(
	int under			// 1 if underflow.
	)
	{
	if (under)
		cerr << "Stack underflow.\n";
	else
		cerr << "Stack overflow.\n";
	exit(1);
	}

/*
 *	Add a possible 'answer'.
 */

void Usecode_machine::add_answer
	(
	char *str
	)
	{
	if (num_answers < max_answers)
		answers[num_answers++] = str;
	}

/*
 *	Add an answer to the list.
 */

void Usecode_machine::add_answer
	(
	Usecode_value& val		// Array or string.
	)
	{
	char *str;
	int size = val.get_array_size();
	if (size)			// An array?
		{
		for (int i = 0; i < size; i++)
			if ((str = val.get_elem(i).get_str_value()) != 0)
				add_answer(str);
		}
	else if ((str = val.get_str_value()) != 0)
		add_answer(str);
	}

/*
 *	Remove an answer from the list.
 */

void Usecode_machine::remove_answer
	(
	Usecode_value& val		// String.
	)
	{
	char *str = val.get_str_value();
	if (!str)
		return;
	int i;				// Look for answer.
	for (i = 0; i < num_answers; i++)
		if (strcmp(answers[i], str) == 0)
			break;		// Found it.
	if (i == num_answers)
		return;			// Not found.
	for (i++; i < num_answers; i++)	// Shift others left.
		answers[i - 1] = answers[i];
	num_answers--;
	}

/*
 *	Show an NPC's face.
 */

void Usecode_machine::show_npc_face
	(
	Usecode_value& arg1,		// Shape (NPC #).
	Usecode_value& arg2		// Frame.
	)
	{
	int shape = -arg1.get_int_value();
	int frame = arg2.get_int_value();
	gwin->show_face(shape, frame);
	cout << "Show face " << shape << '\n';
	}

/*
 *	Remove an NPC's face.
 */

void Usecode_machine::remove_npc_face
	(
	Usecode_value& arg1		// Shape (NPC #).
	)
	{
	int shape = -arg1.get_int_value();
	//++++++++
	cout << "Remove face " << shape << '\n';
	}

/*
 *	Set an item's shape.
 */

void Usecode_machine::set_item_shape
	(
	Usecode_value& item_arg,
	Usecode_value& shape_arg
	)
	{
	Game_object *item = (Game_object *) item_arg.get_int_value();
	int shape = shape_arg.get_int_value();
	if (item != 0)
		item->set_shape(shape);
	cout << "Set_item_shape: " << item << ", " << shape << '\n';
	}

/*
 *	Set an item's frame.
 */

void Usecode_machine::set_item_frame
	(
	Usecode_value& item_arg,
	Usecode_value& frame_arg
	)
	{
	Game_object *item = (Game_object *) item_arg.get_int_value();
	int frame = frame_arg.get_int_value();
	if (item != 0)
		item->set_frame(frame);
	cout << "Set_item_frame: " << item << ", " << frame << '\n';
	}

/*
 *	Get an item's shape.
 */

int Usecode_machine::get_item_shape
	(
	Usecode_value& item_arg
	)
	{
	Game_object *item = (Game_object *) item_arg.get_int_value();
	return (item == 0 ? 0 : item->get_shapenum());
	}

/*
 *	Get an item's frame.
 */

int Usecode_machine::get_item_frame
	(
	Usecode_value& item_arg
	)
	{
	Game_object *item = (Game_object *) item_arg.get_int_value();
	return (item == 0 ? 0 : item->get_framenum());
	}

/*
 *	Call an intrinsic function.
 */

Usecode_value Usecode_machine::call_intrinsic
	(
	int intrinsic,			// The ID.
	int num_parms			// # parms on stack.
	)
	{
	Usecode_value parms[12];	// Get parms.
	for (int i = 0; i < num_parms; i++)
		{
		Usecode_value val = pop();
		parms[i] = val;
		}
	switch (intrinsic)
		{
	case 0:				// Return random #.
		{
		int range = parms[0].get_int_value();
		if (range == 0)
			return Usecode_value(0);
		return Usecode_value(1 + (rand() % range));
		}
	case 3:				// Show NPC face.
		show_npc_face(parms[0], parms[1]);
		break;
	case 4:				// Remove NPC face.
		remove_npc_face(parms[0]);
		break;
	case 5:				// Add possible 'answer'.
		add_answer(parms[0]);
		break;
	case 6:				// Remove answer.
		remove_answer(parms[0]);
		break;
	case 13:			// Set item shape.
		set_item_shape(parms[0], parms[1]);
		break;
	case 17:			// Get item shape.
		return (Usecode_value(get_item_shape(parms[0])));
	case 18:
		return (Usecode_value(get_item_frame(parms[0])));
	case 19:			// Set item shape.
		set_item_frame(parms[0], parms[1]);
		break;
	case 0x27:			// Get player name.
		return Usecode_value("Player");
	default:
cout << "Unhandled intrinsic " << intrinsic << " called with " << num_parms
	<< " parms\n";
		break;
		}
	Usecode_value no_ret;				// Dummy return.
	return (no_ret);
	}

/*
 *	Get user's choice from among the possible responses.
 */

void Usecode_machine::get_user_choice
	(
	)
	{
	if (!num_answers)
		return;			// Shouldn't happen.
	while (!user_choice)		// May have already been done.
		{
		cout << "Choose: ";	// TESTING.
		for (int i = 0; i < num_answers; i++)
			cout << ' ' << answers[i] << '(' << i << ") ";
		int response;
		cin >> response;
		if (response >= 0 && response < num_answers)
			user_choice = answers[response];
		}
	}

/*
 *	Create machine from a 'usecode' file.
 */

Usecode_machine::Usecode_machine
	(
	istream& file,
	Game_window *gw
	) : num_funs(0), string(0), gwin(gw), caller_item(0),
	    stack(new Usecode_value[1024]), user_choice(0), num_answers(0)
	{
	sp = stack;
					// Clear global flags.
	memset(gflags, 0, sizeof(gflags));
	gflags[0x1b3] = 1;		// Testing Ferryman.
	file.seekg(0, ios::end);
	int size = file.tellg();	// Get file size.
	file.seekg(0);
					// Read in all the functions.
	while (num_funs < max_funs && file.tellg() < size)
		funs[num_funs++] = new Usecode_function(file);
	}

/*
 *	Delete.
 */

Usecode_machine::~Usecode_machine
	(
	)
	{
	delete [] stack;
	delete string;
	for (int i = 0; i < num_funs; i++)
		delete funs[i];
	}

int debug = 1;				// 2 for more stuff.

/*
 *	Interpret a single usecode function.
 */

void Usecode_machine::run
	(
	Usecode_function *fun,
	int event			// Event (??) that caused this call.
	)
	{
	printf("Running usecode %04x\n", fun->id);
	Usecode_value *save_sp = sp;	// Save TOS.
					// Save answers list.
	int save_num_answers = num_answers;
	char *save_answers[max_answers];
	for (int i = 0; i < num_answers; i++)
		save_answers[i] = answers[i];
	num_answers = 0;
	unsigned char *ip = fun->code;	// Instruction pointer.
					// Where it ends.
	unsigned char *endp = ip + fun->len;
	int data_len = Read2(ip);	// Get length of (text) data.
	char *data = (char *) ip;	// Save ->text.
	ip += data_len;			// Point past text.
	int num_args = Read2(ip);	// # of args. this function takes.
					// Local variables follow args.
	int num_locals = Read2(ip) + num_args;
					// Allocate locals.
	Usecode_value *locals = new Usecode_value[num_locals];
					// Store args.
	for (int i = 0; i < num_args; i++)
		{
		Usecode_value val = pop();
		locals[num_args - i - 1] = val;
		}
	int num_externs = Read2(ip);	// # of external refs. following.
	unsigned char *externals = ip;	// Save -> to them.
	ip += 2*num_externs;		// ->actual bytecode.
	int offset;			// Gets offset parm.
	int sval;			// Get value from top-of-stack.
	unsigned char *code = ip;	// Save for debugging.
	int set_ret_value = 0;		// 1 when return value is set.
	Usecode_value ret_value;	// Gets return value.
	/*
	 *	Main loop.
	 */
	while (ip < endp)
		{
		int opcode = *ip++;
		if (debug >= 2)
			printf("SP = %d, IP = %04x, op = %02x\n", sp - stack,
						ip - code, opcode);
		switch (opcode)
			{
		case 0x04:		// Jump if done with function.
			offset = (short) Read2(ip);
			if (set_ret_value || !num_answers)
				ip += offset;
			break;
		case 0x05:		// JNE.
			offset = (short) Read2(ip);
			if (!popi())
				ip += offset;
			break;
		case 0x06:		// JMP.
			offset = (short) Read2(ip);
			ip += offset;
			break;
		case 0x07:		// Guessing CMPS.
			{
			get_user_choice();
			int cnt = Read2(ip);	// # strings.
			offset = (short) Read2(ip);
			while (cnt-- && strcmp(pops(), user_choice) != 0)
				;
			if (cnt == -1)	// Jump if no match.
				ip += offset;
			}
			break;
		case 0x09:		// ADD.
			pushi(popi() + popi());
			break;
		case 0x0a:		// SUB.
			sval = popi();
			pushi(sval + popi());
			break;
		case 0x0b:		// DIV.
			sval = popi();
			pushi(popi()/sval);
			break;
		case 0x0c:		// MUL.
			pushi(popi()*popi());
			break;
		case 0x0d:		// MOD.
			sval = popi();
			pushi(popi() % sval);
			break;
		case 0x0e:		// AND.
			pushi(popi() & popi());
			break;
		case 0x0f:		// OR.
			pushi(popi() | popi());
			break;
		case 0x10:		// NOT.
			pushi(!popi());
			break;
		case 0x12:		// POP into a variable.
			{
			offset = Read2(ip);
					// Get value.
			Usecode_value val = pop();
			if (offset < 0 || offset >= num_locals)
				cerr << "Local #" << offset << 
							"out of range\n";
			else
				locals[offset] = val;
			}
			break;
		case 0x13:		// PUSH true.
			pushi(1);
			break;
		case 0x14:		// PUSH false.
			pushi(0);
			break;
		case 0x16:		// CMPGT.
			sval = popi();
			pushi(popi() > sval);	// Order?
			break;
		case 0x17:		// CMPL.
			sval = popi();
			pushi(popi() < sval);
			break;
		case 0x18:		// CMPGE.
			sval = popi();
			pushi(popi() >= sval);
			break;
		case 0x19:		// CMPLE.
			sval = popi();
			pushi(popi() <= sval);
			break;
		case 0x1a:		// CMPNE.
			sval = popi();
			pushi(popi() != sval);
			break;
		case 0x1c:		// ADDSI.
			offset = Read2(ip);
			append_string(data + offset);
			break;
		case 0x1d:		// PUSHS.
			offset = Read2(ip);
			pushs(data + offset);
			break;
		case 0x1e:		// ARRC.
			{		// Get array size.
			offset = Read2(ip);
			Usecode_value arr(offset, 0);
			for (int i = 0; i < offset; i++)
				{
				Usecode_value val = pop();
				arr.put_elem(i, val);
				}
			push(arr);
			}
			break;
		case 0x1f:		// PUSHI.
			offset = Read2(ip);
			pushi(offset);
			break;
		case 0x21:		// PUSH.
			offset = Read2(ip);
			push(locals[offset]);
			break;
		case 0x22:		// CMPEQ.
			sval = popi();
			pushi(sval == popi());
			break;
		case 0x24:		// CALL.
			offset = Read2(ip);
			call_usecode_function(externals[2*offset] + 
					256*externals[2*offset + 1]);
			break;
		case 0x25:		// RET.
			sp = save_sp;		// Restore stack.
			ip = endp;	// End the loop.
			break;
		case 0x26:		// AIDX.
			{
			sval = popi();	// Get index into array.
					// Get # of local to index.
			offset = Read2(ip);
			if (offset < 0 || offset >= num_locals)
				{
				cerr << "Local #" << offset << 
							"out of range\n";
				pushi(0);
				break;
				}
			Usecode_value& val = locals[offset];
			int sz = val.get_array_size();
			if (sz <= 0 || sval < 0 || sval >= sz)
				{
				cerr << 
				"AIDX index out of range, or not an array\n";
				pushi(0);
				break;
				}
			push(val.get_elem(sval));
			break;
			}
		case 0x2c:		// Unknown.
			break;
		case 0x2d:		// Set return value.
			ret_value = pop();
			set_ret_value = 1;
			break;
		case 0x2e:		// Looks like a loop.
			if (*ip++ != 2)
				cout << "2nd byte in loop isn't a 2!\n";
					// FALL THROUGH.
		case 0x02:		// 2nd byte of loop.
			{
					// Counter (1-based).
			int local1 = Read2(ip);
					// Total count.
			int local2 = Read2(ip);
					// Current value of loop var.
			int local3 = Read2(ip);
					// Array of values to loop over.
			int local4 = Read2(ip);
					// Get offset to end of loop.
			offset = (short) Read2(ip);
					// Get array to loop over.
			Usecode_value& arr = locals[local4];
			if (opcode == 0x2e)
				{	// Initialize loop.
				int cnt = arr.get_array_size();
				locals[local2] = Usecode_value(cnt);
				locals[local1] = Usecode_value(0);
				}
			int next = locals[local1].get_int_value();
					// End of loop?
			if (next >= locals[local2].get_int_value())
				ip += offset;
			else		// Get next element.
				{
				locals[local3] = arr.get_elem(next);
				locals[local1] = Usecode_value(next + 1);
				}
			break;
			}
		case 0x2f:		// ADDSV.
			offset = Read2(ip);
			append_string(locals[offset].get_str_value());
			break;
		case 0x30:		// IN.  Is a val. in an array?
			{
			Usecode_value arr = pop();
			Usecode_value val = pop();
			pushi(arr.find_elem(val) >= 0);
			break;
			}
		case 0x31:		// Unknown.
			ip += 4;
			break;
		case 0x32:		// RTS: Push return value & ret. 
					//   from function.
			sp = save_sp;	// Restore stack.
			push(ret_value);
			ip = endp;
			break;
		case 0x33:		// SAY.
			say_string();
			break;
		case 0x38:		// CALLIS.
			{
			offset = Read2(ip);
			sval = *ip++;	// # of parameters.
			Usecode_value ival = call_intrinsic(offset, sval);
			push(ival);
			}
			break;
		case 0x39:		// CALLI.
			offset = Read2(ip);
			sval = *ip++;	// # of parameters.
			call_intrinsic(offset, sval);
			break;
		case 0x3e:		// PUSH ITEMREF.
			pushi((long) caller_item);
			break;
		case 0x3f:		// Guessing some kind of return.
			ip = endp;
			sp = save_sp;		// Restore stack.
			break;
		case 0x40:		// Unknown.
			break;
		case 0x42:		// PUSHF.
			offset = Read2(ip);
			pushi(gflags[offset]);
			break;
		case 0x43:		// POPF.
			offset = Read2(ip);
			gflags[offset] = (unsigned char) popi();
			break;
		case 0x44:		// PUSHW.
			pushi(*ip++);
			break;
		case 0x45:		// Unknown.
			ip++;
			break;
		case 0x46:		// Unknown.
			ip++;
			break;
		case 0x47:		// CALLE.
			offset = Read2(ip);
			call_usecode_function(offset);
			break;
		case 0x48:		// PUSH EVENTID.
			pushi(event);
			break;
		case 0x4a:		// ARRA.
			{
			Usecode_value val = pop();
			Usecode_value arr = pop();
			push(arr.concat(val));
			break;
			}
		case 0x4b:		// POP EVENTID.
			event = popi();
			break;
		default:
			cout << "Opcode " << opcode << " not known.\n";
			break;
			}
		}
	delete [] locals;
					// Restore list of answers.
	num_answers = save_num_answers;
	for (int i = 0; i < num_answers; i++)
		answers[i] = save_answers[i];
	printf("RETurning from usecode %04x\n", fun->id);
	}

/*
 *	Call a usecode function.
 *
 *	Output:	0 if not found.
 */

int Usecode_machine::call_usecode_function
	(
	int id,
	int event,			// Event (?) that caused this call.
	Usecode_value *parm0		// If non-zero, pass this parm.
	)
	{
	int i;				// +++++Hash them!
	for (i = 0; i < num_funs; i++)
		if (funs[i]->id == id)
			break;
	if (i == num_funs)
		return (0);
	if (parm0)
		push(*parm0);
	run(funs[i], event);		// Do it.
	return (1);
	}

#if 0
/*
 *	Testing...
 */

int main(int argc, char **argv)
	{
	ifstream ufile("static/usecode");
	Usecode_machine interp(ufile);
	Usecode_value parm(0);		// They all seem to take 1 parm.
//	interp.call_usecode(0x9b, &parm);	// Try function #2.
	int id;
	if (argc > 1)
		{
		char *stop;
		id = strtoul(argv[1], &stop, 16);
		}
	else
		id = 0x269;
	interp.call_usecode(id, &parm);
	return (0);
	}
#endif
