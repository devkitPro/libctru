; Really simple & stupid PICA200 shader
; Taken from picasso example

; Uniforms
.fvec projection[4]

; Constants
.constf myconst(0.0, 1.0, -1.0, 0.0)
.alias ones myconst.yyyy ; (1.0,1.0,1.0,1.0)

; Outputs : here only position and color
.out outpos position
.out outclr color

; Inputs : here we have only vertices
.alias inpos v0

.proc main
	; r0 = (inpos.x, inpos.y, inpos.z, 1.0)
	mov r0.xyz, inpos
	mov r0.w, ones
	
	; outpos = projection * r1
	dp4 outpos.x, projection[0], r0
	dp4 outpos.y, projection[1], r0
	dp4 outpos.z, projection[2], r0
	dp4 outpos.w, projection[3], r0
	
	; Set vertex color to white
	mov outclr, ones
	end
.end