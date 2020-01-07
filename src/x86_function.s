section .text

global x86_function

x86_function:
push rbp  ; push "calling procedure" frame pointer
mov rbp, rsp  ; set new frame pointer
; - "this procedure" frame pointer

;------------------------------------------------------------------------------

; rdi - buffer, rsi - points, rdx - number of coords
; free registers: r8, r9

; Registers:
; rcx - i
; xmm6 - t
; xmm1 - 1-t

; Useful funcitons:
; cvtsi2sd - Converts a 32bit integer to the bottom 64bit double.
; cvtss2sd - Converts bottom 32bit single to bottom 64bit double.
; movq - Moves a 64bit value, clearing the top 64bits of an XMM register.
; cvtpd2ps - Converts 2 64bit doubles into 2 32bit singles in
;            the bottom of an XMM register.

;------------------------------------------------------------------------------

mov rcx, 0 ; i = 0

; Set up t value
mov r8, 2
cvtsi2sd xmm0, r8
cvtsi2sd xmm1, rdx
divsd xmm0, xmm1 ; t now has value 2 / rdx
movq xmm6, xmm0 ; save t value

for:
cmp rcx , rdx
jg  done

; (1-t)^4 * x0
mov r8, 1
cvtsi2sd xmm1, r8
subsd xmm1, xmm0 ; xmm1 = 1 - t (save for future)
movq xmm2, xmm1 ; xmm2 = 1 - t
mov r8, 1; we need to start from 1 because we have already (1 - t)^1
mov r9, 4
tmpFor1:
cmp r8, r9
jge doneTmpFor1
movq xmm3, xmm1 ; copy t 1 - t value
mulsd xmm2, xmm3 ; xmm2 *= (1-t)
inc r8
jmp tmpFor1
doneTmpFor1:
movq xmm3, [rsi] ; xmm3 = x coord of point 1
cvtps2pd xmm3, xmm3
mulsd xmm2, xmm3

; ---
; xmm2 = 1st summand
; ---

; 4t(1-t)^3 * x1
mov r8, 4
cvtsi2sd xmm3, r8 ; xmm3 = 4
movq xmm4, xmm0 ; copy t value
mulsd xmm3, xmm4 ; xmm3 = 4t
; Calculation of: xmm3 * (1-t)^3
mov r8, 0
mov r9, 3
tmpFor2:
cmp r8, r9
jge doneTmpFor2
movq xmm4, xmm1
mulsd xmm3, xmm4 ; xmm3 *= (1 - t)
inc r8
jmp tmpFor2
doneTmpFor2: ; now xmm3 = 4t * (1-t)^3
; Calculation of: xmm3 * x1
movq xmm4, [rsi + 8] ; xmm4 = x coord of point 2 (8 = sizeof(float) *
    ; 2 (for x and y coord) * offset (1))
cvtps2pd xmm4, xmm4
mulsd xmm3, xmm4 ; xmm3 = 4t(1-t)^3 * x1
addsd xmm2, xmm3

; ---
; xmm2 = 1 + 2 summands
; ---

; 6t^2(1-t)^2 * x2
mov r8, 6
cvtsi2sd xmm3, r8 ; xmm3 = 6
movq xmm4, xmm0 ; copy t value
mulsd xmm3, xmm4 ; xmm3 = 6t
movq xmm4, xmm0 ; copy t value
mulsd xmm3, xmm4 ; xmm3 = 10t^2 ; p.s. looks like mulsd leaves xmm4 = 0 with AVX flag, so
; for now i will live it like this
; Calculation of: xmm3 * (1-t)^2
mov r8, 0
mov r9, 2
tmpFor3:
cmp r8, r9
jge doneTmpFor3
movq xmm4, xmm1
mulsd xmm3, xmm4 ; xmm3 *= (1 - t)
inc r8
jmp tmpFor3
doneTmpFor3: ; now xmm3 = 6t^2 * (1-t)^2
; Calculation of: xmm3 * x2
movq xmm4, [rsi + 16] ; xmm4 = x coord of point 2 (8 = sizeof(float) *
    ; 2 (for x and y coord) * offset (2))
cvtps2pd xmm4, xmm4
mulsd xmm3, xmm4 ; xmm3 = 5t(1-t)^4 * x2
addsd xmm2, xmm3

; ---
; xmm2 = 1 + 2 + 3 summands
; ---

; 4t^3 * (1-t) * x3
mov r8, 4
cvtsi2sd xmm3, r8 ; xmm3 = 4
movq xmm4, xmm0 ; copy t value
mulsd xmm3, xmm4 ; xmm3 = 4t
movq xmm4, xmm0 ; copy t value
mulsd xmm3, xmm4 ; xmm3 = 4t^2 ; p.s. looks like must leave xmm4 = 0 with AVX flag, so
; for now i will leave it like this
movq xmm4, xmm0 ; copy t value
mulsd xmm3, xmm4 ; xmm3 = 4t^3
movq xmm4, xmm1
mulsd xmm3, xmm4; xmm3 = 4t^3 * (1-t)
; Calculation of: xmm3 * x2
movq xmm4, [rsi + 24] ; xmm4 = x coord of point 2 (8 = sizeof(float) *
    ; 2 (for x and y coord) * offset (3))
cvtps2pd xmm4, xmm4
mulsd xmm3, xmm4 ; xmm3 = 4t^3 * (1-t) * x3
addsd xmm2, xmm3

; ---
; xmm2 = (0), (1), (2), (3)
; ---

; t^4 * x4
movq xmm3, xmm0
mov r8, 1
mov r9, 4
tmpFor4:
cmp r8, r9
jge doneTmpFor4
movq xmm4, xmm0
mulsd xmm3, xmm4 ; xmm3 *=  t
inc r8
jmp tmpFor4
doneTmpFor4: ; now xmm3 = t^4
; Calculation of: xmm3 * x4
movq xmm4, [rsi + 32] ; xmm4 = x coord of point 2 (8 = sizeof(float) *
    ; 2 (for x and y coord) * offset (4))
cvtps2pd xmm4, xmm4
mulsd xmm3, xmm4 ; xmm3 =  t^4 * x4
addsd xmm2, xmm3

; Store value into buffer
cvtpd2ps xmm2, xmm2 ; reduce size of xmm2
movd [rdi], xmm2

add rdi, 4 ; go to next vertex from buffer

mov r8, rcx
and r8, 1
cmp r8, 0 ; if(r8 is even) then increment rdi (i.m. next will be for y coord)
je nextY
sub rsi, 4
inc rcx
addsd xmm0, xmm6; add t value
jmp for

nextY:
add rsi, 4
inc rcx
jmp for

done:
;------------------------------------------------------------------------------

mov rsp, rbp  ; restore original stack pointer
pop rbp   ; restore "calling procedure" frame pointer

section .data
