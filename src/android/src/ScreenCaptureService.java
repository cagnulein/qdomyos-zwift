package org.cagnulen.qdomyoszwift;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
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
import org.cagnulen.qdomyoszwift.QLog;
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
import org.cagnulen.qdomyoszwift.QLog;
import android.os.Build;

public class ScreenCaptureService extends Service {

    private static final String TAG = "ScreenCaptureService";
    private static final String RESULT_CODE = "RESULT_CODE";
    private static final String DATA = "DATA";
    private static final String ACTION = "ACTION";
    private static final String START = "START";
    private static final String STOP = "STOP";
    private static final String SCREENCAP_NAME = "screencap";

    private static int IMAGES_PRODUCED;

    private static final String EXTRA_FOREGROUND_SERVICE_TYPE = "FOREGROUND_SERVICE_TYPE";
    private static final int FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE = 0x10;

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
                        //QLog.e(TAG, "Image reviewing");

                          isRunning = true;

                          // create bitmap
								  mWidthImage = mWidth + rowPadding / pixelStride;
								  mHeightImage = mHeight;
                          final Bitmap bitmap = Bitmap.createBitmap(mWidth + rowPadding / pixelStride, mHeight, Bitmap.Config.ARGB_8888);
                          bitmap.copyPixelsFromBuffer(buffer);
/*
                          // write bitmap to a file
                          fos = new FileOutputStream(mStoreDir + "/myscreen.png");
                          bitmap.compress(Bitmap.CompressFormat.JPEG, 100, fos);

                          IMAGES_PRODUCED++;
                          QLog.e(TAG, "captured image: " + IMAGES_PRODUCED);
*/

                          InputImage inputImage = InputImage.fromBitmap(bitmap, 0);
                          /*InputImage inputImage = InputImage.fromByteBuffer(buffer,
                                  mWidth + rowPadding / pixelStride, mHeight,
                                  0,
                                  InputImage.IMAGE_FORMAT_NV21 // or IMAGE_FORMAT_YV12
                          );*/

                          Task<Text> result =
                          recognizer.process(inputImage)
                          .addOnSuccessListener(new OnSuccessListener<Text>() {
                                  @Override
                                  public void onSuccess(Text result) {
                                          // Task completed successfully

                                          //QLog.e(TAG, "Image done!");

                                          String resultText = result.getText();
                                          lastText = resultText;
                                          lastTextExtended = "";
                                          for (Text.TextBlock block : result.getTextBlocks()) {
                                                   String blockText = block.getText();
                                                        Point[] blockCornerPoints = block.getCornerPoints();
                                                        Rect blockFrame = block.getBoundingBox();
                                                          lastTextExtended = lastTextExtended + blockText + "$$" + blockFrame.toString() + "§§";
							  /*for (Text.Line line : block.getLines()) {
                                                                 String lineText = line.getText();
                                                                 Point[] lineCornerPoints = line.getCornerPoints();
                                                                 Rect lineFrame = line.getBoundingBox();
                                                                 for (Text.Element element : line.getElements()) {
                                                                          String elementText = element.getText();
                                                                          Point[] elementCornerPoints = element.getCornerPoints();
                                                                          Rect elementFrame = element.getBoundingBox();
                                                                          for (Text.Symbol symbol : element.getSymbols()) {
                                                                                   String symbolText = symbol.getText();
                                                                                        Point[] symbolCornerPoints = symbol.getCornerPoints();
                                                                                        Rect symbolFrame = symbol.getBoundingBox();
                                                                                        }
                                                                 }
																				}*/
																	}
                                     bitmap.recycle();
                                     isRunning = false;
                                          }
                                  })
                          .addOnFailureListener(
                          new OnFailureListener() {
                                  @Override
                                  public void onFailure(Exception e) {
                                          // Task failed with an exception
                                          //QLog.e(TAG, "Image fail");
                                          isRunning = false;
                                          }
                                  });
                          } else {
                            //QLog.e(TAG, "Image ignored");
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
            QLog.e(TAG, "stopping projection.");
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

        // create store dir
        File externalFilesDir = getExternalFilesDir(null);
        if (externalFilesDir != null) {
            mStoreDir = externalFilesDir.getAbsolutePath() + "/screenshots/";
            File storeDirectory = new File(mStoreDir);
            if (!storeDirectory.exists()) {
                boolean success = storeDirectory.mkdirs();
                if (!success) {
                    QLog.e(TAG, "failed to create file storage directory.");
                    stopSelf();
                }
            }
        } else {
            QLog.e(TAG, "failed to create file storage directory, getExternalFilesDir is null.");
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

            try {
                int serviceType = intent.getIntExtra(EXTRA_FOREGROUND_SERVICE_TYPE, FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    startForeground(notification.first, notification.second, serviceType);
                } else {
                    startForeground(notification.first, notification.second);
                }
            } catch (Exception e) {
                QLog.e("ForegroundService", "Failed to start foreground service", e);
                return START_NOT_STICKY;
            }
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
