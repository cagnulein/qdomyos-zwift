How to use these patched libraries (Qt 6.10.0, Android arm64)

- FFmpeg (7.1.1, JNI+MediaCodec enabled)
  1) Backup your Qt libs: /Users/<you>/Qt/6.10.0/android_arm64_v8a/lib/
  2) Copy files from ffmpeg/arm64-v8a/ into that directory (overwrite libav* and libsw*)

- OpenSSL (openssl-3.6)
  - Copy libssl.so and libcrypto.so (openssl/arm64-v8a/) to the path used by your packaging step.
    With CMake/Qt6 you can deploy as android extra libs or place them where your build looks for them.

- 16KB page alignment (Google Play requirement)
  - Ensure your Android target link options include:
    -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384

- Verify alignment
  - https://github.com/hotbrainstech/verify-16k-page-align
  - ./verify-16k-page-align.sh YourApp.apk

Include paths
- FFmpeg headers: qt-patches/android/6.10.0/ffmpeg/include
  Add to your compiler include path if you compile sources using FFmpeg APIs.
- OpenSSL headers: qt-patches/android/6.10.0/openssl/include
  Add to your compiler include path if you build against OpenSSL APIs.
