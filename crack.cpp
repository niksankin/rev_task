#include "windows.h"
#include <stdlib.h> 
#include <string.h> 
#include <tchar.h> 
#include <iostream>
#include <vector>
#include <iomanip>

#include "sha256.h"
#include "rc4.h"

LONG WINAPI vectored_handler(_EXCEPTION_POINTERS *);

BYTE *generate_start;	//address of crypted code start point
BYTE *generate_end;	//of crypted end start point
DWORD size;	//size of crypted code

BYTE* hidden_prepare_start = NULL;
BYTE* hidden_prepare_end = NULL;
BYTE* hidden_prepare_ptr;

std::string str_flag;
BYTE flag[256] = {};
BYTE flag_size = 0;

int generate_flag() {
	std::cout << "Enter data: " << std::endl;
	std::cin >> str_flag;

	flag_size = str_flag.size() % 255;

	// Data will be xored with gamma bytes if process will be launched without debugger
	for (int i = 0; i < flag_size; ++i)
		flag[i] ^= str_flag[i];

	flag[flag_size] = '\0';

	for (std::size_t i = 0; i < flag_size; ++i)
	{
		__asm {
			lea eax, flag
			add eax, i
			// The next two instructions will be replaced in runtime
			mov bl, [eax]
			xor bl, 0x94
			mov [eax], bl

			// Two instructions with opcodes
			// {0x8a, 0x18, 0x80}
			// will be replaced by instructions with opcodes
			// {0x8a, 0x58, 0x01, 0x32, 0x18}
		}
	}

	// This will not cause any exceptions, but if we replace fs:[0] by ds:[0], it will
	__asm {
		xor eax, fs:[0]
	}

	std::cout << "Encrypted flag: ";
	for(std::size_t i = 0; i < flag_size; ++i)
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)flag[i];

	std::cout << std::endl;

	return 0;
}

int ThrashFunc1() {
	return 0;
}

int ThrashFunc2() {		
	return 0;
}

struct Hidden {
	PVOID h1;	// Handler to VEH
	DWORD old_flag;	// Old memory assigment flag
	
	// Change access rights to the code block in function generate_flag from write only to rw
	Hidden() {
		__asm {
			lea eax, generate_start
			lea ebx, generate_flag
			mov dword ptr[eax], ebx
			lea eax, generate_end
			lea ebx, ThrashFunc1
			mov dword ptr[eax], ebx
		}

		size = (DWORD)(generate_end)-(DWORD)(generate_start);
		VirtualProtect(generate_start, size, PAGE_EXECUTE_READWRITE, &old_flag);
	}

	// Abort VEH registration and restore memory access rights
	~Hidden() {
		RemoveVectoredExceptionHandler(h1);
		VirtualProtect(generate_start, size, old_flag, &old_flag);
	}

	// Changes code of generate_flag function in runtime
	int hidden_prepare_code() {
		BYTE* local_gen_ptr = generate_start;
		DWORD local_size = size;

		// Hand-made IsDebuggerPresent code
		__asm {
			mov eax, fs:0x18
			mov eax, [eax + 0x30]
			mov al, [eax + 2]
			push ss
			pop ss
			pushf
			pop bx
			or al, bh
			and al, 1
			test al, al
			jne passed
		}

		// get address of code block that should be modified
		BYTE* xor_op = NULL;

		// Obfuscated byte comparsion method
		for (int i = 0; i < local_size; ++i) {
			if (local_gen_ptr[i] == 0x8A && (local_gen_ptr[i + 1] ^ local_gen_ptr[i]) == 0x92 && (local_gen_ptr[i + 2] ^ (local_gen_ptr[i + 1] ^ local_gen_ptr[i])) == 0x12) {
				xor_op = local_gen_ptr + i;
				break;
			}
		}

		// Opcodes of the new generate_flag xor algorithm. It is equals to:
		// mov bl, [eax+1]
		// xor bl, [eax]
		BYTE opcodes[] = { 0x8a, 0x58, 0x01, 0x32, 0x18 };

		for (int i = 0; i < sizeof(opcodes); ++i)
			xor_op[i] = opcodes[i];

		// Change instruction 
		// xor eax, fs:[0]
		// to
		// xor eax, ds:[0]
		// that will cause an exception that will be handled by VectoredHandler1
		xor_op[sizeof(opcodes) + 4] = 0x3e;

		__asm {
			passed:
		}

		return 0;
	}

	// Prepares some data that will be used by algorithm
	int hidden_prepare_data() {
		// Hand-made IsDebuggerPresent code
		__asm {
			mov eax, fs:0x18
			mov eax, [eax + 0x30]
			mov al, [eax + 2]
			push ss
			pop ss
			pushf
			pop bx
			or al, bh
			and al, 1
			test al, al
			jne passed
		}

		BYTE gamma[] = { 40, 166, 68, 207, 194, 16, 35, 48, 239, 105, 145, 117, 182, 35, 109, 225 };

		// Fill gamma
		for (int i = 0; i < 256; i += sizeof(gamma))
			memcpy(flag + i, gamma, sizeof(gamma));

		// Move hidden_prepare_code address in variable for next calculations
		__asm {
			lea eax, hidden_prepare_ptr
			lea ebx, Hidden::hidden_prepare_code
			mov dword ptr[eax], ebx
		}

		// Find start and end addresses for hidden_prepare_code static code block
		// Here we also use obfuscated byte comparsion method
		for (int i = 0; ; ++i) {
			if (hidden_prepare_ptr[i] == 0x64 &&
				(hidden_prepare_ptr[i + 1] ^ hidden_prepare_ptr[i]) == 0xc5 &&
				(hidden_prepare_ptr[i + 2] ^ (hidden_prepare_ptr[i + 1] ^ hidden_prepare_ptr[i])) == 0xdd) {
				hidden_prepare_start = hidden_prepare_ptr + i;
			}

			if (hidden_prepare_ptr[i] == 0xC6 &&
				(hidden_prepare_ptr[i + 1] ^ hidden_prepare_ptr[i]) == 0xC2 &&
				(hidden_prepare_ptr[i + 2] ^ (hidden_prepare_ptr[i + 1] ^ hidden_prepare_ptr[i])) == 0xC8 &&
				(hidden_prepare_ptr[i + 3] ^ (hidden_prepare_ptr[i + 2] ^ (hidden_prepare_ptr[i + 1] ^ hidden_prepare_ptr[i]))) == 0xF6) {
				hidden_prepare_end = hidden_prepare_ptr + i + 4;
				break;
			}
		}

		__asm {
		passed:
		}

		return 0;
	}

	// Registrate our custom VEH
	int hidden_add_veh() {
		LONG(_stdcall * veh) (_EXCEPTION_POINTERS*) = vectored_handler;

		h1 = AddVectoredExceptionHandler(1, veh);

		return 0;
	}
};

// Static variables are initialized before calling main function
// This will help us to execute some code before main
Hidden kek = Hidden();
int lol = kek.hidden_prepare_code();
int lel = kek.hidden_prepare_data();
int puk = kek.hidden_add_veh();

SHA256_CTX ctx;
unsigned char hash[SHA256_BLOCK_SIZE] = {};

//VEH naked-handler, calculates sha256 and rc4
__declspec(naked) LONG WINAPI vectored_handler(struct _EXCEPTION_POINTERS* e) {
	//It's only him
	/*__asm {
		jmp Jojo
		_emit 'Y'
		_emit 'o'
		_emit 'u'
		_emit '`'
		_emit 'v'
		_emit 'e'
		_emit ' '
		_emit 'e'
		_emit 'x'
		_emit 'p'
		_emit 'e'
		_emit 'c'
		_emit 't'
		_emit 'e'
		_emit 'd'
		_emit ' '
		_emit 'a'
		_emit ' '
		_emit 'f'
		_emit 'l'
		_emit 'a'
		_emit 'g'
		_emit '?'
		_emit ' '
		_emit 'B'
		_emit 'u'
		_emit 't'
		_emit ' '
		_emit 'h'
		_emit 'e'
		_emit 'r'
		_emit 'e'
		_emit ' '
		_emit 'i'
		_emit 's'
		_emit ' '
		_emit 'o'
		_emit 'n'
		_emit 'l'
		_emit 'y'
		_emit ' '
		_emit 'm'
		_emit 'e'
		_emit ','
		_emit ' '
		_emit 'D'
		_emit 'i'
		_emit 'o'
		_emit '!'
		Jojo:
	}*/

	// Hand-made IsDebuggerPresent code
	__asm {
		mov eax, fs:0x18
		mov eax, [eax + 0x30]
		mov al, [eax + 2]
		push ss
		pop ss
		pushf
		pop bx
		or al, bh
		and al, 1
		test al, al
		je passed
	}

	// If debugger is found - leave exception unhandled
	__asm {
		mov eax, EXCEPTION_CONTINUE_SEARCH
		ret 4
	}

	__asm {
	passed:
	}

	// Calculate SHA256 from hidden_prepare_code static code block
	sha256_init(&ctx);
	sha256_update(&ctx, (const unsigned char*)hidden_prepare_start, ((DWORD)hidden_prepare_end - (DWORD)hidden_prepare_start));
	sha256_final(&ctx, hash);

	// Init RC4 using previously found hash as a key
	rc4_init(hash, SHA256_BLOCK_SIZE);

	// Encrypt flag, that was modified in generate_flag
	for (int i = 0; i < flag_size; ++i)
		flag[i] ^= rc4_output();

	// We need to skip instruction that causes exception, so we add 7 to EIP value stored in _EXCEPTION_POINTERS structure
	// We use assembly offsets instead of C reference because of __declspec(naked) attribute
	__asm {
		mov eax, [esp + 0x4]
		mov ecx, [eax + 4]
		mov eax, ecx
		add[eax + 0xb8], 7
	}

	// This handler is the last in the chain, so we must continue normal execution flow
	__asm {
		mov eax, EXCEPTION_CONTINUE_EXECUTION
		ret 4
	}
}

int main()
{
	__asm {		//push all variables to make code static
		lea ebx, gen_flag		//mov
	}
	
	//obfuscation
	__asm {
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		ror eax, 0
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 0
		rol eax, 0
		sub eax, 0
		cmp ecx, ecx
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 1
		sub eax, 1
		ror eax, 3
		rol eax, 3
		cmp ecx, ecx
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 1
		sub eax, 1
		ror eax, 3
		rol eax, 3
		cmp ecx, ecx
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 1
		sub eax, 1
		ror eax, 3
		rol eax, 3
		cmp ecx, ecx
	}

	__asm {
		push ebx
		push esi
	}

	//obfuscation
	__asm {
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 1
		sub eax, 1
		ror eax, 3
		rol eax, 3
		cmp ecx, ecx
	}

	//ciphered flag
	__asm {
		pop esi
		pop ebx
		jmp ebx
	}

	//obfuscation
	__asm {
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 1
		sub eax, 1
		ror eax, 3
		rol eax, 3
		cmp ecx, ecx
	}

	__asm {
	gen_flag:
	}

	generate_flag();

	//obfuscation
	__asm {
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 1
		sub eax, 1
		ror eax, 3
		rol eax, 3
		cmp ecx, ecx
	}

	//obfuscation
	__asm {
		add eax, 0
		cmp edx, edx
		rol edx, 2
		ror edx, 2
		test eax, eax
		add ecx, 1
		sub ecx, 1
		ror ecx, 6
		rol ecx, 6
		add edx, 1
		add edx, 0
		add ecx, 0
		sub edx, 1
		add eax, 1
		sub eax, 1
		ror eax, 3
		rol eax, 3
		cmp ecx, ecx
	}

	return 0;
}