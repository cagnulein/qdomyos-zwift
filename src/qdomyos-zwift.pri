include(../defaults.pri)
QT += bluetooth widgets xml positioning quick networkauth websockets texttospeech location multimedia
QTPLUGIN += qavfmediaplayer
QT+= charts

qtHaveModule(httpserver) {
    QT += httpserver
    DEFINES += Q_HTTPSERVER
    SOURCES += webserverinfosender.cpp
    HEADERS += webserverinfosender.h

    # android and iOS are using ChartJS
    unix:android: {
        QT+= webview
        DEFINES += CHARTJS
    }
    ios: {
        QT+= webview
        DEFINES += CHARTJS
    }
#	 win32: {
#	     DEFINES += CHARTJS
#		}
}

CONFIG += c++17 console app_bundle optimize_full ltcg

CONFIG += qmltypes

#win32: CONFIG += webengine
#unix:!android: CONFIG += webengine

QML_IMPORT_NAME = org.cagnulein.qdomyoszwift
QML_IMPORT_MAJOR_VERSION = 1
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

win32:QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc
win32:QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc

QMAKE_LFLAGS_RELEASE += -s
QMAKE_CXXFLAGS += -fno-sized-deallocation
unix:android: {
    CONFIG -= optimize_size
    QMAKE_CFLAGS_OPTIMIZE_FULL -= -Oz
    QMAKE_CFLAGS_OPTIMIZE_FULL += -O3
}
macx: CONFIG += debug
win32: CONFIG += debug
macx: CONFIG += static
macx {
    QMAKE_INFO_PLIST = macx/Info.plist
}
INCLUDEPATH += qmdnsengine/src/include

android {

   #QMAKE_LINK += -nostdlib++

   equals(ANDROID_TARGET_ARCH,arm64-v8a) {
	   LIBS += -L$$PWD/opencv2/staticlibs/arm64-v8a/ -lopencv_core
		ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/arm64-v8a/libopencv_core.a
		LIBS += -L$$PWD/opencv2/staticlibs/arm64-v8a/ -lopencv_features2d
		ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/arm64-v8a/libopencv_features2d
		LIBS += -L$$PWD/opencv2/staticlibs/arm64-v8a/ -lopencv_highgui
		ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/arm64-v8a/libopencv_highgui.a
		LIBS += -L$$PWD/opencv2/staticlibs/arm64-v8a/ -lopencv_imgproc
		ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/arm64-v8a/libopencv_imgproc.a
		LIBS += -L$$PWD/opencv2/staticlibs/arm64-v8a/ -lopencv_photo
		ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/arm64-v8a/libopencv_photo.a
	}
	equals(ANDROID_TARGET_ARCH,armeabi-v7a) {
	LIBS += -L$$PWD/opencv2/staticlibs/armeabi-v7a/ -lopencv_core
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/armeabi-v7a/libopencv_core.a
	LIBS += -L$$PWD/opencv2/staticlibs/armeabi-v7a/ -lopencv_features2d
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/armeabi-v7a/libopencv_features2d
	LIBS += -L$$PWD/opencv2/staticlibs/armeabi-v7a/ -lopencv_highgui
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/armeabi-v7a/libopencv_highgui.a
	LIBS += -L$$PWD/opencv2/staticlibs/armeabi-v7a/ -lopencv_imgproc
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/armeabi-v7a/libopencv_imgproc.a
	LIBS += -L$$PWD/opencv2/staticlibs/armeabi-v7a/ -lopencv_photo
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/armeabi-v7a/libopencv_photo.a
	}
	equals(ANDROID_TARGET_ARCH,x86) {
	LIBS += -L$$PWD/opencv2/staticlibs/x86/ -lopencv_core
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86/libopencv_core.a
	LIBS += -L$$PWD/opencv2/staticlibs/x86/ -lopencv_features2d
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86/libopencv_features2d
	LIBS += -L$$PWD/opencv2/staticlibs/x86/ -lopencv_highgui
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86/libopencv_highgui.a
	LIBS += -L$$PWD/opencv2/staticlibs/x86/ -lopencv_imgproc
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86/libopencv_imgproc.a
	LIBS += -L$$PWD/opencv2/staticlibs/x86/ -lopencv_photo
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86/libopencv_photo.a
	}
	equals(ANDROID_TARGET_ARCH,x86_64) {
	LIBS += -L$$PWD/opencv2/staticlibs/x86_64/ -lopencv_core
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86_64/libopencv_core.a
	LIBS += -L$$PWD/opencv2/staticlibs/x86_64/ -lopencv_features2d
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86_64/libopencv_features2d
	LIBS += -L$$PWD/opencv2/staticlibs/x86_64/ -lopencv_highgui
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86_64/libopencv_highgui.a
	LIBS += -L$$PWD/opencv2/staticlibs/x86_64/ -lopencv_imgproc
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86_64/libopencv_imgproc.a
	LIBS += -L$$PWD/opencv2/staticlibs/x86_64/ -lopencv_photo
	ANDROID_EXTRA_LIBS += $$PWD/opencv2/staticlibs/x86_64/libopencv_photo.a
	}
}


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS IO_UNDER_QT SMTP_BUILD


# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# include(../qtzeroconf/qtzeroconf.pri)

SOURCES += \
   $$PWD/bkoolbike.cpp \
   $$PWD/csafe.cpp \
   $$PWD/csaferower.cpp \
   $$PWD/fakerower.cpp \
    $$PWD/virtualdevice.cpp \
    $$PWD/androidactivityresultreceiver.cpp \
    $$PWD/androidadblog.cpp \
   $$PWD/apexbike.cpp \
    $$PWD/handleurl.cpp \
   $$PWD/iconceptelliptical.cpp \
    $$PWD/localipaddress.cpp \
   $$PWD/pelotonbike.cpp \
   $$PWD/schwinn170bike.cpp \
   $$PWD/wahookickrheadwind.cpp \
   $$PWD/windows_zwift_workout_paddleocr_thread.cpp \
   $$PWD/ypooelliptical.cpp \
   $$PWD/ziprotreadmill.cpp \
   Computrainer.cpp \
   PathController.cpp \
    characteristicnotifier2a53.cpp \
    characteristicnotifier2a5b.cpp \
    characteristicnotifier2acc.cpp \
    characteristicnotifier2acd.cpp \
    characteristicnotifier2ad9.cpp \
   characteristicwriteprocessor.cpp \
   characteristicwriteprocessore005.cpp \
   computrainerbike.cpp \
    fakeelliptical.cpp \
   faketreadmill.cpp \
   lifefitnesstreadmill.cpp \
   mepanelbike.cpp \
   nautilusbike.cpp \
    nordictrackelliptical.cpp \
    nordictrackifitadbbike.cpp \
   nordictrackifitadbtreadmill.cpp \
   octaneelliptical.cpp \
   octanetreadmill.cpp \
   proformellipticaltrainer.cpp \
   proformrower.cpp \
   proformwifibike.cpp \
   proformwifitreadmill.cpp \
    qmdnsengine/src/src/abstractserver.cpp \
    qmdnsengine/src/src/bitmap.cpp \
    qmdnsengine/src/src/browser.cpp \
    qmdnsengine/src/src/cache.cpp \
    qmdnsengine/src/src/dns.cpp \
    qmdnsengine/src/src/hostname.cpp \
    qmdnsengine/src/src/mdns.cpp \
    qmdnsengine/src/src/message.cpp \
    qmdnsengine/src/src/prober.cpp \
    qmdnsengine/src/src/provider.cpp \
    qmdnsengine/src/src/query.cpp \
    qmdnsengine/src/src/record.cpp \
    qmdnsengine/src/src/resolver.cpp \
    qmdnsengine/src/src/server.cpp \
    qmdnsengine/src/src/service.cpp \
    activiotreadmill.cpp \
   bhfitnesselliptical.cpp \
   bike.cpp \
	     bluetooth.cpp \
		bluetoothdevice.cpp \
    characteristicnotifier2a37.cpp \
    characteristicnotifier2a63.cpp \
    characteristicnotifier2ad2.cpp \
    characteristicwriteprocessor2ad9.cpp \
   bowflext216treadmill.cpp \
    bowflextreadmill.cpp \
   chronobike.cpp \
    concept2skierg.cpp \
   cscbike.cpp \
    dirconmanager.cpp \
    dirconpacket.cpp \
    dirconprocessor.cpp \
	 domyoselliptical.cpp \
   domyosrower.cpp \
	     domyostreadmill.cpp \
		echelonconnectsport.cpp \
   echelonrower.cpp \
   echelonstride.cpp \
   eliterizer.cpp \
   elitesterzosmart.cpp \
	 elliptical.cpp \
	eslinkertreadmill.cpp \
    fakebike.cpp \
   filedownloader.cpp \
    fitmetria_fanfit.cpp \
   fitplusbike.cpp \
	fitshowtreadmill.cpp \
	fit-sdk/fit.cpp \
	fit-sdk/fit_accumulated_field.cpp \
	fit-sdk/fit_accumulator.cpp \
	fit-sdk/fit_buffer_encode.cpp \
	fit-sdk/fit_buffered_mesg_broadcaster.cpp \
	fit-sdk/fit_buffered_record_mesg_broadcaster.cpp \
	fit-sdk/fit_crc.cpp \
	fit-sdk/fit_date_time.cpp \
	fit-sdk/fit_decode.cpp \
	fit-sdk/fit_developer_field.cpp \
	fit-sdk/fit_developer_field_definition.cpp \
	fit-sdk/fit_developer_field_description.cpp \
	fit-sdk/fit_encode.cpp \
	fit-sdk/fit_factory.cpp \
	fit-sdk/fit_field.cpp \
	fit-sdk/fit_field_base.cpp \
	fit-sdk/fit_field_definition.cpp \
	fit-sdk/fit_mesg.cpp \
	fit-sdk/fit_mesg_broadcaster.cpp \
	fit-sdk/fit_mesg_definition.cpp \
	fit-sdk/fit_mesg_with_event_broadcaster.cpp \
	fit-sdk/fit_profile.cpp \
	fit-sdk/fit_protocol_validator.cpp \
	fit-sdk/fit_unicode.cpp \
	flywheelbike.cpp \
	ftmsbike.cpp \
    ftmsrower.cpp \
	     gpx.cpp \
		heartratebelt.cpp \
   homefitnessbuddy.cpp \
	homeform.cpp \
    horizongr7bike.cpp \
   horizontreadmill.cpp \
   iconceptbike.cpp \
	inspirebike.cpp \
	keepawakehelper.cpp \
   keepbike.cpp \
   kingsmithr1protreadmill.cpp \
   kingsmithr2treadmill.cpp \
	     main.cpp \
   mcfbike.cpp \
		metric.cpp \
   nautiluselliptical.cpp \
    nautilustreadmill.cpp \
    npecablebike.cpp \
   pafersbike.cpp \
   paferstreadmill.cpp \
   peloton.cpp \
   powerzonepack.cpp \
	proformbike.cpp \
   proformelliptical.cpp \
	proformtreadmill.cpp \
	qfit.cpp \
    qzsettings.cpp \
   renphobike.cpp \
   rower.cpp \
	schwinnic4bike.cpp \
   screencapture.cpp \
	sessionline.cpp \
   shuaa5treadmill.cpp \
	signalhandler.cpp \
   simplecrypt.cpp \
    skandikawiribike.cpp \
   smartrowrower.cpp \
   smartspin2k.cpp \
    smtpclient/src/emailaddress.cpp \
    smtpclient/src/mimeattachment.cpp \
    smtpclient/src/mimecontentformatter.cpp \
    smtpclient/src/mimefile.cpp \
    smtpclient/src/mimehtml.cpp \
    smtpclient/src/mimeinlinefile.cpp \
    smtpclient/src/mimemessage.cpp \
    smtpclient/src/mimemultipart.cpp \
    smtpclient/src/mimepart.cpp \
    smtpclient/src/mimetext.cpp \
    smtpclient/src/quotedprintable.cpp \
    smtpclient/src/smtpclient.cpp \
   snodebike.cpp \
   solebike.cpp \
   soleelliptical.cpp \
   solef80treadmill.cpp \
   spirittreadmill.cpp \
   sportsplusbike.cpp \
   sportstechbike.cpp \
   strydrunpowersensor.cpp \
   tacxneo2.cpp \
    tcpclientinfosender.cpp \
   technogymmyruntreadmill.cpp \
    technogymmyruntreadmillrfcomm.cpp \
    templateinfosender.cpp \
    templateinfosenderbuilder.cpp \
   stagesbike.cpp \
	     toorxtreadmill.cpp \
		  treadmill.cpp \
   truetreadmill.cpp \
   trxappgateusbbike.cpp \
   ultrasportbike.cpp \
   virtualrower.cpp \
   wahookickrsnapbike.cpp \
		yesoulbike.cpp \
		  trainprogram.cpp \
		trxappgateusbtreadmill.cpp \
	 virtualbike.cpp \
	     virtualtreadmill.cpp \
             m3ibike.cpp \
                domyosbike.cpp \
               scanrecordresult.cpp \
					windows_zwift_incline_paddleocr_thread.cpp \
   zwiftworkout.cpp
macx: SOURCES += macos/lockscreen.mm
!ios: SOURCES += mainwindow.cpp charts.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += fit-sdk/

HEADERS += \
   $$PWD/bkoolbike.h \
   $$PWD/csafe.h \
   $$PWD/csaferower.h \
   $$PWD/opencv2/core.hpp \
   $$PWD/opencv2/core/affine.hpp \
   $$PWD/opencv2/core/async.hpp \
   $$PWD/opencv2/core/base.hpp \
   $$PWD/opencv2/core/bindings_utils.hpp \
   $$PWD/opencv2/core/bufferpool.hpp \
   $$PWD/opencv2/core/check.hpp \
   $$PWD/opencv2/core/core.hpp \
   $$PWD/opencv2/core/core_c.h \
   $$PWD/opencv2/core/cuda.hpp \
   $$PWD/opencv2/core/cuda.inl.hpp \
   $$PWD/opencv2/core/cuda/block.hpp \
   $$PWD/opencv2/core/cuda/border_interpolate.hpp \
   $$PWD/opencv2/core/cuda/color.hpp \
   $$PWD/opencv2/core/cuda/common.hpp \
   $$PWD/opencv2/core/cuda/datamov_utils.hpp \
   $$PWD/opencv2/core/cuda/detail/color_detail.hpp \
   $$PWD/opencv2/core/cuda/detail/reduce.hpp \
   $$PWD/opencv2/core/cuda/detail/reduce_key_val.hpp \
   $$PWD/opencv2/core/cuda/detail/transform_detail.hpp \
   $$PWD/opencv2/core/cuda/detail/type_traits_detail.hpp \
   $$PWD/opencv2/core/cuda/detail/vec_distance_detail.hpp \
   $$PWD/opencv2/core/cuda/dynamic_smem.hpp \
   $$PWD/opencv2/core/cuda/emulation.hpp \
   $$PWD/opencv2/core/cuda/filters.hpp \
   $$PWD/opencv2/core/cuda/funcattrib.hpp \
   $$PWD/opencv2/core/cuda/functional.hpp \
   $$PWD/opencv2/core/cuda/limits.hpp \
   $$PWD/opencv2/core/cuda/reduce.hpp \
   $$PWD/opencv2/core/cuda/saturate_cast.hpp \
   $$PWD/opencv2/core/cuda/scan.hpp \
   $$PWD/opencv2/core/cuda/simd_functions.hpp \
   $$PWD/opencv2/core/cuda/transform.hpp \
   $$PWD/opencv2/core/cuda/type_traits.hpp \
   $$PWD/opencv2/core/cuda/utility.hpp \
   $$PWD/opencv2/core/cuda/vec_distance.hpp \
   $$PWD/opencv2/core/cuda/vec_math.hpp \
   $$PWD/opencv2/core/cuda/vec_traits.hpp \
   $$PWD/opencv2/core/cuda/warp.hpp \
   $$PWD/opencv2/core/cuda/warp_reduce.hpp \
   $$PWD/opencv2/core/cuda/warp_shuffle.hpp \
   $$PWD/opencv2/core/cuda_stream_accessor.hpp \
   $$PWD/opencv2/core/cuda_types.hpp \
   $$PWD/opencv2/core/cv_cpu_dispatch.h \
   $$PWD/opencv2/core/cv_cpu_helper.h \
   $$PWD/opencv2/core/cvdef.h \
   $$PWD/opencv2/core/cvstd.hpp \
   $$PWD/opencv2/core/cvstd.inl.hpp \
   $$PWD/opencv2/core/cvstd_wrapper.hpp \
   $$PWD/opencv2/core/detail/async_promise.hpp \
   $$PWD/opencv2/core/detail/dispatch_helper.impl.hpp \
   $$PWD/opencv2/core/detail/exception_ptr.hpp \
   $$PWD/opencv2/core/directx.hpp \
   $$PWD/opencv2/core/dualquaternion.hpp \
   $$PWD/opencv2/core/dualquaternion.inl.hpp \
   $$PWD/opencv2/core/eigen.hpp \
   $$PWD/opencv2/core/fast_math.hpp \
   $$PWD/opencv2/core/hal/hal.hpp \
   $$PWD/opencv2/core/hal/interface.h \
   $$PWD/opencv2/core/hal/intrin.hpp \
   $$PWD/opencv2/core/hal/intrin_avx.hpp \
   $$PWD/opencv2/core/hal/intrin_avx512.hpp \
   $$PWD/opencv2/core/hal/intrin_cpp.hpp \
   $$PWD/opencv2/core/hal/intrin_forward.hpp \
   $$PWD/opencv2/core/hal/intrin_msa.hpp \
   $$PWD/opencv2/core/hal/intrin_neon.hpp \
   $$PWD/opencv2/core/hal/intrin_rvv.hpp \
   $$PWD/opencv2/core/hal/intrin_rvv071.hpp \
   $$PWD/opencv2/core/hal/intrin_sse.hpp \
   $$PWD/opencv2/core/hal/intrin_sse_em.hpp \
   $$PWD/opencv2/core/hal/intrin_vsx.hpp \
   $$PWD/opencv2/core/hal/intrin_wasm.hpp \
   $$PWD/opencv2/core/hal/msa_macros.h \
   $$PWD/opencv2/core/hal/simd_utils.impl.hpp \
   $$PWD/opencv2/core/mat.hpp \
   $$PWD/opencv2/core/mat.inl.hpp \
   $$PWD/opencv2/core/matx.hpp \
   $$PWD/opencv2/core/neon_utils.hpp \
   $$PWD/opencv2/core/ocl.hpp \
   $$PWD/opencv2/core/ocl_genbase.hpp \
   $$PWD/opencv2/core/opencl/ocl_defs.hpp \
   $$PWD/opencv2/core/opencl/opencl_info.hpp \
   $$PWD/opencv2/core/opencl/opencl_svm.hpp \
   $$PWD/opencv2/core/opencl/runtime/autogenerated/opencl_clblas.hpp \
   $$PWD/opencv2/core/opencl/runtime/autogenerated/opencl_clfft.hpp \
   $$PWD/opencv2/core/opencl/runtime/autogenerated/opencl_core.hpp \
   $$PWD/opencv2/core/opencl/runtime/autogenerated/opencl_core_wrappers.hpp \
   $$PWD/opencv2/core/opencl/runtime/autogenerated/opencl_gl.hpp \
   $$PWD/opencv2/core/opencl/runtime/autogenerated/opencl_gl_wrappers.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_clblas.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_clfft.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_core.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_core_wrappers.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_gl.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_gl_wrappers.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_svm_20.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_svm_definitions.hpp \
   $$PWD/opencv2/core/opencl/runtime/opencl_svm_hsa_extension.hpp \
   $$PWD/opencv2/core/opengl.hpp \
   $$PWD/opencv2/core/operations.hpp \
   $$PWD/opencv2/core/optim.hpp \
   $$PWD/opencv2/core/ovx.hpp \
   $$PWD/opencv2/core/parallel/backend/parallel_for.openmp.hpp \
   $$PWD/opencv2/core/parallel/backend/parallel_for.tbb.hpp \
   $$PWD/opencv2/core/parallel/parallel_backend.hpp \
   $$PWD/opencv2/core/persistence.hpp \
   $$PWD/opencv2/core/quaternion.hpp \
   $$PWD/opencv2/core/quaternion.inl.hpp \
   $$PWD/opencv2/core/saturate.hpp \
   $$PWD/opencv2/core/simd_intrinsics.hpp \
   $$PWD/opencv2/core/softfloat.hpp \
   $$PWD/opencv2/core/sse_utils.hpp \
   $$PWD/opencv2/core/traits.hpp \
   $$PWD/opencv2/core/types.hpp \
   $$PWD/opencv2/core/types_c.h \
   $$PWD/opencv2/core/utility.hpp \
   $$PWD/opencv2/core/utils/allocator_stats.hpp \
   $$PWD/opencv2/core/utils/allocator_stats.impl.hpp \
   $$PWD/opencv2/core/utils/filesystem.hpp \
   $$PWD/opencv2/core/utils/fp_control_utils.hpp \
   $$PWD/opencv2/core/utils/instrumentation.hpp \
   $$PWD/opencv2/core/utils/logger.defines.hpp \
   $$PWD/opencv2/core/utils/logger.hpp \
   $$PWD/opencv2/core/utils/logtag.hpp \
   $$PWD/opencv2/core/utils/tls.hpp \
   $$PWD/opencv2/core/utils/trace.hpp \
   $$PWD/opencv2/core/va_intel.hpp \
   $$PWD/opencv2/core/version.hpp \
   $$PWD/opencv2/core/vsx_utils.hpp \
   $$PWD/opencv2/cvconfig.h \
   $$PWD/opencv2/features2d.hpp \
   $$PWD/opencv2/features2d/features2d.hpp \
   $$PWD/opencv2/features2d/hal/interface.h \
   $$PWD/opencv2/highgui.hpp \
   $$PWD/opencv2/highgui/highgui.hpp \
   $$PWD/opencv2/imgproc.hpp \
   $$PWD/opencv2/imgproc/bindings.hpp \
   $$PWD/opencv2/imgproc/detail/gcgraph.hpp \
   $$PWD/opencv2/imgproc/hal/hal.hpp \
   $$PWD/opencv2/imgproc/hal/interface.h \
   $$PWD/opencv2/imgproc/imgproc.hpp \
   $$PWD/opencv2/imgproc/imgproc_c.h \
   $$PWD/opencv2/imgproc/segmentation.hpp \
   $$PWD/opencv2/imgproc/types_c.h \
   $$PWD/opencv2/opencv.hpp \
   $$PWD/opencv2/opencv_modules.hpp \
   $$PWD/opencv2/photo.hpp \
   $$PWD/opencv2/photo/cuda.hpp \
   $$PWD/opencv2/photo/legacy/constants_c.h \
   $$PWD/opencv2/photo/photo.hpp \
   $$PWD/opencv2/video.hpp \
   $$PWD/opencv2/video/background_segm.hpp \
   $$PWD/opencv2/video/detail/tracking.detail.hpp \
   $$PWD/opencv2/video/legacy/constants_c.h \
   $$PWD/opencv2/video/tracking.hpp \
   $$PWD/opencv2/video/video.hpp \
   $$PWD/windows_zwift_workout_paddleocr_thread.h \
   $$PWD/fakerower.h \
    virtualdevice.h \
    $$PWD/androidactivityresultreceiver.h \
    $$PWD/androidadblog.h \
   $$PWD/apexbike.h \
    $$PWD/discoveryoptions.h \
    $$PWD/handleurl.h \
   $$PWD/iconceptelliptical.h \
    $$PWD/localipaddress.h \
   $$PWD/pelotonbike.h \
   $$PWD/schwinn170bike.h \
   $$PWD/wahookickrheadwind.h \
   $$PWD/ypooelliptical.h \
   $$PWD/ziprotreadmill.h \
   Computrainer.h \
   PathController.h \
    characteristicnotifier2a53.h \
    characteristicnotifier2a5b.h \
    characteristicnotifier2acc.h \
    characteristicnotifier2acd.h \
    characteristicnotifier2ad9.h \
   characteristicwriteprocessore005.h \
   computrainerbike.h \
    definitions.h \
    fakeelliptical.h \
   faketreadmill.h \
   lifefitnesstreadmill.h \
   mepanelbike.h \
   nautilusbike.h \
    nordictrackelliptical.h \
    nordictrackifitadbbike.h \
   nordictrackifitadbtreadmill.h \
   octaneelliptical.h \
   octanetreadmill.h \
   proformellipticaltrainer.h \
   proformrower.h \
   proformwifibike.h \
   proformwifitreadmill.h \
    qmdnsengine/src/include/qmdnsengine/abstractserver.h \
    qmdnsengine/src/include/qmdnsengine/bitmap.h \
    qmdnsengine/src/include/qmdnsengine/browser.h \
    qmdnsengine/src/include/qmdnsengine/cache.h \
    qmdnsengine/src/include/qmdnsengine/dns.h \
    qmdnsengine/src/include/qmdnsengine/hostname.h \
    qmdnsengine/src/include/qmdnsengine/mdns.h \
    qmdnsengine/src/include/qmdnsengine/message.h \
    qmdnsengine/src/include/qmdnsengine/prober.h \
    qmdnsengine/src/include/qmdnsengine/provider.h \
    qmdnsengine/src/include/qmdnsengine/query.h \
    qmdnsengine/src/include/qmdnsengine/record.h \
    qmdnsengine/src/include/qmdnsengine/resolver.h \
    qmdnsengine/src/include/qmdnsengine/server.h \
    qmdnsengine/src/include/qmdnsengine/service.h \
    qmdnsengine/src/src/bitmap_p.h \
    qmdnsengine/src/src/browser_p.h \
    qmdnsengine/src/src/cache_p.h \
    qmdnsengine/src/src/hostname_p.h \
    qmdnsengine/src/src/message_p.h \
    qmdnsengine/src/src/prober_p.h \
    qmdnsengine/src/src/provider_p.h \
    qmdnsengine/src/src/query_p.h \
    qmdnsengine/src/src/record_p.h \
    qmdnsengine/src/src/resolver_p.h \
    qmdnsengine/src/src/server_p.h \
    qmdnsengine/src/src/service_p.h \
    activiotreadmill.h \
   bhfitnesselliptical.h \
   bike.h \
	bluetooth.h \
	bluetoothdevice.h \
    characteristicnotifier.h \
    characteristicnotifier2a37.h \
    characteristicnotifier2a63.h \
    characteristicnotifier2ad2.h \
    characteristicwriteprocessor.h \
    characteristicwriteprocessor2ad9.h \
   bowflext216treadmill.h \
    bowflextreadmill.h \
   chronobike.h \
    concept2skierg.h \
   cscbike.h \
    dirconmanager.h \
    dirconpacket.h \
    dirconprocessor.h \
	 domyoselliptical.h \
   domyosrower.h \
	domyostreadmill.h \
	echelonconnectsport.h \
   echelonrower.h \
   echelonstride.h \
   eliterizer.h \
   elitesterzosmart.h \
	 elliptical.h \
   eslinkertreadmill.h \
    fakebike.h \
   filedownloader.h \
    fitmetria_fanfit.h \
   fitplusbike.h \
    ftmsrower.h \
   homefitnessbuddy.h \
    horizongr7bike.h \
   iconceptbike.h \
   keepbike.h \
   kingsmithr1protreadmill.h \
   kingsmithr2treadmill.h \
   m3ibike.h \
        fitshowtreadmill.h \
	fit-sdk/FitDecode.h \
	fit-sdk/FitDeveloperField.h \
	fit-sdk/FitEncode.h \
	fit-sdk/FitField.h \
	fit-sdk/FitFieldDefinition.h \
	fit-sdk/FitMesg.h \
	fit-sdk/FitMesgDefinition.h \
	fit-sdk/fit.hpp \
	fit-sdk/fit_accelerometer_data_mesg.hpp \
	fit-sdk/fit_accelerometer_data_mesg_listener.hpp \
	fit-sdk/fit_accumulated_field.hpp \
	fit-sdk/fit_accumulator.hpp \
	fit-sdk/fit_activity_mesg.hpp \
	fit-sdk/fit_activity_mesg_listener.hpp \
	fit-sdk/fit_ant_channel_id_mesg.hpp \
	fit-sdk/fit_ant_channel_id_mesg_listener.hpp \
	fit-sdk/fit_ant_rx_mesg.hpp \
	fit-sdk/fit_ant_rx_mesg_listener.hpp \
	fit-sdk/fit_ant_tx_mesg.hpp \
	fit-sdk/fit_ant_tx_mesg_listener.hpp \
	fit-sdk/fit_aviation_attitude_mesg.hpp \
	fit-sdk/fit_aviation_attitude_mesg_listener.hpp \
	fit-sdk/fit_barometer_data_mesg.hpp \
	fit-sdk/fit_barometer_data_mesg_listener.hpp \
	fit-sdk/fit_bike_profile_mesg.hpp \
	fit-sdk/fit_bike_profile_mesg_listener.hpp \
	fit-sdk/fit_blood_pressure_mesg.hpp \
	fit-sdk/fit_blood_pressure_mesg_listener.hpp \
	fit-sdk/fit_buffer_encode.hpp \
	fit-sdk/fit_buffered_mesg_broadcaster.hpp \
	fit-sdk/fit_buffered_record_mesg.hpp \
	fit-sdk/fit_buffered_record_mesg_broadcaster.hpp \
	fit-sdk/fit_buffered_record_mesg_listener.hpp \
	fit-sdk/fit_cadence_zone_mesg.hpp \
	fit-sdk/fit_cadence_zone_mesg_listener.hpp \
	fit-sdk/fit_camera_event_mesg.hpp \
	fit-sdk/fit_camera_event_mesg_listener.hpp \
	fit-sdk/fit_capabilities_mesg.hpp \
	fit-sdk/fit_capabilities_mesg_listener.hpp \
	fit-sdk/fit_climb_pro_mesg.hpp \
	fit-sdk/fit_climb_pro_mesg_listener.hpp \
	fit-sdk/fit_config.hpp \
	fit-sdk/fit_connectivity_mesg.hpp \
	fit-sdk/fit_connectivity_mesg_listener.hpp \
	fit-sdk/fit_course_mesg.hpp \
	fit-sdk/fit_course_mesg_listener.hpp \
	fit-sdk/fit_course_point_mesg.hpp \
	fit-sdk/fit_course_point_mesg_listener.hpp \
	fit-sdk/fit_crc.hpp \
	fit-sdk/fit_date_time.hpp \
	fit-sdk/fit_decode.hpp \
	fit-sdk/fit_developer_data_id_mesg.hpp \
	fit-sdk/fit_developer_data_id_mesg_listener.hpp \
	fit-sdk/fit_developer_field.hpp \
	fit-sdk/fit_developer_field_definition.hpp \
	fit-sdk/fit_developer_field_description.hpp \
	fit-sdk/fit_developer_field_description_listener.hpp \
	fit-sdk/fit_device_info_mesg.hpp \
	fit-sdk/fit_device_info_mesg_listener.hpp \
	fit-sdk/fit_device_settings_mesg.hpp \
	fit-sdk/fit_device_settings_mesg_listener.hpp \
	fit-sdk/fit_dive_alarm_mesg.hpp \
	fit-sdk/fit_dive_alarm_mesg_listener.hpp \
	fit-sdk/fit_dive_gas_mesg.hpp \
	fit-sdk/fit_dive_gas_mesg_listener.hpp \
	fit-sdk/fit_dive_settings_mesg.hpp \
	fit-sdk/fit_dive_settings_mesg_listener.hpp \
	fit-sdk/fit_dive_summary_mesg.hpp \
	fit-sdk/fit_dive_summary_mesg_listener.hpp \
	fit-sdk/fit_encode.hpp \
	fit-sdk/fit_event_mesg.hpp \
	fit-sdk/fit_event_mesg_listener.hpp \
	fit-sdk/fit_exd_data_concept_configuration_mesg.hpp \
	fit-sdk/fit_exd_data_concept_configuration_mesg_listener.hpp \
	fit-sdk/fit_exd_data_field_configuration_mesg.hpp \
	fit-sdk/fit_exd_data_field_configuration_mesg_listener.hpp \
	fit-sdk/fit_exd_screen_configuration_mesg.hpp \
	fit-sdk/fit_exd_screen_configuration_mesg_listener.hpp \
	fit-sdk/fit_exercise_title_mesg.hpp \
	fit-sdk/fit_exercise_title_mesg_listener.hpp \
	fit-sdk/fit_factory.hpp \
	fit-sdk/fit_field.hpp \
	fit-sdk/fit_field_base.hpp \
	fit-sdk/fit_field_capabilities_mesg.hpp \
	fit-sdk/fit_field_capabilities_mesg_listener.hpp \
	fit-sdk/fit_field_definition.hpp \
	fit-sdk/fit_field_description_mesg.hpp \
	fit-sdk/fit_field_description_mesg_listener.hpp \
	fit-sdk/fit_file_capabilities_mesg.hpp \
	fit-sdk/fit_file_capabilities_mesg_listener.hpp \
	fit-sdk/fit_file_creator_mesg.hpp \
	fit-sdk/fit_file_creator_mesg_listener.hpp \
	fit-sdk/fit_file_id_mesg.hpp \
	fit-sdk/fit_file_id_mesg_listener.hpp \
	fit-sdk/fit_goal_mesg.hpp \
	fit-sdk/fit_goal_mesg_listener.hpp \
	fit-sdk/fit_gps_metadata_mesg.hpp \
	fit-sdk/fit_gps_metadata_mesg_listener.hpp \
	fit-sdk/fit_gyroscope_data_mesg.hpp \
	fit-sdk/fit_gyroscope_data_mesg_listener.hpp \
	fit-sdk/fit_hr_mesg.hpp \
	fit-sdk/fit_hr_mesg_listener.hpp \
	fit-sdk/fit_hr_zone_mesg.hpp \
	fit-sdk/fit_hr_zone_mesg_listener.hpp \
	fit-sdk/fit_hrm_profile_mesg.hpp \
	fit-sdk/fit_hrm_profile_mesg_listener.hpp \
	fit-sdk/fit_hrv_mesg.hpp \
	fit-sdk/fit_hrv_mesg_listener.hpp \
	fit-sdk/fit_jump_mesg.hpp \
	fit-sdk/fit_jump_mesg_listener.hpp \
	fit-sdk/fit_lap_mesg.hpp \
	fit-sdk/fit_lap_mesg_listener.hpp \
	fit-sdk/fit_length_mesg.hpp \
	fit-sdk/fit_length_mesg_listener.hpp \
	fit-sdk/fit_magnetometer_data_mesg.hpp \
	fit-sdk/fit_magnetometer_data_mesg_listener.hpp \
	fit-sdk/fit_memo_glob_mesg.hpp \
	fit-sdk/fit_memo_glob_mesg_listener.hpp \
	fit-sdk/fit_mesg.hpp \
	fit-sdk/fit_mesg_broadcast_plugin.hpp \
	fit-sdk/fit_mesg_broadcaster.hpp \
	fit-sdk/fit_mesg_capabilities_mesg.hpp \
	fit-sdk/fit_mesg_capabilities_mesg_listener.hpp \
	fit-sdk/fit_mesg_definition.hpp \
	fit-sdk/fit_mesg_definition_listener.hpp \
	fit-sdk/fit_mesg_listener.hpp \
	fit-sdk/fit_mesg_with_event.hpp \
	fit-sdk/fit_mesg_with_event_broadcaster.hpp \
	fit-sdk/fit_mesg_with_event_listener.hpp \
	fit-sdk/fit_met_zone_mesg.hpp \
	fit-sdk/fit_met_zone_mesg_listener.hpp \
	fit-sdk/fit_monitoring_info_mesg.hpp \
	fit-sdk/fit_monitoring_info_mesg_listener.hpp \
	fit-sdk/fit_monitoring_mesg.hpp \
	fit-sdk/fit_monitoring_mesg_listener.hpp \
	fit-sdk/fit_nmea_sentence_mesg.hpp \
	fit-sdk/fit_nmea_sentence_mesg_listener.hpp \
	fit-sdk/fit_obdii_data_mesg.hpp \
	fit-sdk/fit_obdii_data_mesg_listener.hpp \
	fit-sdk/fit_ohr_settings_mesg.hpp \
	fit-sdk/fit_ohr_settings_mesg_listener.hpp \
	fit-sdk/fit_one_d_sensor_calibration_mesg.hpp \
	fit-sdk/fit_one_d_sensor_calibration_mesg_listener.hpp \
	fit-sdk/fit_pad_mesg.hpp \
	fit-sdk/fit_pad_mesg_listener.hpp \
	fit-sdk/fit_power_zone_mesg.hpp \
	fit-sdk/fit_power_zone_mesg_listener.hpp \
	fit-sdk/fit_profile.hpp \
	fit-sdk/fit_protocol_validator.hpp \
	fit-sdk/fit_record_mesg.hpp \
	fit-sdk/fit_record_mesg_listener.hpp \
	fit-sdk/fit_runtime_exception.hpp \
	fit-sdk/fit_schedule_mesg.hpp \
	fit-sdk/fit_schedule_mesg_listener.hpp \
	fit-sdk/fit_sdm_profile_mesg.hpp \
	fit-sdk/fit_sdm_profile_mesg_listener.hpp \
	fit-sdk/fit_segment_file_mesg.hpp \
	fit-sdk/fit_segment_file_mesg_listener.hpp \
	fit-sdk/fit_segment_id_mesg.hpp \
	fit-sdk/fit_segment_id_mesg_listener.hpp \
	fit-sdk/fit_segment_lap_mesg.hpp \
	fit-sdk/fit_segment_lap_mesg_listener.hpp \
	fit-sdk/fit_segment_leaderboard_entry_mesg.hpp \
	fit-sdk/fit_segment_leaderboard_entry_mesg_listener.hpp \
	fit-sdk/fit_segment_point_mesg.hpp \
	fit-sdk/fit_segment_point_mesg_listener.hpp \
	fit-sdk/fit_session_mesg.hpp \
	fit-sdk/fit_session_mesg_listener.hpp \
	fit-sdk/fit_set_mesg.hpp \
	fit-sdk/fit_set_mesg_listener.hpp \
	fit-sdk/fit_slave_device_mesg.hpp \
	fit-sdk/fit_slave_device_mesg_listener.hpp \
	fit-sdk/fit_software_mesg.hpp \
	fit-sdk/fit_software_mesg_listener.hpp \
	fit-sdk/fit_speed_zone_mesg.hpp \
	fit-sdk/fit_speed_zone_mesg_listener.hpp \
	fit-sdk/fit_sport_mesg.hpp \
	fit-sdk/fit_sport_mesg_listener.hpp \
	fit-sdk/fit_stress_level_mesg.hpp \
	fit-sdk/fit_stress_level_mesg_listener.hpp \
	fit-sdk/fit_three_d_sensor_calibration_mesg.hpp \
	fit-sdk/fit_three_d_sensor_calibration_mesg_listener.hpp \
	fit-sdk/fit_timestamp_correlation_mesg.hpp \
	fit-sdk/fit_timestamp_correlation_mesg_listener.hpp \
	fit-sdk/fit_totals_mesg.hpp \
	fit-sdk/fit_totals_mesg_listener.hpp \
	fit-sdk/fit_training_file_mesg.hpp \
	fit-sdk/fit_training_file_mesg_listener.hpp \
	fit-sdk/fit_unicode.hpp \
	fit-sdk/fit_user_profile_mesg.hpp \
	fit-sdk/fit_user_profile_mesg_listener.hpp \
	fit-sdk/fit_video_clip_mesg.hpp \
	fit-sdk/fit_video_clip_mesg_listener.hpp \
	fit-sdk/fit_video_description_mesg.hpp \
	fit-sdk/fit_video_description_mesg_listener.hpp \
	fit-sdk/fit_video_frame_mesg.hpp \
	fit-sdk/fit_video_frame_mesg_listener.hpp \
	fit-sdk/fit_video_mesg.hpp \
	fit-sdk/fit_video_mesg_listener.hpp \
	fit-sdk/fit_video_title_mesg.hpp \
	fit-sdk/fit_video_title_mesg_listener.hpp \
	fit-sdk/fit_watchface_settings_mesg.hpp \
	fit-sdk/fit_watchface_settings_mesg_listener.hpp \
	fit-sdk/fit_weather_alert_mesg.hpp \
	fit-sdk/fit_weather_alert_mesg_listener.hpp \
	fit-sdk/fit_weather_conditions_mesg.hpp \
	fit-sdk/fit_weather_conditions_mesg_listener.hpp \
	fit-sdk/fit_weight_scale_mesg.hpp \
	fit-sdk/fit_weight_scale_mesg_listener.hpp \
	fit-sdk/fit_workout_mesg.hpp \
	fit-sdk/fit_workout_mesg_listener.hpp \
	fit-sdk/fit_workout_session_mesg.hpp \
	fit-sdk/fit_workout_session_mesg_listener.hpp \
	fit-sdk/fit_workout_step_mesg.hpp \
	fit-sdk/fit_workout_step_mesg_listener.hpp \
	fit-sdk/fit_zones_target_mesg.hpp \
	fit-sdk/fit_zones_target_mesg_listener.hpp \
	flywheelbike.h \
	ftmsbike.h \
	 heartratebelt.h \
	homeform.h \
   horizontreadmill.h \
	inspirebike.h \
	ios/lockscreen.h \
	keepawakehelper.h \
	macos/lockscreen.h \
        ios/M3iIOS-Interface.h \
	material.h \
   mcfbike.h \
	metric.h \
   nautiluselliptical.h \
    nautilustreadmill.h \
    npecablebike.h \
   pafersbike.h \
   paferstreadmill.h \
   peloton.h \
   powerzonepack.h \
	proformbike.h \
   proformelliptical.h \
	proformtreadmill.h \
    qdebugfixup.h \
	qfit.h \
    qmdnsengine_export.h \
    qzsettings.h \
   renphobike.h \
   rower.h \
	schwinnic4bike.h \
   screencapture.h \
	sessionline.h \
   shuaa5treadmill.h \
	signalhandler.h \
   simplecrypt.h \
    skandikawiribike.h \
   smartrowrower.h \
   smartspin2k.h \
    smtpclient/src/SmtpMime \
    smtpclient/src/emailaddress.h \
    smtpclient/src/mimeattachment.h \
    smtpclient/src/mimecontentformatter.h \
    smtpclient/src/mimefile.h \
    smtpclient/src/mimehtml.h \
    smtpclient/src/mimeinlinefile.h \
    smtpclient/src/mimemessage.h \
    smtpclient/src/mimemultipart.h \
    smtpclient/src/mimepart.h \
    smtpclient/src/mimetext.h \
    smtpclient/src/quotedprintable.h \
    smtpclient/src/smtpclient.h \
    smtpclient/src/smtpexports.h \
   snodebike.h \
   solebike.h \
   soleelliptical.h \
   solef80treadmill.h \
   spirittreadmill.h \
   sportsplusbike.h \
   sportstechbike.h \
   strydrunpowersensor.h \
   tacxneo2.h \
    tcpclientinfosender.h \
   technogymmyruntreadmill.h \
    technogymmyruntreadmillrfcomm.h \
    templateinfosender.h \
    templateinfosenderbuilder.h \
   stagesbike.h \
	toorxtreadmill.h \
	gpx.h \
	treadmill.h \
	mainwindow.h \
	trainprogram.h \
   truetreadmill.h \
   trxappgateusbbike.h \
	trxappgateusbtreadmill.h \
   ultrasportbike.h \
	 virtualbike.h \
   virtualrower.h \
	virtualtreadmill.h \
	 domyosbike.h \
   wahookickrsnapbike.h \
   wobjectdefs.h \
   wobjectimpl.h \
        yesoulbike.h \
        scanrecordresult.h \
		  windows_zwift_incline_paddleocr_thread.h \
   zwiftworkout.h


exists(secret.h): HEADERS += secret.h

!ios: HEADERS += charts.h

!ios: FORMS += \
   charts.ui \
	mainwindow.ui

RESOURCES += \
   icons.qrc \
	qml.qrc

DISTFILES += \
    $$PWD/android/libs/android_antlib_4-16-0.aar \
    $$PWD/android/libs/connectiq-mobile-sdk-android-1.5.aar \
    $$PWD/android/res/xml/device_filter.xml \
   $$PWD/android/src/CSafeRowerUSBHID.java \
    $$PWD/android/src/Garmin.java \
   $$PWD/android/src/HidBridge.java \
    $$PWD/android/src/IQMessageReceiverWrapper.java \
    $$PWD/android/src/MediaProjection.java \
    $$PWD/android/src/NotificationUtils.java \
    $$PWD/android/src/ScreenCaptureService.java \
    .clang-format \
   AppxManifest.xml \
   android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
   android/res/layout/floating_layout.xml \
	android/res/values/libs.xml \
	android/src/Ant.java \
	android/src/ChannelService.java \
   android/src/FloatingHandler.java \
   android/src/FloatingWindowGFG.java \
   android/src/ForegroundService.java \
   android/src/NotificationClient.java \
   android/src/QZAdbRemote.java \
        android/src/ScanRecordResult.java \
        android/src/NativeScanCallback.java \
   android/src/HeartChannelController.java \
	android/src/MyActivity.java \
	android/src/PowerChannelController.java \
	android/src/SpeedChannelController.java \
    android/src/Usbserial.java \
   android/src/com/cgutman/adblib/AdbBase64.java \
   android/src/com/cgutman/adblib/AdbConnection.java \
   android/src/com/cgutman/adblib/AdbCrypto.java \
   android/src/com/cgutman/adblib/AdbProtocol.java \
   android/src/com/cgutman/adblib/AdbStream.java \
   android/src/com/cgutman/adblib/package-info.java \
   android/src/com/cgutman/androidremotedebugger/AdbUtils.java \
   android/src/com/cgutman/androidremotedebugger/adblib/AndroidBase64.java \
   android/src/com/cgutman/androidremotedebugger/console/CommandHistory.java \
   android/src/com/cgutman/androidremotedebugger/console/ConsoleBuffer.java \
   android/src/com/cgutman/androidremotedebugger/devconn/DeviceConnection.java \
   android/src/com/cgutman/androidremotedebugger/devconn/DeviceConnectionListener.java \
   android/src/com/cgutman/androidremotedebugger/service/ShellListener.java \
   android/src/com/cgutman/androidremotedebugger/service/ShellService.java \
   android/src/com/cgutman/androidremotedebugger/ui/Dialog.java \
   android/src/com/cgutman/androidremotedebugger/ui/SpinnerDialog.java \
	android/src/com/dsi/ant/channel/PredefinedNetwork.java \
    android/gradle.properties \
	android/src/org/qtproject/qt/android/purchasing/Security.java \
	android/src/org/qtproject/qt/android/purchasing/InAppPurchase.java \
	android/src/org/qtproject/qt/android/purchasing/Base64.java \
	android/src/org/qtproject/qt/android/purchasing/Base64DecoderException.java \
	ios/AppDelegate.swift \
	ios/BLEPeripheralManager.swift

win32: DISTFILES += \
   $$PWD/adb/AdbWinApi.dll \
	$$PWD/adb/AdbWinUsbApi.dll \
	$$PWD/adb/adb.exe \


ios {
    ios_icon.files = $$files($$PWD/icons/ios/*.png)
	 QMAKE_BUNDLE_DATA += ios_icon
}

ios {
    OBJECTIVE_SOURCES += ios/lockscreen.mm \
    ios/ios_app_delegate.mm \
	 fit-sdk/FitDecode.mm \
	 fit-sdk/FitDeveloperField.mm \
	 fit-sdk/FitEncode.mm \
	 fit-sdk/FitField.mm \
	 fit-sdk/FitFieldDefinition.mm \
	 fit-sdk/FitMesg.mm \
         fit-sdk/FitMesgDefinition.mm \
         ios/M3iNS.mm \

    SOURCES += ios/M3iNSQT.cpp

    OBJECTIVE_HEADERS += ios/M3iNS.h

    QMAKE_INFO_PLIST = ios/Info.plist
	 QMAKE_ASSET_CATALOGS = $$PWD/ios/Images.xcassets
	 QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
	 QMAKE_ASSET_CATALOGS_BUILD_PATH = $$PWD/ios/ 

    TARGET = qdomyoszwift
	 QMAKE_TARGET_BUNDLE_PREFIX = org.cagnulein
    DEFINES+=_Nullable_result=_Nullable NS_FORMAT_ARGUMENT\\(A\\)=
}

include($$PWD/purchasing/purchasing.pri)
INCLUDEPATH += purchasing/qmltypes
INCLUDEPATH += purchasing/inapp

WINRT_MANIFEST = AppxManifest.xml

VERSION = 2.13.98
