package com.kimentii.cameraresearch;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

public class GrayscaleFilteringSurface extends SurfaceView implements SurfaceHolder.Callback, Camera.PreviewCallback {
    // maybe need to be 64n
    public static final int MAX_DISP_IMG_WIDTH = 640;
    public static final int MAX_DISP_IMG_HEIGHT = 480;
    public static final int MAX_FPS = 60000;
    public static final int MIN_FPS = 60000;

    static {
        System.loadLibrary("opencl-blur");
    }

    native private int[] runFilter(int[] in, int width, int height);

    private static final String TAG = "FilteringSurface";

    private Camera mCamera;
    private Bitmap mFilteredImage;
    private boolean isFiltering = false;


    public GrayscaleFilteringSurface(Context context) {
        super(context);
        getHolder().addCallback(this);
        getHolder().setFixedSize(MAX_DISP_IMG_WIDTH, MAX_DISP_IMG_WIDTH);
        setWillNotDraw(false);
    }

    public void surfaceCreated(SurfaceHolder pHolder) {
        try {
            Log.d(TAG, "Surface created");
            mCamera = Camera.open(0);
            Camera.Parameters params = mCamera.getParameters();
            params.setPreviewFormat(ImageFormat.NV21);
            params.setPreviewSize(MAX_DISP_IMG_WIDTH, MAX_DISP_IMG_HEIGHT);
            //params.setPreviewFpsRange(MIN_FPS, MAX_FPS);
            if (params.getSupportedFocusModes().contains(
                    Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO)) {
                params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
            }
            mCamera.setParameters(params);
            mCamera.setPreviewDisplay(null);
//            for (Camera.Size a : mCamera.getParameters().getSupportedPreviewSizes()) {
//                Log.d(TAG, "Supported image size: " + a.width + "x" + a.height);
//            }
            mCamera.setPreviewCallback(this);
            mCamera.startPreview();
            Log.d(TAG, "surfaceCreated: end");
        } catch (Exception e) {
            Log.i(TAG, "Exception at surfaceCreated", e);
        }
    }

    public void surfaceChanged(SurfaceHolder pHolder, int pFormat, int pW, int pH) {
        Log.d(TAG, "Surface changed");
        try {
            mCamera.setPreviewDisplay(pHolder);
        } catch (IOException e) {
            e.printStackTrace();
        }
        mCamera.startPreview();
    }

    public void onPreviewFrame(byte[] data, Camera camera) {
        Log.d(TAG, "Got frame");
        try {
//            Camera.Size s = camera.getParameters().getPreviewSize();
//            Log.d(TAG, "Camera format: " + camera.getParameters().getPreviewFormat() + " Image size: " + data.length + "Preview size: " + s.width + "-" + s.height);
            int[] result = YUV_NV21_TO_RGB(data, MAX_DISP_IMG_WIDTH, MAX_DISP_IMG_HEIGHT);
//            int[] result = mock(MAX_DISP_IMG_HEIGHT);
            if (isFiltering()) {
                result = runFilter(result, MAX_DISP_IMG_HEIGHT, MAX_DISP_IMG_HEIGHT);
            }
//            printImage(result, MAX_DISP_IMG_HEIGHT);
            mFilteredImage = Bitmap.createBitmap(result, MAX_DISP_IMG_HEIGHT, MAX_DISP_IMG_HEIGHT, Bitmap.Config.ARGB_8888);
//            mFilteredImage = Bitmap.createBitmap(argb, MAX_DISP_IMG_WIDTH, MAX_DISP_IMG_HEIGHT, Bitmap.Config.ARGB_8888);
        } catch (Exception e) {
            Log.i(TAG, "Got exception", e);
        }
//        Log.d(TAG, "Calling postInvalidate...");
        postInvalidate();
    }

    @Override
    protected void onDraw(Canvas pCanvas) {
        try {
            if (mFilteredImage != null) {
                //Log.d(TAG, "onDraw: mFilterdImage is null");
                pCanvas.drawBitmap(mFilteredImage, 0, 0, null);
            }
        } catch (Exception e) {
//            Log.e(TAG, "Exception onDraw", e);
        }
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mCamera != null) {
            mCamera.stopPreview();
            mCamera.setPreviewCallback(null);
            mCamera.release();
        }
    }

    public static int[] YUV_NV21_TO_RGB(byte[] yuv, int width, int height) {
        final int frameSize = width * height;
        int[] argb = new int[height * height];

        final int ii = 0;
        final int ij = 0;
        final int di = +1;
        final int dj = +1;

        int a = 0;
        for (int i = 0, ci = ii; i < height; ++i, ci += di) {
            for (int j = 0, cj = ij; j < width; ++j, cj += dj) {
                int y = (0xff & ((int) yuv[ci * width + cj]));
                int v = (0xff & ((int) yuv[frameSize + (ci >> 1) * width + (cj & ~1) + 0]));
                int u = (0xff & ((int) yuv[frameSize + (ci >> 1) * width + (cj & ~1) + 1]));
                y = y < 16 ? 16 : y;

                int r = (int) (1.164f * (y - 16) + 1.596f * (v - 128));
                int g = (int) (1.164f * (y - 16) - 0.813f * (v - 128) - 0.391f * (u - 128));
                int b = (int) (1.164f * (y - 16) + 2.018f * (u - 128));

                r = r < 0 ? 0 : (r > 255 ? 255 : r);
                g = g < 0 ? 0 : (g > 255 ? 255 : g);
                b = b < 0 ? 0 : (b > 255 ? 255 : b);
                if (j < height) {
                    argb[j * height + (height - i % height - 1)] = 0xff000000 | (r << 16) | (g << 8) | b;
                }

//                argb[j*width + (width - i%width - 1)] = 0xff000000 | (0 << 16) | (0 << 8) | 100;
            }
        }
        return argb;
    }

    private static int[] mock(int w) {
        int result[] = new int[w * w];
        for (int i = 0; i < w; ++i) {
            for (int j = 0; j < w; ++j) {
                result[j * w + i] = 0xff_00_00_00 | (0 << 16) | (0 << 8) | 100;
            }
        }
        return result;
    }

    private static int convertYUVtoRGB(int y, int u, int v) {
        int r, g, b;

        r = y + (int) (1.402f * v);
        g = y - (int) (0.344f * u + 0.714f * v);
        b = y + (int) (1.772f * u);
        r = r > 255 ? 255 : r < 0 ? 0 : r;
        g = g > 255 ? 255 : g < 0 ? 0 : g;
        b = b > 255 ? 255 : b < 0 ? 0 : b;
        return 0xff000000 | (b << 16) | (g << 8) | r;
    }

    private static int getA(int pixel) {
        return (pixel >> 24) & 0xFF;
    }

    private static int getR(int pixel) {
        return (pixel >> 16) & 0xFF;
    }

    private static int getG(int pixel) {
        return (pixel >> 8) & 0xFF;
    }

    private static int getB(int pixel) {
        return (pixel & 0x000000FF);
    }

    private static void printImage(int[] img, int w) {
        Log.d(TAG, "");
        for (int i = 0; i < 10; i++) {
            String logString = "";
            for (int j = 0; j < 10; j++) {
                logString += "[" + getA(img[j + i * w]) + ", " + getR(img[j + i * w]) + ", " + getG(img[j + i * w]) + ", " + getB(img[j + i * w]) + "]---";
            }
            Log.d(TAG, logString);
        }

    }

    public boolean isFiltering() {
        return isFiltering;
    }

    public void setFiltering(boolean filtering) {
        isFiltering = filtering;
    }
}