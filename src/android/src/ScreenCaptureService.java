package org.cagnulen.qdomyoszwift;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PixelFormat;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.Image;
import android.media.ImageReader;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;
import android.view.Display;
import android.view.OrientationEventListener;
import android.view.WindowManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Objects;

import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.text.Text;
import com.google.mlkit.vision.text.TextRecognition;
import com.google.mlkit.vision.text.TextRecognizer;
import com.google.mlkit.vision.text.latin.TextRecognizerOptions;
import android.media.ImageReader.OnImageAvailableListener;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;

import android.graphics.Rect;
import android.graphics.Point;

import androidx.core.util.Pair;

import com.baidu.paddle.fastdeploy.LitePowerMode;
import com.equationl.fastdeployocr.OCR;
import com.equationl.fastdeployocr.OcrConfig;
import com.equationl.fastdeployocr.RunPrecision;
import com.equationl.fastdeployocr.RunType;
import com.equationl.fastdeployocr.bean.OcrResult;
import com.equationl.fastdeployocr.bean.OcrResultModel;
import com.equationl.fastdeployocr.callback.OcrInitCallback;
import com.equationl.fastdeployocr.callback.OcrRunCallback;

public class ScreenCaptureService extends Service {

    private static final String TAG = "ScreenCaptureService";
    private static final String RESULT_CODE = "RESULT_CODE";
    private static final String DATA = "DATA";
    private static final String ACTION = "ACTION";
    private static final String START = "START";
    private static final String STOP = "STOP";
    private static final String SCREENCAP_NAME = "screencap";

    private static int IMAGES_PRODUCED;

    private MediaProjection mMediaProjection;
    private String mStoreDir;
    private ImageReader mImageReader;
    private Handler mHandler;
    private Display mDisplay;
    private VirtualDisplay mVirtualDisplay;
    private int mDensity;
    private int mWidth;
    private int mHeight;
	 private static int mWidthImage;
	 private static int mHeightImage;
    private int mRotation;
    private OrientationChangeCallback mOrientationChangeCallback;

    private TextRecognizer recognizer = TextRecognition.getClient(TextRecognizerOptions.DEFAULT_OPTIONS);

	 private static String lastText = "";
	 private static String lastTextExtended = "";
	 private static boolean isRunning = false;

     private OcrConfig config = new OcrConfig();
     private OCR ocr = null;

	 public static String getLastText() {
		 return lastText;
	 }

    public static String getLastTextExtended() {
		 return lastTextExtended;
	 }

    public static int getImageWidth() {
		 return mWidthImage;
		}

	 public static int getImageHeight() {
		 return mHeightImage;
	 }

    public static Intent getStartIntent(Context context, int resultCode, Intent data) {
        Intent intent = new Intent(context, ScreenCaptureService.class);
        intent.putExtra(ACTION, START);
        intent.putExtra(RESULT_CODE, resultCode);
        intent.putExtra(DATA, data);
        return intent;
    }

    public static Intent getStopIntent(Context context) {
        Intent intent = new Intent(context, ScreenCaptureService.class);
        intent.putExtra(ACTION, STOP);
        return intent;
    }

    private static boolean isStartCommand(Intent intent) {
        return intent.hasExtra(RESULT_CODE) && intent.hasExtra(DATA)
                && intent.hasExtra(ACTION) && Objects.equals(intent.getStringExtra(ACTION), START);
    }

    private static boolean isStopCommand(Intent intent) {
        return intent.hasExtra(ACTION) && Objects.equals(intent.getStringExtra(ACTION), STOP);
    }

    private static int getVirtualDisplayFlags() {
        return DisplayManager.VIRTUAL_DISPLAY_FLAG_OWN_CONTENT_ONLY | DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC;
    }

    private class ImageAvailableListener implements ImageReader.OnImageAvailableListener {
        @Override
        public void onImageAvailable(ImageReader reader) {

            FileOutputStream fos = null;            
            try (Image image = mImageReader.acquireLatestImage()) {
                if (image != null) {
                    if(!isRunning) {
                        Image.Plane[] planes = image.getPlanes();
                        ByteBuffer buffer = planes[0].getBuffer();
                        int pixelStride = planes[0].getPixelStride();
                        int rowStride = planes[0].getRowStride();
                        int rowPadding = rowStride - pixelStride * mWidth;
                        //Log.e(TAG, "Image reviewing");

                          isRunning = true;

                          // create bitmap
								  mWidthImage = mWidth + rowPadding / pixelStride;
								  mHeightImage = mHeight;
                          final Bitmap bitmap = Bitmap.createBitmap(mWidth + rowPadding / pixelStride, mHeight, Bitmap.Config.ARGB_8888);
                          bitmap.copyPixelsFromBuffer(buffer);

                          // write bitmap to a file
                          fos = new FileOutputStream(mStoreDir + "/myscreen.jpg");
                          bitmap.compress(Bitmap.CompressFormat.PNG, 100, fos);

                          IMAGES_PRODUCED++;
                          Log.e(TAG, "captured image: " + IMAGES_PRODUCED);

                          Bitmap bMap = BitmapFactory.decodeFile(mStoreDir + "/processed_screenshot.jpg");

                          ocr.run(bMap, new OcrRunCallback() {
                            @Override
                            public void onSuccess(OcrResult result) {
                                lastText = result.getSimpleText();
                                lastTextExtended = "";
                                /*
                                for (int index = 0; index < result.getOutputRawResult().size(); index++) {
                                    OcrResultModel ocrResultModel = result.getOutputRawResult().get(index);
                                    // 文字方向 ocrResultModel.clsLabel 可能为 "0" 或 "180"
                                    lastText += ocrResultModel.getClsLabel(); 
                                    lastTextExtended += index + ": 文字方向：" + ocrResultModel.getClsLabel() +
                                            "；文字方向置信度：" + ocrResultModel.getClsConfidenceL() +
                                            "；识别置信度 " + ocrResultModel.getConfidence() +
                                            "；；文字位置：" + ocrResultModel.getPoints() + "\n";
                                }*/
                                isRunning = false;
                                bitmap.recycle();
                                Log.e(TAG, "onSuccess: " + lastText);
                            }
                        
                            @Override
                            public void onFail(Throwable e) {
                                Log.e(TAG, "onFail: 识别失败！", e);
                                isRunning = false;
                                bitmap.recycle();
                            }
                        });
                      }
                    }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private class OrientationChangeCallback extends OrientationEventListener {

        OrientationChangeCallback(Context context) {
            super(context);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            final int rotation = mDisplay.getRotation();
            if (rotation != mRotation) {
                mRotation = rotation;
                try {
                    // clean up
                    if (mVirtualDisplay != null) mVirtualDisplay.release();
                    if (mImageReader != null) mImageReader.setOnImageAvailableListener(null, null);

                    // re-create virtual display depending on device width / height
                    createVirtualDisplay();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private class MediaProjectionStopCallback extends MediaProjection.Callback {
        @Override
        public void onStop() {
            Log.e(TAG, "stopping projection.");
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mVirtualDisplay != null) mVirtualDisplay.release();
                    if (mImageReader != null) mImageReader.setOnImageAvailableListener(null, null);
                    if (mOrientationChangeCallback != null) mOrientationChangeCallback.disable();
                    mMediaProjection.unregisterCallback(MediaProjectionStopCallback.this);
                }
            });
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

                  Log.i(TAG, "onCreate1");
		  config.setModelPath("models/ch_PP-OCRv2"); // 不使用 "/" 开头的路径表示安装包中 assets 目录下的文件，例如当前表示 assets/models/ocr_v2_for_cpu
		  //config.modelPath = "/sdcard/Android/data/com.equationl.paddleocr4android.app/files/models" // 使用 "/" 表示手机储存路径，测试时请将下载的三个模型放置于该目录下
                  config.setClsModelFilenameCustom("cls.nb"); // cls 模型文件名
                  config.setDetModelFilename("det_db.nb"); // det 模型文件名
                  config.setRecModelFilename("rec_crnn.nb"); // rec 模型文件名

                  config.setIsRunDet(true);
                  config.setIsRunCls(true);
                  config.setIsRunRec(true);


		  // 运行全部模型
		  config.setRunType(RunType.All);

		  // 使用所有核心运行
		  config.setCpuPowerMode(LitePowerMode.LITE_POWER_FULL);

		  // 绘制文本位置
		  //config.setIsDrwwTextPositionBox(true);

		  // 如果是原始模型，则使用 FP16 精度
		  config.setRecRunPrecision(RunPrecision.LiteFp16);
		  config.setDetRunPrecision(RunPrecision.LiteFp16);
		  config.setClsRunPrecision(RunPrecision.LiteFp16);

                  Log.i(TAG, "onCreate2");

		  // 如果是量化模型则使用 int8 精度
		  //config.recRunPrecision(RunPrecision.LiteInt8
		  //config.detRunPrecision(RunPrecision.LiteInt8
		  //config.clsRunPrecision(RunPrecision.LiteInt8

		  // 1.同步初始化
		  /*ocr.initModelSync(config).fold(
		  {
			  if (it) {
				  Log.i(TAG, "onCreate: init success")
				  }
			  },
		  {
			  it.printStackTrace()
			  }
		  )*/

		  // 2.异步初始化
                  ocr = new OCR(this);
          ocr.initModel(config, new OcrInitCallback() {
            @Override
            public void onSuccess() {
                Log.i(TAG, "onSuccess: 初始化成功");
            }
        
            @Override
            public void onFail(Throwable e) {
                Log.e(TAG, "onFail: 初始化失败", e);
            }
        });

        Log.i(TAG, "onCreate3");

        // create store dir
        File externalFilesDir = getExternalFilesDir(null);
        if (externalFilesDir != null) {
            mStoreDir = externalFilesDir.getAbsolutePath() + "/screenshots/";
            File storeDirectory = new File(mStoreDir);
            if (!storeDirectory.exists()) {
                boolean success = storeDirectory.mkdirs();
                if (!success) {
                    Log.e(TAG, "failed to create file storage directory.");
                    stopSelf();
                }
            }
        } else {
            Log.e(TAG, "failed to create file storage directory, getExternalFilesDir is null.");
            stopSelf();
        }

        // start capture handling thread
        new Thread() {
            @Override
            public void run() {
                Looper.prepare();
                mHandler = new Handler();
                Looper.loop();
            }
        }.start();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (isStartCommand(intent)) {
            // create notification
            Pair<Integer, Notification> notification = NotificationUtils.getNotification(this);
            startForeground(notification.first, notification.second);
            // start projection
            int resultCode = intent.getIntExtra(RESULT_CODE, Activity.RESULT_CANCELED);
            Intent data = intent.getParcelableExtra(DATA);
            startProjection(resultCode, data);
        } else if (isStopCommand(intent)) {
            stopProjection();
            stopSelf();
        } else {
            stopSelf();
        }

        return START_NOT_STICKY;
    }

    private void startProjection(int resultCode, Intent data) {
        MediaProjectionManager mpManager =
                (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        if (mMediaProjection == null) {
            mMediaProjection = mpManager.getMediaProjection(resultCode, data);
            if (mMediaProjection != null) {
                // display metrics
                mDensity = Resources.getSystem().getDisplayMetrics().densityDpi;
                WindowManager windowManager = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
                mDisplay = windowManager.getDefaultDisplay();

                // create virtual display depending on device width / height
                createVirtualDisplay();

                // register orientation change callback
                mOrientationChangeCallback = new OrientationChangeCallback(this);
                if (mOrientationChangeCallback.canDetectOrientation()) {
                    mOrientationChangeCallback.enable();
                }

                // register media projection stop callback
                mMediaProjection.registerCallback(new MediaProjectionStopCallback(), mHandler);
            }
        }
    }

    private void stopProjection() {
        if (mHandler != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mMediaProjection != null) {
                        mMediaProjection.stop();
                    }
                }
            });
        }
    }

    @SuppressLint("WrongConstant")
    private void createVirtualDisplay() {
        // get width and height
        mWidth = Resources.getSystem().getDisplayMetrics().widthPixels;
        mHeight = Resources.getSystem().getDisplayMetrics().heightPixels;

        // start capture reader
        mImageReader = ImageReader.newInstance(mWidth, mHeight, PixelFormat.RGBA_8888, 2);
        mVirtualDisplay = mMediaProjection.createVirtualDisplay(SCREENCAP_NAME, mWidth, mHeight,
                mDensity, getVirtualDisplayFlags(), mImageReader.getSurface(), null, mHandler);
        mImageReader.setOnImageAvailableListener(new ImageAvailableListener(), mHandler);
    }
}
