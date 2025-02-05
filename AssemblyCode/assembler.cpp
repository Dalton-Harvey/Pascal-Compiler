#include <iostream>
char DataSegment[65536];
int* look;
int main()
{
	look = (int*)DataSegment;
	_asm{
		push eax
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
		push esp
		lea edi, DataSegment
		jmp Kmain

		Kmain:

		mov eax, 1//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 3

		mov eax, 2//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 1

		mov eax, 3//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 4

		mov eax, 4//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 5

		mov eax, 5//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 2

		mov eax, 6//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov ebx, 7//move left side into a register
		neg ebx
		mov [edi+eax], ebx

		mov eax, 7//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 8

		mov eax, 8//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 99

		mov eax, 9//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 0

		mov eax, 10//move left side into a register
		sub eax, 1
		imul eax, 4
		add eax, 12
		//integer(global, 0) = integer(literal, 0)
		mov [edi+eax], 5

		//integer(global, 0) = integer(literal, 0)
		mov [edi+0], 1

	TopWhile1:
		//integer(global, 0) < integer(literal, 0)
		mov eax, [edi+0]//move left side into a register
		cmp eax, 11
		jl InsideWhile1
		jmp EndWhile1
	InsideWhile1:
		//integer(global, 0) = integer(literal, 0)
		mov [edi+4], 2

	TopWhile2:
		//integer(literal, 0) - integer(global, 0)
		mov eax, 12//move left side into a register
		sub eax, [edi+0]

		//integer(global, 0) < integer(, 0)
		mov ecx, [edi+4]//move left side into a register
		cmp ecx, eax
		jl InsideWhile2
		jmp EndWhile2
	InsideWhile2:
		mov ecx, [edi+4]//move left side into a register
		sub ecx, 1
		imul ecx, 4
		add ecx, 12
		mov ecx, [edi+ecx]
		//integer(global, 0) - integer(literal, 0)
		mov eax, [edi+4]//move left side into a register
		sub eax, 1

		sub eax, 1
		imul eax, 4
		add eax, 12
		mov eax, [edi+eax]
		//integer(, 0) < integer(, 0)
		cmp ecx, eax
		jl InsideIf3
		jmp elseif3
	InsideIf3:
		mov ecx, [edi+4]//move left side into a register
		sub ecx, 1
		imul ecx, 4
		add ecx, 12
		mov ecx, [edi+ecx]
		//integer(global, 0) = integer(, 0)
		mov [edi+8], ecx

		mov ecx, [edi+4]//move left side into a register
		sub ecx, 1
		imul ecx, 4
		add ecx, 12
		//integer(global, 0) - integer(literal, 0)
		mov eax, [edi+4]//move left side into a register
		sub eax, 1

		sub eax, 1
		imul eax, 4
		add eax, 12
		mov eax, [edi+eax]
		//integer(global, 0) = integer(, 0)
		mov [edi+ecx], eax

		//integer(global, 0) - integer(literal, 0)
		mov ecx, [edi+4]//move left side into a register
		sub ecx, 1

		sub ecx, 1
		imul ecx, 4
		add ecx, 12
		//integer(global, 0) = integer(global, 0)
		mov eax, [edi+8]//move left side into a register
		mov [edi+ecx], eax

	endif3:
	elseif3:
		//integer(global, 0) + integer(literal, 0)
		mov ecx, [edi+4]//move left side into a register
		add ecx, 1

		//integer(global, 0) = integer(, 0)
		mov [edi+4], ecx

		jmp TopWhile2
	EndWhile2:
		//integer(global, 0) + integer(literal, 0)
		mov ecx, [edi+0]//move left side into a register
		add ecx, 1

		//integer(global, 0) = integer(, 0)
		mov [edi+0], ecx

		jmp TopWhile1
	EndWhile1:
		pop esp
		pop esi
		pop edi
		pop ebp
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
return 0;
}


