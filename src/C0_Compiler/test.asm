.386
.model flat
option casemap: none
includelib lib\msvcrt.lib
printf proto c
scanf proto c
.data
$a dword 0
$b dword 0
$c dword 0
@str byte "please input 3 numbers:", 0
@str1 byte "max:", 0
@str2 byte "middle:", 0
@str3 byte "min:", 0
@figure dword ?
@figure_format_d byte "%d", 0
@figure_format_c byte "%c", 0
@figure_format_f byte "%f", 0
@space byte 20h, 0
@new_line byte 0dh, 0ah, 0

.code
_main proc
push	ebx
push	ebp
push	esi
push	edi
mov	ebp, esp
sub	esp, 32
lea	eax, @str
push	eax
call	printf
add	esp, 4
lea	eax, @new_line
push	eax
call	printf
add	esp, 4
lea	eax, @figure
push	eax
lea	eax, @figure_format_d
push	eax
call	scanf
add	esp, 8
mov	eax, @figure
mov	$a, eax
lea	eax, @figure
push	eax
lea	eax, @figure_format_d
push	eax
call	scanf
add	esp, 8
mov	eax, @figure
mov	$b, eax
lea	eax, @figure
push	eax
lea	eax, @figure_format_d
push	eax
call	scanf
add	esp, 8
mov	eax, @figure
mov	$c, eax
push	$c
push	$b
call	_max
add	esp, 8
push	eax
push	$a
call	_max
add	esp, 8
mov	edi, eax
push	$c
push	$b
push	$a
call	_middle
add	esp, 12
mov	esi, eax
push	$c
push	$b
call	_min
add	esp, 8
push	eax
push	$a
call	_min
add	esp, 8
mov	ebx, eax
lea	eax, @str1
push	eax
call	printf
add	esp, 4
push	edi
lea	eax, @figure_format_d
push	eax
call	printf
add	esp, 8
lea	eax, @new_line
push	eax
call	printf
add	esp, 4
lea	eax, @str2
push	eax
call	printf
add	esp, 4
push	esi
lea	eax, @figure_format_d
push	eax
call	printf
add	esp, 8
lea	eax, @new_line
push	eax
call	printf
add	esp, 4
lea	eax, @str3
push	eax
call	printf
add	esp, 4
push	ebx
lea	eax, @figure_format_d
push	eax
call	printf
add	esp, 8
lea	eax, @new_line
push	eax
call	printf
add	esp, 4
L8:
@main_end:
mov	esp, ebp
pop	edi
pop	esi
pop	ebp
pop	ebx
ret
_main endp
_max proc
push	ebx
push	ebp
push	esi
push	edi
mov	ebp, esp
mov	eax, dword ptr [ebp + 20]
cmp	eax, dword ptr [ebp + 24]
jl	L1
mov	dword ptr [ebp + 20], eax
mov	eax, eax
jmp	@max_end
mov	dword ptr [ebp + 20], eax
jmp	L2
L1:
mov	eax, dword ptr [ebp + 24]
jmp	@max_end
L2:
@max_end:
mov	esp, ebp
pop	edi
pop	esi
pop	ebp
pop	ebx
ret
_max endp
_middle proc
push	ebx
push	ebp
push	esi
push	edi
mov	ebp, esp
sub	esp, 16
push	dword ptr [ebp + 28]
push	dword ptr [ebp + 24]
call	_max
add	esp, 8
mov	ecx, dword ptr [ebp + 20]
cmp	ecx, eax
jg	L6
push	dword ptr [ebp + 28]
push	dword ptr [ebp + 24]
call	_min
add	esp, 8
mov	ecx, dword ptr [ebp + 20]
cmp	ecx, eax
jl	L5
mov	eax, dword ptr [ebp + 20]
jmp	@middle_end
jmp	L7
L5:
push	dword ptr [ebp + 28]
push	dword ptr [ebp + 24]
call	_min
add	esp, 8
mov	dword ptr [ebp - 4], eax
mov	eax, eax
jmp	@middle_end
jmp	L7
L6:
push	dword ptr [ebp + 28]
push	dword ptr [ebp + 24]
call	_max
add	esp, 8
mov	dword ptr [ebp - 8], eax
mov	eax, eax
jmp	@middle_end
L7:
@middle_end:
mov	esp, ebp
pop	edi
pop	esi
pop	ebp
pop	ebx
ret
_middle endp
_min proc
push	ebx
push	ebp
push	esi
push	edi
mov	ebp, esp
mov	eax, dword ptr [ebp + 20]
cmp	eax, dword ptr [ebp + 24]
jl	L3
mov	dword ptr [ebp + 20], eax
mov	eax, dword ptr [ebp + 24]
jmp	@min_end
mov	dword ptr [ebp + 20], eax
jmp	L4
L3:
mov	eax, dword ptr [ebp + 20]
jmp	@min_end
L4:
@min_end:
mov	esp, ebp
pop	edi
pop	esi
pop	ebp
pop	ebx
ret
_min endp
_start:
call	_main
ret
end _start
