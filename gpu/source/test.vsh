; setup constants
	.const 5, 0.0, 1.0, 2.0, 3.0

; setup outmap
	.out o0, result.position
	.out o1, result.color
	.out o2, result.texcoord0
	.out o3, result.texcoord1
	.out o4, result.texcoord2

; setup uniform map (not required)
	.uniform 0x10, 0x13, mdlvMtx
	.uniform 0x14, 0x17, projMtx

;code
	main:
		; result.pos = mdlvMtx * in.pos
		dp4 d40, d40, d00 (0x0)
		dp4 d40, d41, d00 (0x1)
		dp4 d40, d42, d00 (0x2)
		mov d40, d25 (0x4)
		; result.pos = projMtx * in.pos
		dp4 d00, d44, d40 (0x0)
		dp4 d00, d45, d40 (0x1)
		dp4 d00, d46, d40 (0x2)
		dp4 d00, d47, d40 (0x3)
		; result.color = in.pos
		mov d04, d25 (0x5)
		; result.texcoord = const
		mov d08, d25 (0x5)
		mov d0C, d25 (0x5)
		mov d10, d25 (0x5)
		flush
		end
	endmain:

;operand descriptors
	.opdesc x___, xyzw, xyzw ; 0x0
	.opdesc _y__, xyzw, xyzw ; 0x1
	.opdesc __z_, xyzw, xyzw ; 0x2
	.opdesc ___w, xyzw, xyzw ; 0x3
	.opdesc ___w, yyyy, xyzw ; 0x4
	.opdesc xyzw, xyzw, xyzw ; 0x5
