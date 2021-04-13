; ModuleID = 'branch2.c'
source_filename = "branch2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@globalint = dso_local global i16 127, align 2
@globalint2 = dso_local global i16 0, align 2
@.str = private unnamed_addr constant [28 x i8] c"Global int is 16 bits wide\0A\00", align 1
@.str.1 = private unnamed_addr constant [32 x i8] c"Global int is not 16 bits wide\0A\00", align 1
@.str.2 = private unnamed_addr constant [29 x i8] c"Global int2 is 16 bits wide\0A\00", align 1
@.str.3 = private unnamed_addr constant [33 x i8] c"Global int2 is not 16 bits wide\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = load i16, i16* @globalint, align 2
  %3 = sext i16 %2 to i32
  %4 = add nsw i32 %3, 1
  %5 = trunc i32 %4 to i16
  store i16 %5, i16* @globalint, align 2
  %6 = load i16, i16* @globalint, align 2
  store i16 %6, i16* @globalint2, align 2
  %7 = load i16, i16* @globalint, align 2
  %8 = sext i16 %7 to i32
  %9 = icmp eq i32 %8, 128
  br i1 %9, label %10, label %12

10:                                               ; preds = %0
  %11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str, i64 0, i64 0))
  br label %19

12:                                               ; preds = %0
  %13 = load i16, i16* @globalint, align 2
  %14 = sext i16 %13 to i32
  %15 = icmp eq i32 %14, -128
  br i1 %15, label %16, label %18

16:                                               ; preds = %12
  %17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([32 x i8], [32 x i8]* @.str.1, i64 0, i64 0))
  br label %18

18:                                               ; preds = %16, %12
  br label %19

19:                                               ; preds = %18, %10
  %20 = load i16, i16* @globalint2, align 2
  %21 = sext i16 %20 to i32
  %22 = icmp eq i32 %21, 128
  br i1 %22, label %23, label %25

23:                                               ; preds = %19
  %24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @.str.2, i64 0, i64 0))
  br label %32

25:                                               ; preds = %19
  %26 = load i16, i16* @globalint2, align 2
  %27 = sext i16 %26 to i32
  %28 = icmp eq i32 %27, -128
  br i1 %28, label %29, label %31

29:                                               ; preds = %25
  %30 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.3, i64 0, i64 0))
  br label %31

31:                                               ; preds = %29, %25
  br label %32

32:                                               ; preds = %31, %23
  ret i32 0
}

declare dso_local i32 @printf(i8*, ...) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
