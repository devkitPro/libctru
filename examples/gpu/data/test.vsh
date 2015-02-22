; setup constants
	.const c20, 1.0, 0.0, 0.5, 1.0
 
; setup outmap
	.out o0, result.position, 0x0
	.out o1, result.color, 0x0
	.out o2, result.texcoord0, 0x0
	.out o3, result.texcoord1, 0x0
	.out o4, result.texcoord2, 0x0
 
; setup uniform map (not required)
	.uniform c0, c3, projection
	.uniform c4, c7, modelview
	.uniform c8, c8, lightDirection
	.uniform c9, c9, lightAmbient
	
	.vsh vmain, end_vmain
 
;code
	vmain:
		mov r1, v0 (0x4)
		mov r1, c20 (0x3)
		; temp = modvMtx * in.pos
		dp4 r0, c4, r1 (0x0)
		dp4 r0, c5, r1 (0x1)
		dp4 r0, c6, r1 (0x2)
		mov r0, c20 (0x3)
		; result.pos = projMtx * temp
		dp4 o0, c0, r0 (0x0)
		dp4 o0, c1, r0 (0x1)
		dp4 o0, c2, r0 (0x2)
		dp4 o0, c3, r0 (0x3)
		; result.texcoord = in.texcoord
		mov o2, v1 (0x5)
		mov o3, c20 (0x7)
		mov o4, c20 (0x7)
		; result.color = crappy lighting
		dp3 r0, c8, v2 (0x4)
		max r0, c20, r0 (0x9)
		mul r0, c9, r0 (0x8)
		add o1, c9, r0 (0x4)
		mov o1, c20 (0x3)
		nop
		end
	end_vmain:
 
;operand descriptors
	.opdesc x___, xyzw, xyzw ; 0x0
	.opdesc _y__, xyzw, xyzw ; 0x1
	.opdesc __z_, xyzw, xyzw ; 0x2
	.opdesc ___w, xyzw, xyzw ; 0x3
	.opdesc xyz_, xyzw, xyzw ; 0x4
	.opdesc xyzw, xyzw, xyzw ; 0x5
	.opdesc x_zw, xyzw, xyzw ; 0x6
	.opdesc xyzw, yyyw, xyzw ; 0x7
	.opdesc xyz_, wwww, wwww ; 0x8
	.opdesc xyz_, yyyy, xyzw ; 0x9
