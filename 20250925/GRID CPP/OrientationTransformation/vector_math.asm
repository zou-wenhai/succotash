; File: vector_math.asm
; Assemble with MASM for x64. Note that VS x64 doesn’t support inline asm in C++.
; The following routines assume that the arrays are of type std::array<double,3>.
; 
; Dot product function:
;   Signature (C++): extern "C" double dotProduct(const double* m, const double* n);
;   Parameters:
;     RCX = pointer to m (first array)
;     RDX = pointer to n (second array)
;   Return:
;     XMM0 contains the double dot product result.
;
; Cross product function:
;   Signature (C++): extern "C" std::array<double,3> crossProduct(const double* m, const double* n);
;   Because the returned struct (24 bytes) is larger than 16 bytes, the caller allocates space
;   and passes a hidden pointer in RCX. Then:
;     RCX = pointer to return storage (3 doubles)
;     RDX = pointer to m (first array)
;     R8  = pointer to n (second array)
;   The routine computes:
;     result[0] = m[1]*n[2] - m[2]*n[1]
;     result[1] = m[2]*n[0] - m[0]*n[2]
;     result[2] = m[0]*n[1] - m[1]*n[0]


; File: vector_math_long.asm
; MASM code for computing dot and cross products on arrays of 3 long ints.
; Assumes "long" is 32-bit.
; 
; dotProductLong:
;   C++ signature: extern "C" long dotProductLong(const long* m, const long* n);
;   Parameters:
;     RCX = pointer to first array (m)
;     RDX = pointer to second array (n)
;   Returns the dot product (m[0]*n[0] + m[1]*n[1] + m[2]*n[2]) in EAX.
;
; crossProductLong:
;   C++ signature (explicit return pointer version):
;     extern "C" void crossProductLong(long* ret, const long* m, const long* n);
;   Parameters:
;     RCX = pointer to return storage (3 long ints = 12 bytes)
;     RDX = pointer to m (first array)
;     R8  = pointer to n (second array)
;   Computes:
;     ret[0] = m[1]*n[2] - m[2]*n[1]
;     ret[1] = m[2]*n[0] - m[0]*n[2]
;     ret[2] = m[0]*n[1] - m[1]*n[0]

PUBLIC dotProductDouble
PUBLIC crossProductDouble

PUBLIC dotProductDouble_Optimized
PUBLIC crossProductDouble_Optimized

PUBLIC dotProductLong
PUBLIC crossProductLong

PUBLIC dotProductLong_Optimized
PUBLIC crossProductLong_Optimized

.code

;---------------------------------------------------------------
; double dotProduct(const double* m, const double* n);
dotProductDouble PROC
    movsd   xmm0, qword ptr [rcx]       ; xmm0 = m[0]
    movsd   xmm1, qword ptr [rdx]       ; xmm1 = n[0]
    mulsd   xmm0, xmm1                ; xmm0 = m[0] * n[0]
    movsd   xmm1, qword ptr [rcx+8]    ; xmm1 = m[1]
    movsd   xmm2, qword ptr [rdx+8]    ; xmm2 = n[1]
    mulsd   xmm1, xmm2                ; xmm1 = m[1] * n[1]
    addsd   xmm0, xmm1                ; xmm0 += m[1]*n[1]
    movsd   xmm1, qword ptr [rcx+16]   ; xmm1 = m[2]
    movsd   xmm2, qword ptr [rdx+16]   ; xmm2 = n[2]
    mulsd   xmm1, xmm2                ; xmm1 = m[2] * n[2]
    addsd   xmm0, xmm1                ; xmm0 += m[2]*n[2]
    ret
dotProductDouble ENDP

; Optimized dotProductDouble using SSE2 instructions
; Computes: result = m[0]*n[0] + m[1]*n[1] + m[2]*n[2]
dotProductDouble_Optimized PROC
    movupd   xmm0, qword ptr [rcx]      ; load m[0] and m[1]
    movupd   xmm1, qword ptr [rdx]      ; load n[0] and n[1]
    mulpd    xmm0, xmm1                ; pairwise multiply m[0]*n[0], m[1]*n[1]
    haddpd   xmm0, xmm0                ; horizontally add: xmm0[0] = m[0]*n[0] + m[1]*n[1]
    movsd    xmm1, qword ptr [rcx+16]   ; load m[2]
    movsd    xmm2, qword ptr [rdx+16]   ; load n[2]
    mulsd    xmm1, xmm2                ; m[2] * n[2]
    addsd   xmm0, xmm1                ; add the third product
    ret
dotProductDouble_Optimized ENDP

;---------------------------------------------------------------
; void crossProduct(double* ret, const double* m, const double* n);
; RCX = pointer to return buffer (3 doubles)
; RDX = pointer to m, R8 = pointer to n
crossProductDouble PROC
    ; Compute ret[0] = m[1]*n[2] - m[2]*n[1]
    movsd   xmm0, qword ptr [rdx+8]   ; xmm0 = m[1]
    movsd   xmm1, qword ptr [r8+16]     ; xmm1 = n[2]
    mulsd   xmm0, xmm1                ; xmm0 = m[1]*n[2]
    movsd   xmm1, qword ptr [rdx+16]  ; xmm1 = m[2]
    movsd   xmm2, qword ptr [r8+8]      ; xmm2 = n[1]
    mulsd   xmm1, xmm2                ; xmm1 = m[2]*n[1]
    subsd   xmm0, xmm1                ; xmm0 = m[1]*n[2] - m[2]*n[1]
    movsd   qword ptr [rcx], xmm0     ; store in ret[0]

    ; Compute ret[1] = m[2]*n[0] - m[0]*n[2]
    movsd   xmm0, qword ptr [rdx+16]  ; xmm0 = m[2]
    movsd   xmm1, qword ptr [r8]       ; xmm1 = n[0]
    mulsd   xmm0, xmm1                ; xmm0 = m[2]*n[0]
    movsd   xmm1, qword ptr [rdx]      ; xmm1 = m[0]
    movsd   xmm2, qword ptr [r8+16]    ; xmm2 = n[2]
    mulsd   xmm1, xmm2                ; xmm1 = m[0]*n[2]
    subsd   xmm0, xmm1                ; xmm0 = m[2]*n[0] - m[0]*n[2]
    movsd   qword ptr [rcx+8], xmm0   ; store in ret[1]

    ; Compute ret[2] = m[0]*n[1] - m[1]*n[0]
    movsd   xmm0, qword ptr [rdx]      ; xmm0 = m[0]
    movsd   xmm1, qword ptr [r8+8]     ; xmm1 = n[1]
    mulsd   xmm0, xmm1                ; xmm0 = m[0]*n[1]
    movsd   xmm1, qword ptr [rdx+8]    ; xmm1 = m[1]
    movsd   xmm2, qword ptr [r8]       ; xmm2 = n[0]
    mulsd   xmm1, xmm2                ; xmm1 = m[1]*n[0]
    subsd   xmm0, xmm1                ; xmm0 = m[0]*n[1] - m[1]*n[0]
    movsd   qword ptr [rcx+16], xmm0  ; store in ret[2]

    ret
crossProductDouble ENDP

; crossProductDouble_Optimized
; Computes the same result as your original scalar version:
;    ret[0] = m[1]*n[2] - m[2]*n[1]
;    ret[1] = m[2]*n[0] - m[0]*n[2]
;    ret[2] = m[0]*n[1] - m[1]*n[0]
;
; This version uses SSE/SSE3 instructions to do a pair of
; multiplications in parallel, then a horizontal subtract.

;  Requirements:
;    - Uses SSE2 instructions for packed loads/multiplies.
;    - Uses SSE3 instruction HSUBPD for the horizontal subtract.
;    - If you must remain strictly SSE2-only, you can replace
;      HSUBPD with a shuffle + SUBPD sequence.

crossProductDouble_Optimized PROC
    ; ---------------------------------------------------------------------
    ; ret[0] = m[1]*n[2] - m[2]*n[1]
    ; ---------------------------------------------------------------------
    movsd   xmm0, qword ptr [rdx+8]          ; xmm0 = { m[1], ?? }
    movsd   xmm2, qword ptr [rdx+16]         ; xmm2 = { m[2], ?? }
    unpcklpd xmm0, xmm2            ; xmm0 = { m[1], m[2] }

    movsd   xmm1, qword ptr [r8+16]          ; xmm1 = { n[2], ?? }
    movsd   xmm3, qword ptr [r8+8]           ; xmm3 = { n[1], ?? }
    unpcklpd xmm1, xmm3            ; xmm1 = { n[2], n[1] }

    mulpd   xmm0, xmm1             ; xmm0 = { m[1]*n[2], m[2]*n[1] }

    ; Horizontal subtract (SSE3): low double = (xmm0[0] - xmm0[1])
    hsubpd  xmm0, xmm0             ; xmm0 = { m[1]*n[2] - m[2]*n[1], *don't care* }

    movsd   qword ptr [rcx], xmm0            ; store ret[0]

    ; ---------------------------------------------------------------------
    ; ret[1] = m[2]*n[0] - m[0]*n[2]
    ; ---------------------------------------------------------------------
    movsd   xmm4, qword ptr [rdx+16]         ; xmm4 = { m[2], ?? }
    movsd   xmm6, qword ptr [rdx]            ; xmm6 = { m[0], ?? }
    unpcklpd xmm4, xmm6            ; xmm4 = { m[2], m[0] }

    movsd   xmm5, qword ptr [r8]             ; xmm5 = { n[0], ?? }
    movsd   xmm7, qword ptr [r8+16]          ; xmm7 = { n[2], ?? }
    unpcklpd xmm5, xmm7            ; xmm5 = { n[0], n[2] }

    mulpd   xmm4, xmm5             ; xmm4 = { m[2]*n[0], m[0]*n[2] }
    hsubpd  xmm4, xmm4             ; xmm4 = { m[2]*n[0] - m[0]*n[2], *don't care* }

    movsd   qword ptr [rcx+8], xmm4          ; store ret[1]

    ; ---------------------------------------------------------------------
    ; ret[2] = m[0]*n[1] - m[1]*n[0]
    ; ---------------------------------------------------------------------
    movsd   xmm0, qword ptr [rdx]            ; xmm0 = { m[0], ?? }
    movsd   xmm2, qword ptr [rdx+8]          ; xmm2 = { m[1], ?? }
    unpcklpd xmm0, xmm2            ; xmm0 = { m[0], m[1] }

    movsd   xmm1, qword ptr [r8+8]           ; xmm1 = { n[1], ?? }
    movsd   xmm3, qword ptr [r8]             ; xmm3 = { n[0], ?? }
    unpcklpd xmm1, xmm3            ; xmm1 = { n[1], n[0] }

    mulpd   xmm0, xmm1             ; xmm0 = { m[0]*n[1], m[1]*n[0] }
    hsubpd  xmm0, xmm0             ; xmm0 = { m[0]*n[1] - m[1]*n[0], *don't care* }

    movsd   qword ptr [rcx+16], xmm0         ; store ret[2]
    ret
crossProductDouble_Optimized ENDP



;---------------------------------------------------------------
; long dotProductLong(const long* m, const long* n);
; RCX = pointer to m, RDX = pointer to n.
; Returns result in EAX.
dotProductLong PROC
    ; Compute m[0] * n[0]
    mov   eax, dword ptr [rcx]       ; eax = m[0]
    mov   r8d, dword ptr [rdx]         ; r8d = n[0]
    imul  eax, r8d                   ; eax = m[0] * n[0]
    mov   r9d, eax                   ; r9d will accumulate the sum

    ; Compute m[1] * n[1]
    mov   eax, dword ptr [rcx+4]     ; eax = m[1]
    mov   r8d, dword ptr [rdx+4]       ; r8d = n[1]
    imul  eax, r8d                   ; eax = m[1] * n[1]
    add   r9d, eax                   ; r9d += m[1]*n[1]

    ; Compute m[2] * n[2]
    mov   eax, dword ptr [rcx+8]     ; eax = m[2]
    mov   r8d, dword ptr [rdx+8]       ; r8d = n[2]
    imul  eax, r8d                   ; eax = m[2] * n[2]
    add   r9d, eax                   ; r9d += m[2]*n[2]

    mov   eax, r9d                   ; move accumulated sum into eax
    ret
dotProductLong ENDP

; dotProductLong_Optimized
; ---------------------------------------------------------
; Calculates the sum of products:
;   m[0]*n[0] + m[1]*n[1] + m[2]*n[2]
; ---------------------------------------------------------
; Requirements:
;   - SSE4.1 (for PMULLD and PINSRD)
;   - SSSE3  (for PHADDD)
;   - The final 32-bit sum is returned in EAX
;   - Same overflow behavior as the original scalar IMUL
;     (no 64-bit intermediate accumulation)
; ---------------------------------------------------------
dotProductLong_Optimized PROC
    ;--- Load m into XMM0: (m0, m1, m2, 0) ---
    mov   eax, dword ptr [rcx]           ; m[0]
    movd  xmm0, eax            ; xmm0 = (m0,  0,   0,   0)
    mov   eax, dword ptr [rcx+4]         ; m[1]
    pinsrd xmm0, eax, 1        ; xmm0 = (m0,  m1,  0,   0)
    mov   eax, dword ptr [rcx+8]         ; m[2]
    pinsrd xmm0, eax, 2        ; xmm0 = (m0,  m1,  m2,  0)

    ;--- Load n into XMM1: (n0, n1, n2, 0) ---
    mov   eax, dword ptr [rdx]           ; n[0]
    movd  xmm1, eax            ; xmm1 = (n0,  0,   0,   0)
    mov   eax, dword ptr [rdx+4]         ; n[1]
    pinsrd xmm1, eax, 1        ; xmm1 = (n0,  n1,  0,   0)
    mov   eax, dword ptr [rdx+8]         ; n[2]
    pinsrd xmm1, eax, 2        ; xmm1 = (n0,  n1,  n2,  0)

    ;--- Pairwise multiply (SSE4.1: PMULLD) ---
    pmulld xmm0, xmm1          ; xmm0 = (m0*n0, m1*n1, m2*n2, 0)

    ;--- Sum the 3 products horizontally (SSSE3: PHADDD) ---
    ; 1st PHADDD: (m0*n0 + m1*n1, m2*n2 + 0, ..., ...)
    phaddd xmm0, xmm0
    ; 2nd PHADDD: adds the two results -> final sum in the low 32 bits
    phaddd xmm0, xmm0

    ;--- Move final 32-bit sum to EAX ---
    movd  eax, xmm0
    ret
dotProductLong_Optimized ENDP

;---------------------------------------------------------------
; void crossProductLong(long* ret, const long* m, const long* n);
; RCX = pointer to return storage (3 long ints = 12 bytes)
; RDX = pointer to m, R8 = pointer to n.
; Computes:
;   ret[0] = m[1]*n[2] - m[2]*n[1]
;   ret[1] = m[2]*n[0] - m[0]*n[2]
;   ret[2] = m[0]*n[1] - m[1]*n[0]
crossProductLong PROC
    ;--- Compute ret[0] = m[1]*n[2] - m[2]*n[1] ---
    mov   eax, dword ptr [rdx+4]     ; eax = m[1]
    mov   r10d, dword ptr [r8+8]       ; r10d = n[2]
    imul  eax, r10d                  ; eax = m[1] * n[2]
    mov   r11d, eax                  ; r11d = m[1]*n[2]
    mov   eax, dword ptr [rdx+8]     ; eax = m[2]
    mov   r10d, dword ptr [r8+4]       ; r10d = n[1]
    imul  eax, r10d                  ; eax = m[2] * n[1]
    sub   r11d, eax                  ; r11d = m[1]*n[2] - m[2]*n[1]
    mov   dword ptr [rcx], r11d      ; store result in ret[0]

    ;--- Compute ret[1] = m[2]*n[0] - m[0]*n[2] ---
    mov   eax, dword ptr [rdx+8]     ; eax = m[2]
    mov   r10d, dword ptr [r8]         ; r10d = n[0]
    imul  eax, r10d                  ; eax = m[2] * n[0]
    mov   r11d, eax                  ; r11d = m[2]*n[0]
    mov   eax, dword ptr [rdx]       ; eax = m[0]
    mov   r10d, dword ptr [r8+8]       ; r10d = n[2]
    imul  eax, r10d                  ; eax = m[0] * n[2]
    sub   r11d, eax                  ; r11d = m[2]*n[0] - m[0]*n[2]
    mov   dword ptr [rcx+4], r11d    ; store result in ret[1]

    ;--- Compute ret[2] = m[0]*n[1] - m[1]*n[0] ---
    mov   eax, dword ptr [rdx]       ; eax = m[0]
    mov   r10d, dword ptr [r8+4]       ; r10d = n[1]
    imul  eax, r10d                  ; eax = m[0] * n[1]
    mov   r11d, eax                  ; r11d = m[0]*n[1]
    mov   eax, dword ptr [rdx+4]     ; eax = m[1]
    mov   r10d, dword ptr [r8]         ; r10d = n[0]
    imul  eax, r10d                  ; eax = m[1] * n[0]
    sub   r11d, eax                  ; r11d = m[0]*n[1] - m[1]*n[0]
    mov   dword ptr [rcx+8], r11d    ; store result in ret[2]

    ret
crossProductLong ENDP

; crossProductLong_Optimized
; ---------------------------------------------------------
; Computes the same result as your scalar routine:
;    ret[0] = m[1]*n[2] - m[2]*n[1]
;    ret[1] = m[2]*n[0] - m[0]*n[2]
;    ret[2] = m[0]*n[1] - m[1]*n[0]
; ---------------------------------------------------------
; RCX = pointer to return storage  (3 x 32-bit = 12 bytes)
; RDX = pointer to m (3 x 32-bit)
; R8  = pointer to n (3 x 32-bit)
; ---------------------------------------------------------
; Uses SSE4.1 and SSSE3 instructions:
;   - PMULLD  (SSE4.1)   : pairwise 32-bit multiply
;   - PHSUBD  (SSSE3)    : horizontal subtract on packed ints
; ---------------------------------------------------------
crossProductLong_Optimized PROC

    ;-------------------------------------------------------
    ; ret[0] = m[1]*n[2] - m[2]*n[1]
    ;-------------------------------------------------------
    mov     eax, dword ptr [rdx+4]  ; eax = m[1]
    movd    xmm0, eax                ; xmm0 = (m[1], 0, 0, 0)
    mov	    eax, dword ptr [rdx+8]  ; eax = m[2]
    pinsrd  xmm0, eax, 1             ; xmm0 = (m[1], m[2], 0, 0)

    mov     eax, dword ptr [r8+8]   ; eax = n[1]
    movd    xmm1, eax   ; xmm1 = (n[2], 0, 0, 0)
    mov     eax, dword ptr [r8+4]   ; eax = n[2]
    pinsrd  xmm1, eax, 1             ; xmm1 = (n[1], n[2], 0, 0)

    pmulld  xmm0, xmm1              ; xmm0 = (m[1]*n[2], m[2]*n[1])
    phsubd  xmm0, xmm0              ; xmm0 = { (m[1]*n[2] - m[2]*n[1]), ??? }
    movd    dword ptr [rcx], xmm0    ; store ret[0]

    ;-------------------------------------------------------
    ; ret[1] = m[2]*n[0] - m[0]*n[2]
    ;-------------------------------------------------------
    mov     eax , dword ptr [rdx+8]  ; eax = m[2]
    movd    xmm0, eax  ; xmm0 = (m[2], ...)
    mov     eax, dword ptr [rdx]     ; eax = m[0]
    pinsrd  xmm0, eax, 1             ; xmm0 = (m[2], m[0])

    mov     eax, dword ptr [r8]      ; eax = n[0]
    movd    xmm1, eax     ; xmm1 = (n[0], ...)
    mov     eax, dword ptr [r8+8]    ; eax = n[2]
    pinsrd  xmm1, eax, 1             ; xmm1 = (n[0], n[2])

    pmulld  xmm0, xmm1              ; (m[2]*n[0], m[0]*n[2])
    phsubd  xmm0, xmm0              ; { (m[2]*n[0] - m[0]*n[2]), ??? }
    movd    dword ptr [rcx+4], xmm0  ; store ret[1]

    ;-------------------------------------------------------
    ; ret[2] = m[0]*n[1] - m[1]*n[0]
    ;-------------------------------------------------------
    mov     eax, dword ptr [rdx]     ; eax = m[0]
    movd    xmm0, eax    ; (m[0], 0, ...)
    mov     eax, dword ptr [rdx+4]   ; eax = m[1]
    pinsrd  xmm0, eax, 1             ; xmm0 = (m[0], m[1])

    mov     eax, dword ptr [r8+4]   ; eax = n[1]
    movd    xmm1, eax   ; (n[1], 0, ...)
    mov     eax, dword ptr [r8]     ; eax = n[0]
    pinsrd  xmm1, eax, 1             ; xmm1 = (n[1], n[0])

    pmulld  xmm0, xmm1              ; (m[0]*n[1], m[1]*n[0])
    phsubd  xmm0, xmm0              ; { (m[0]*n[1] - m[1]*n[0]), ??? }
    movd    dword ptr [rcx+8], xmm0  ; store ret[2]

    ret
crossProductLong_Optimized ENDP


END
