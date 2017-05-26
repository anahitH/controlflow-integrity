; ModuleID = 'out.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i32 } { i8* bitcast (void ()* @_Z7readKeyv to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i32 0, i32 0), i32 17 }], section "llvm.metadata"
@.str.1 = private unnamed_addr constant [10 x i8] c"protected\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [9 x i8] c"test.cpp\00", section "llvm.metadata"
@.str = private unnamed_addr constant [14 x i8] c"Super Secret\0A\00", align 1

; Function Attrs: uwtable
define void @_Z7readKeyv() #0 {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0))
  call void @check()
  ret void
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: nounwind uwtable
define void @_Z7displayv() #2 {
entry:
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z4calcv() #2 {
entry:
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z6notifyv() #2 {
entry:
  ret void
}

; Function Attrs: uwtable
define void @_Z7encryptv() #3 {
entry:
  call void @_Z7readKeyv()
  ret void
}

; Function Attrs: uwtable
define void @_Z7decryptv() #3 {
entry:
  call void @_Z7readKeyv()
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z6loadUIv() #2 {
entry:
  call void @_Z7displayv()
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z10operation1v() #2 {
entry:
  call void @_Z4calcv()
  call void @_Z6notifyv()
  ret void
}

; Function Attrs: uwtable
define void @_Z10operation2v() #3 {
entry:
  call void @_Z7encryptv()
  call void @_Z7decryptv()
  ret void
}

; Function Attrs: norecurse uwtable
define i32 @main() #4 {
entry:
  call void @_Z6loadUIv()
  call void @_Z10operation1v()
  call void @_Z10operation2v()
  ret i32 0
}

; Function Attrs: nounwind uwtable
define void @check() #2 {
entry:
  %array = alloca [100 x i8*], align 16
  %size = alloca i64, align 8
  %arraydecay = getelementptr inbounds [100 x i8*], [100 x i8*]* %array, i32 0, i32 0
  %call = call i32 @backtrace(i8** %arraydecay, i32 100)
  %conv = sext i32 %call to i64
  store i64 %conv, i64* %size, align 8
  %arraydecay1 = getelementptr inbounds [100 x i8*], [100 x i8*]* %array, i32 0, i32 0
  %0 = load i64, i64* %size, align 8
  %conv2 = trunc i64 %0 to i32
  call void @backtrace_symbols_fd(i8** %arraydecay1, i32 %conv2, i32 1) #6
  ret void
}

declare i32 @backtrace(i8**, i32) #1

; Function Attrs: nounwind
declare void @backtrace_symbols_fd(i8**, i32, i32) #5

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "protected" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { norecurse uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind }

!llvm.ident = !{!0, !0}

!0 = !{!"clang version 3.9.0 (tags/RELEASE_390/final)"}
