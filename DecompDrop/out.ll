; ModuleID = 'out.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu-elf"

; Function Attrs: noinline
define void @deregister_tm_clones() local_unnamed_addr #0 {
  ret void
}

; Function Attrs: noinline
define void @__do_global_dtors_aux() local_unnamed_addr #0 {
  %1 = load i8, i8* inttoptr (i64 6295616 to i8*)
  %2 = icmp eq i8 %1, 0
  %3 = zext i1 %2 to i8
  %4 = icmp eq i8 %3, 0
  br i1 %4, label %6, label %5

5:                                                ; preds = %0
  call void @deregister_tm_clones()
  br label %6

6:                                                ; preds = %5, %0
  ret void
}

attributes #0 = { noinline }

!llvm.ident = !{!0, !0, !0}
!llvm.module.flags = !{!1, !2, !3}
!llvm.dbg.cu = !{}

!0 = !{!"clang version 9.0.0 (https://github.com/microsoft/vcpkg.git ad2933e97e7f6d2e2bece2a7a372be7a6833f28c)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 2, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
