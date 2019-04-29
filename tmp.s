.intel_syntax noprefix
.global _main
_main:
  push 24
  push 3
  pop rdi
  pop rax
  mov rdx, 0
  div rdi
  push rax
  pop rax
  ret
