bits 64
default rel
; = = = > LERP
global qLerp_f
global qLerp_v2
global qLerp_v3
; = = = > ABS
global qAbs_v2
global qAbs_v3
; = = = > ROUNDING
global qFloor
global qCeil
; = = = > SQRT
global qSqrt
; = = = > NORMALIZE
global qNormalize_v2
global qNormalize_v3

section .rodata
	align 16
	abs_mask dd 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x00000000

section .text
; = = = > LERP
qLerp_f:
	subss xmm1, xmm0
	mulss xmm1, xmm2
	addss xmm0, xmm1
	ret
qLerp_v2:
	shufps xmm2, xmm2, 0x00
	subps xmm1, xmm0
	mulps xmm1, xmm2
	addps xmm0, xmm1
	ret
qLerp_v3:
	shufps  xmm4, xmm4, 0x00
	subps   xmm2, xmm0
	mulps   xmm2, xmm4
	addps   xmm0, xmm2
	subss   xmm3, xmm1
	mulss   xmm3, xmm4
	addss   xmm1, xmm3
	ret
; = = = > ABS
qAbs_v2:
	andps xmm0, [abs_mask]
	ret
qAbs_v3:
	andps xmm0, [abs_mask]
	ret
; = = = > ROUNDING
qFloor:
	roundss xmm0, xmm0, 1
	ret
qCeil:
	roundss xmm0, xmm0, 2
	ret
; = = = > SQRT
qSqrt:
	sqrtss xmm0, xmm0
	ret
; = = = > NORMALIZE
qNormalize_v2:
	movaps  xmm1, xmm0
	mulps   xmm1, xmm1
	movaps  xmm2, xmm1
	shufps  xmm2, xmm2, 0x01
	addss   xmm1, xmm2
	rsqrtss xmm1, xmm1
	shufps  xmm1, xmm1, 0x00
	mulps   xmm0, xmm1
	ret
qNormalize_v3:
	movaps  xmm2, xmm0
	mulps   xmm2, xmm2
	movaps  xmm3, xmm1
	mulss   xmm3, xmm3
	movaps  xmm4, xmm2
	shufps  xmm4, xmm4, 0x01
	addss   xmm2, xmm4
	addss   xmm2, xmm3
	rsqrtss xmm2, xmm2
	shufps  xmm2, xmm2, 0x00
	mulps   xmm0, xmm2
	mulss   xmm1, xmm2
	ret
