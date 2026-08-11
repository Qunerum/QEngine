bits 64
default rel
; LERP
global qLerp_f
global qLerp_v2
global qLerp_v3
; = = = > CLAMPS

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
; = = = > CLAMP
; ...
