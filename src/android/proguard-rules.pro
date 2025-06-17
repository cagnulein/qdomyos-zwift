# Regole ProGuard per gestire conflitti protobuf e gRPC

# Mantieni tutte le classi protobuf (rimuove duplicati automaticamente)
-keep class com.google.protobuf.** { *; }
-dontwarn com.google.protobuf.**

# Mantieni tutte le classi gRPC
-keep class io.grpc.** { *; }
-dontwarn io.grpc.**

# Risolvi specificamente il conflitto Any$1
-keep class com.google.protobuf.Any$1 { *; }
-keep class com.google.protobuf.Any { *; }

# Evita warning su classi duplicate durante la minification
-dontnote com.google.protobuf.**
-dontwarn com.google.protobuf.**

# Mantieni annotazioni necessarie per protobuf
-keepattributes Signature
-keepattributes *Annotation*

# Qt specifico
-keep class org.qtproject.qt5.android.** { *; }
-dontwarn org.qtproject.qt5.android.**

# Rimuovi automaticamente classi duplicate durante shrinking
-optimizations !code/simplification/arithmetic,!field/*,!class/merging/*