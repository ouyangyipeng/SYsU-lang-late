; ModuleID = 'exam3.c'
source_filename = "exam3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@a = internal unnamed_addr global i32 0, align 4

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn
define dso_local i32 @foo(i32 noundef %0) local_unnamed_addr #0 {
  %2 = load i32, i32* @a, align 4, !tbaa !5
  %3 = add nsw i32 %2, %0
  store i32 %3, i32* @a, align 4, !tbaa !5
  ret i32 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn
define dso_local i32 @bar(i32 noundef %0) local_unnamed_addr #0 {
  %2 = load i32, i32* @a, align 4, !tbaa !5
  %3 = sub nsw i32 %2, %0
  store i32 %3, i32* @a, align 4, !tbaa !5
  ret i32 %3
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{!"Ubuntu clang version 14.0.0-1ubuntu1"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
