; RUN: llc -mtriple=x86_64-unknown-unknown -mattr=+sse2 < %s | FileCheck %s -check-prefix=SSE2
; RUN: llc -mtriple=x86_64-unknown-unknown -mattr=+avx < %s | FileCheck %s -check-prefix=AVX

define <8 x i16> @test1(<8 x i16> %A, <8 x i16> %B) {
; SSE2-LABEL: test1:
; SSE2:       # BB#0
; SSE2-NEXT:    movd  %xmm1, %eax
; SSE2-NEXT:    movzwl  %ax, %eax
; SSE2-NEXT:    movd  %eax, %xmm1
; SSE2-NEXT:    psllw  %xmm1, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test1:
; AVX:       # BB#0
; AVX-NEXT:    vmovd  %xmm1, %eax
; AVX-NEXT:    movzwl  %ax, %eax
; AVX-NEXT:    vmovd  %eax, %xmm1
; AVX-NEXT:    vpsllw  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit14 = shufflevector <8 x i16> %B, <8 x i16> undef, <8 x i32> zeroinitializer
  %shl = shl <8 x i16> %A, %vecinit14
  ret <8 x i16> %shl
}

define <4 x i32> @test2(<4 x i32> %A, <4 x i32> %B) {
; SSE2-LABEL: test2:
; SSE2:       # BB#0
; SSE2-NEXT:    xorps  %xmm2, %xmm2
; SSE2-NEXT:    movss  %xmm1, %xmm2
; SSE2-NEXT:    pslld  %xmm2, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test2:
; AVX:       # BB#0
; AVX-NEXT:    vpxor  %xmm2, %xmm2
; AVX-NEXT:    vpblendw  {{.*#+}} xmm1 = xmm1[0,1],xmm2[2,3,4,5,6,7]
; AVX-NEXT:    vpslld  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit6 = shufflevector <4 x i32> %B, <4 x i32> undef, <4 x i32> zeroinitializer
  %shl = shl <4 x i32> %A, %vecinit6
  ret <4 x i32> %shl
}

define <2 x i64> @test3(<2 x i64> %A, <2 x i64> %B) {
; SSE2-LABEL: test3:
; SSE2:       # BB#0
; SSE2-NEXT:    psllq  %xmm1, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test3:
; AVX:       # BB#0
; AVX-NEXT:    vpsllq  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit2 = shufflevector <2 x i64> %B, <2 x i64> undef, <2 x i32> zeroinitializer
  %shl = shl <2 x i64> %A, %vecinit2
  ret <2 x i64> %shl
}

define <8 x i16> @test4(<8 x i16> %A, <8 x i16> %B) {
; SSE2-LABEL: test4:
; SSE2:       # BB#0
; SSE2-NEXT:    movd  %xmm1, %eax
; SSE2-NEXT:    movzwl  %ax, %eax
; SSE2-NEXT:    movd  %eax, %xmm1
; SSE2-NEXT:    psrlw  %xmm1, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test4:
; AVX:       # BB#0
; AVX-NEXT:    vmovd  %xmm1, %eax
; AVX-NEXT:    movzwl  %ax, %eax
; AVX-NEXT:    vmovd  %eax, %xmm1
; AVX-NEXT:    vpsrlw  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit14 = shufflevector <8 x i16> %B, <8 x i16> undef, <8 x i32> zeroinitializer
  %shr = lshr <8 x i16> %A, %vecinit14
  ret <8 x i16> %shr
}

define <4 x i32> @test5(<4 x i32> %A, <4 x i32> %B) {
; SSE2-LABEL: test5:
; SSE2:       # BB#0
; SSE2-NEXT:    xorps  %xmm2, %xmm2
; SSE2-NEXT:    movss  %xmm1, %xmm2
; SSE2-NEXT:    psrld  %xmm2, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test5:
; AVX:       # BB#0
; AVX-NEXT:    vpxor  %xmm2, %xmm2
; AVX-NEXT:    vpblendw  {{.*#+}} xmm1 = xmm1[0,1],xmm2[2,3,4,5,6,7]
; AVX-NEXT:    vpsrld  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit6 = shufflevector <4 x i32> %B, <4 x i32> undef, <4 x i32> zeroinitializer
  %shr = lshr <4 x i32> %A, %vecinit6
  ret <4 x i32> %shr
}

define <2 x i64> @test6(<2 x i64> %A, <2 x i64> %B) {
; SSE2-LABEL: test6:
; SSE2:       # BB#0
; SSE2-NEXT:    psrlq  %xmm1, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test6:
; AVX:       # BB#0
; AVX-NEXT:    vpsrlq  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit2 = shufflevector <2 x i64> %B, <2 x i64> undef, <2 x i32> zeroinitializer
  %shr = lshr <2 x i64> %A, %vecinit2
  ret <2 x i64> %shr
}

define <8 x i16> @test7(<8 x i16> %A, <8 x i16> %B) {
; SSE2-LABEL: test7:
; SSE2:       # BB#0
; SSE2-NEXT:    movd  %xmm1, %eax
; SSE2-NEXT:    movzwl  %ax, %eax
; SSE2-NEXT:    movd  %eax, %xmm1
; SSE2-NEXT:    psraw  %xmm1, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test7:
; AVX:       # BB#0
; AVX-NEXT:    vmovd  %xmm1, %eax
; AVX-NEXT:    movzwl  %ax, %eax
; AVX-NEXT:    vmovd  %eax, %xmm1
; AVX-NEXT:    vpsraw  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit14 = shufflevector <8 x i16> %B, <8 x i16> undef, <8 x i32> zeroinitializer
  %shr = ashr <8 x i16> %A, %vecinit14
  ret <8 x i16> %shr
}

define <4 x i32> @test8(<4 x i32> %A, <4 x i32> %B) {
; SSE2-LABEL: test8:
; SSE2:       # BB#0
; SSE2-NEXT:    xorps  %xmm2, %xmm2
; SSE2-NEXT:    movss  %xmm1, %xmm2
; SSE2-NEXT:    psrad  %xmm2, %xmm0
; SSE2-NEXT:    retq
; AVX-LABEL: test8:
; AVX:       # BB#0
; AVX-NEXT:    vpxor  %xmm2, %xmm2
; AVX-NEXT:    vpblendw  {{.*#+}} xmm1 = xmm1[0,1],xmm2[2,3,4,5,6,7]
; AVX-NEXT:    vpsrad  %xmm1, %xmm0, %xmm0
; AVX-NEXT:    retq
entry:
  %vecinit6 = shufflevector <4 x i32> %B, <4 x i32> undef, <4 x i32> zeroinitializer
  %shr = ashr <4 x i32> %A, %vecinit6
  ret <4 x i32> %shr
}
