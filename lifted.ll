; ModuleID = '/Users/pag/Build/remill-1000/lib/Arch/X86/Runtime/amd64.bc'
source_filename = "llvm-link"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx-macho"

%struct.State = type { %struct.ArchState, [32 x %union.VectorReg], %struct.ArithFlags, %union.anon, %struct.Segments, %struct.AddressSpace, %struct.GPR, %struct.X87Stack, %struct.MMX, %struct.FPUStatusFlags, %union.anon, %union.FPU, %struct.SegmentCaches }
%struct.ArchState = type { i32, i32, %union.anon }
%union.VectorReg = type { %union.vec512_t }
%union.vec512_t = type { %struct.uint64v8_t }
%struct.uint64v8_t = type { [8 x i64] }
%struct.ArithFlags = type { i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8 }
%struct.Segments = type { i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector }
%union.SegmentSelector = type { i16 }
%struct.AddressSpace = type { i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg }
%struct.Reg = type { %union.anon }
%struct.GPR = type { i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg }
%struct.X87Stack = type { [8 x %struct.anon.3] }
%struct.anon.3 = type { i64, double }
%struct.MMX = type { [8 x %struct.anon.4] }
%struct.anon.4 = type { i64, %union.vec64_t }
%union.vec64_t = type { %struct.uint64v1_t }
%struct.uint64v1_t = type { [1 x i64] }
%struct.FPUStatusFlags = type { i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, [4 x i8] }
%union.anon = type { i64 }
%union.FPU = type { %struct.anon.13 }
%struct.anon.13 = type { %struct.FpuFXSAVE, [96 x i8] }
%struct.FpuFXSAVE = type { %union.SegmentSelector, %union.SegmentSelector, %union.FPUAbridgedTagWord, i8, i16, i32, %union.SegmentSelector, i16, i32, %union.SegmentSelector, i16, %union.FPUControlStatus, %union.FPUControlStatus, [8 x %struct.FPUStackElem], [16 x %union.vec128_t] }
%union.FPUAbridgedTagWord = type { i8 }
%union.FPUControlStatus = type { i32 }
%struct.FPUStackElem = type { %union.anon.11, [6 x i8] }
%union.anon.11 = type { %struct.float80_t }
%struct.float80_t = type { [10 x i8] }
%union.vec128_t = type { %struct.uint128v1_t }
%struct.uint128v1_t = type { [1 x i128] }
%struct.SegmentCaches = type { %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow }
%struct.SegmentShadow = type { %union.anon, i32, i32 }
%struct.Memory = type opaque
%anvill.struct.0 = type { i8 }

; Function Attrs: noduplicate noinline nounwind optnone
declare !remill.function.type !6 %struct.Memory* @__remill_error(%struct.State* dereferenceable(3376), i64, %struct.Memory*) local_unnamed_addr #0

; Function Attrs: noinline
define x86_64_sysvcc i8* @___cxa_allocate_exception(i64 %thrown_size) local_unnamed_addr #1 {
  %1 = call i64 asm sideeffect "# read register RAX", "=r"()
  %2 = call %struct.Memory* @__remill_error(%struct.State* undef, i64 4296307732, %struct.Memory* null)
  %3 = inttoptr i64 %1 to i8*
  ret i8* %3
}

; Function Attrs: noinline
define x86_64_sysvcc void @__Unwind_Resume(%anvill.struct.0* %exception_object) local_unnamed_addr #1 {
  %1 = call %struct.Memory* @__remill_error(%struct.State* undef, i64 4296307624, %struct.Memory* null)
  ret void
}

; Function Attrs: noinline
define x86_64_sysvcc void @___cxa_free_exception(i8* %0) local_unnamed_addr #1 {
  %2 = call %struct.Memory* @__remill_error(%struct.State* undef, i64 4296307768, %struct.Memory* null)
  ret void
}

; Function Attrs: noinline noreturn
define x86_64_sysvcc void @___cxa_throw(i8* %0, i8* %lptinfo, void (i8*)* %1) local_unnamed_addr #2 {
  %3 = call %struct.Memory* @__remill_error(%struct.State* undef, i64 4296307804, %struct.Memory* null)
  ret void
}

; Function Attrs: noinline
define x86_64_sysvcc i64 @__ZNSt12length_errorC1EPKc(i8* %this, i8* %0) local_unnamed_addr #1 {
  %2 = call i64 asm sideeffect "# read register RAX", "=r"()
  %3 = call %struct.Memory* @__remill_error(%struct.State* undef, i64 4294983920, %struct.Memory* null)
  ret i64 %2
}

; Function Attrs: noinline noreturn
define x86_64_sysvcc void @__ZNSt3__1L20__throw_length_errorEPKc(i8* %this, i8* %0) local_unnamed_addr #2 {
  %2 = call i8* @___cxa_allocate_exception(i64 16)
  %3 = call i64 @__ZNSt12length_errorC1EPKc(i8* %2, i8* %0)
  %4 = load i64, i64* inttoptr (i64 4297897967 to i64*)
  %5 = load i64, i64* inttoptr (i64 4297897935 to i64*)
  %6 = inttoptr i64 %4 to i8*
  %7 = inttoptr i64 %5 to void (i8*)*
  call void @___cxa_throw(i8* %2, i8* %6, void (i8*)* %7)
  unreachable
}

attributes #0 = { noduplicate noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-builtins" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline }
attributes #2 = { noinline noreturn }

!llvm.ident = !{!0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!llvm.dbg.cu = !{}

!0 = !{!"clang version 10.0.0 "}
!1 = !{i32 2, !"SDK Version", [3 x i32] [i32 10, i32 15, i32 6]}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{i32 7, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"base.helper.semantics"}
