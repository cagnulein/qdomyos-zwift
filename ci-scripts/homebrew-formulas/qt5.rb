class Qt5 < Formula
  desc "Cross-platform application and UI framework"
  homepage "https://www.qt.io/"
  url "https://download.qt.io/official_releases/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz"
  mirror "https://mirrors.dotsrc.org/qtproject/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz"
  mirror "https://mirrors.ocf.berkeley.edu/qt/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz"
  sha256 "3a530d1b243b5dec00bc54937455471aaa3e56849d2593edb8ded07228202240"
  license all_of: ["GFDL-1.3-only", "GPL-2.0-only", "GPL-3.0-only", "LGPL-2.1-only", "LGPL-3.0-only"]

  head "https://code.qt.io/qt/qt5.git", branch: "dev", shallow: false

  livecheck do
    url "https://download.qt.io/official_releases/qt/5.15/"
    regex(%r{href=["']?v?(\d+(?:\.\d+)+)/?["' >]}i)
  end

  bottle do
    sha256 cellar: :any,                 arm64_monterey: "8c734e90fb331e80242652aa19e5e427b7119a73b9abf99f2e1f8576b2ad5c51"
    sha256 cellar: :any,                 arm64_big_sur:  "b23511e84ce7f3a2a3bf3d13eeb54b50b23c52b79b29ce31c6e4eb8ad1006eae"
    sha256 cellar: :any,                 monterey:       "1481de79fb599b77b7c71788a07e4b5894e03b8cc5509b2a30e4c3e1f5ca4bcb"
    sha256 cellar: :any,                 big_sur:        "1e2f35ffa5b10d5d81831f34b1a8ea3bbc9e7aab96e5a6dea5a433e3e9e7f6b0"
    sha256 cellar: :any,                 catalina:       "9d6ad925c80a6bd4c7f7b7a3c0b5b42c21999da7b5f5b7ad3b9d96b98fbe89b5"
    sha256 cellar: :any_skip_relocation, x86_64_linux:   "9c7f25a7c5c5b5e4b44e7bb7b0c49e7de9c7d89e9d3b3f7e7e0b6c9b0f3b6e8d"
  end

  depends_on "node" => :build
  depends_on "pkg-config" => :build
  depends_on "python@3.9" => :build

  depends_on "freetype"
  depends_on "glib"
  depends_on "jpeg-turbo"
  depends_on "libpng"
  depends_on "pcre2"

  uses_from_macos "gperf" => :build
  uses_from_macos "bison"
  uses_from_macos "flex"
  uses_from_macos "sqlite"

  on_linux do
    depends_on "alsa-lib"
    depends_on "at-spi2-core"
    depends_on "expat"
    depends_on "fontconfig"
    depends_on "gstreamer"
    depends_on "gst-plugins-base"
    depends_on "harfbuzz"
    depends_on "icu4c"
    depends_on "krb5"
    depends_on "libdrm"
    depends_on "libevent"
    depends_on "libice"
    depends_on "libsm"
    depends_on "libvpx"
    depends_on "libxcomposite"
    depends_on "libxkbcommon"
    depends_on "libxkbfile"
    depends_on "libxrandr"
    depends_on "libxtst"
    depends_on "little-cms2"
    depends_on "mesa"
    depends_on "minizip"
    depends_on "nss"
    depends_on "opus"
    depends_on "pulseaudio"
    depends_on "sdl2"
    depends_on "snappy"
    depends_on "systemd"
    depends_on "wayland"
    depends_on "webp"
    depends_on "xcb-util"
    depends_on "xcb-util-image"
    depends_on "xcb-util-keysyms"
    depends_on "xcb-util-renderutil"
    depends_on "xcb-util-wm"
    depends_on "zstd"
  end

  fails_with gcc: "5"

  resource "qtwebengine" do
    url "https://code.qt.io/qt/qtwebengine.git",
        tag:      "v5.15.2-lts",
        revision: "d6041c6e9bf0b9e9395ce33b35e1c9f90b8eb2d5"

    # Add missing includes for newer Xcode
    # https://code.qt.io/cgit/qt/qtwebengine.git/commit/?id=96d4c79fe14b2b4b85b9b1b36b9b6b4c3e0ca9a0
    patch do
      url "https://raw.githubusercontent.com/Homebrew/formula-patches/7ae178a617d1e0eceb742557e63721af949bd28c/qt5/qtwebengine-xcode12.5.patch"
      sha256 "ac7bb0c1b8b6f29b3fb8218a4f91a9f4b3b6e3da6a9b4c5e1a8f3a5d4e0b2c3d"
    end
  end

  def install
    args = %W[
      -verbose
      -prefix #{prefix}
      -release
      -opensource -confirm-license
      -system-freetype
      -system-pcre
      -system-zlib
      -qt-libpng
      -qt-libjpeg
      -qt-sqlite
      -nomake examples
      -nomake tests
      -pkg-config
      -dbus-runtime
      -proprietary-codecs
    ]

    if OS.mac?
      args << "-no-rpath"
      args << "-system-png"
    else
      args << "-system-harfbuzz"
      args << "-system-sqlite"
      args << "-opengl es2"
      args << "-no-opengl"
      args << "-R#{lib}"
      # https://bugreports.qt.io/browse/QTBUG-71564
      args << "-no-avx2"
      args << "-no-avx512"
      args << "-no-feature-avx2"
      args << "-no-feature-avx512f"
    end

    # Disable QtWebEngine on Apple Silicon
    if Hardware::CPU.arm?
      args << "-skip" << "qtwebengine"
      args << "-skip" << "qtwebkit"
    end

    ENV.deparallelize
    system "./configure", *args
    system "make"
    ENV.deparallelize
    system "make", "install"

    # Some config scripts will only find Qt in a "Frameworks" folder
    frameworks.install_symlink Dir["#{lib}/*.framework"]

    # The pkg-config files installed suggest that headers can be found in the
    # `include` directory. Make this so by creating symlinks from `include` to
    # the Frameworks' Headers folders.
    Pathname.glob("#{lib}/*.framework/Headers") do |path|
      include.install_symlink path => path.parent.basename(".framework")
    end

    # Move `*.app` bundles into `libexec` to expose them to `brew linkapps` and
    # because we don't like having them in `bin`.
    # (Note: This move breaks invocation of Assistant via the Help menu
    # of both Designer and Linguist as that relies on Assistant being in `bin`.)
    libexec.mkpath
    Pathname.glob("#{bin}/*.app") { |app| mv app, libexec }
  end

  def caveats
    s = ""

    if Hardware::CPU.arm?
      s += <<~EOS
        This version of Qt on Apple Silicon does not include QtWebEngine.
      EOS
    end

    s
  end

  test do
    (testpath/"hello.pro").write <<~EOS
      QT += core
      QT -= gui
      TARGET = hello
      CONFIG += console
      CONFIG -= app_bundle
      SOURCES += main.cpp
    EOS

    (testpath/"main.cpp").write <<~EOS
      #include <QCoreApplication>
      #include <QDebug>

      int main(int argc, char *argv[])
      {
        QCoreApplication a(argc, argv);
        qDebug() << "Hello World!";
        return 0;
      }
    EOS

    system bin/"qmake", testpath/"hello.pro"
    system "make"
    assert_predicate testpath/"hello", :exist?

    assert_match "Hello World!", shell_output("./hello")
  end
end