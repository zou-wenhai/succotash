

PUBLIC quantDotDoule_Optimized
PUBLIC quantDotlong_Optimized

.code

;---------------------------------------------------------------
; double quatDotDoule_Optimized(const double* m, const double* n);

; Optimized dotProductDouble using SSE2 instructions
; Computes: result = m[0]*n[0] + m[1]*n[1] + m[2]*n[2] + m[3]*n[3]

quantDotDoule_Optimized PROC
    MOVUPD xmm0, QWORD ptr [rcx] ; load m[0] and m[1]
    MOVUPD xmm1, QWORD ptr [rdx] ; load n[0] and n[1]
    mulpd  xmm0, xmm1 ; multiply m[0]*n[0] and m[1]*n[1]

    haddpd xmm0, xmm0 ; horizontal add to get m[0]*n[0] + m[1]*n[1]

    MOVUPD xmm1, QWORD ptr [rcx + 16] ; load m[2] and m[3]
    MOVUPD xmm2, QWORD ptr [rdx + 16] ; load n[2] and n[3]
    mulpd  xmm1, xmm2 ; multiply m[2]*n[2] and m[3]*n[3]

    haddpd xmm1, xmm1 ; horizontal add to get m[2]*n[2] + m[3]*n[3]

    addsd  xmm0, xmm1 ; add the two results
    ret
quantDotDoule_Optimized ENDP

; quatDotlong_Optimized
; ---------------------------------------------------------
; Calculates the sum of products:
;   m[0]*n[0] + m[1]*n[1] + m[2]*n[2] + m[3]*n[3]
; ---------------------------------------------------------
; Requirements:
;   - SSE4.1 (for PMULLD and PINSRD)
;   - SSSE3  (for PHADDD)
;   - The final 32-bit sum is returned in EAX
;   - Same overflow behavior as the original scalar IMUL
;     (no 64-bit intermediate accumulation)
; ---------------------------------------------------------

quantDotlong_Optimized PROC
;--- Load m into XMM0: (m0, m1, m2, m3) ---
    mov   eax, dword ptr [rcx]           ; m[0]
    movd  xmm0, eax            ; xmm0 = (m0,  0,   0,   0)
    mov   eax, dword ptr [rcx+4]         ; m[1]
    pinsrd xmm0, eax, 1        ; xmm0 = (m0,  m1,  0,   0)
    mov   eax, dword ptr [rcx+8]         ; m[2]
    pinsrd xmm0, eax, 2        ; xmm0 = (m0,  m1,  m2,  0)
    mov   eax, dword ptr [rcx+12]        ; m[3]
    pinsrd xmm0, eax, 3        ; xmm0 = (m0,  m1,  m2,  m3)
    

    ;--- Load n into XMM1: (n0, n1, n2, n3) ---
    mov   eax, dword ptr [rdx]           ; n[0]
    movd  xmm1, eax            ; xmm1 = (n0,  0,   0,   0)
    mov   eax, dword ptr [rdx+4]         ; n[1]
    pinsrd xmm1, eax, 1        ; xmm1 = (n0,  n1,  0,   0)
    mov   eax, dword ptr [rdx+8]         ; n[2]
    pinsrd xmm1, eax, 2        ; xmm1 = (n0,  n1,  n2,  0)
    mov   eax, dword ptr [rdx+12]        ; n[3]
    pinsrd xmm1, eax, 3        ; xmm1 = (n0,  n1,  n2,  n3)

    ;--- Pairwise multiply (SSE4.1: PMULLD) ---
    pmulld xmm0, xmm1          ; xmm0 = (m0*n0, m1*n1, m2*n2, m3*n3)

    ;--- Sum the 3 products horizontally (SSSE3: PHADDD) ---
    ; 1st PHADDD: (m0*n0 + m1*n1, m2*n2 + 0, ..., ...)
    phaddd xmm0, xmm0
    ; 2nd PHADDD: adds the two results -> final sum in the low 32 bits
    phaddd xmm0, xmm0

    ;--- Move final 32-bit sum to EAX ---
    movd  eax, xmm0
    ret
quantDotlong_Optimized ENDP
END