package com.kimentii.cameraresearch;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceView;

import static com.kimentii.cameraresearch.MainActivity.FILTER_SIZE;

/**
 * Gets images from camera, converts it
 * <ol>
 *    <li>Gets image from camera</li>
 *    <li>Converts it to one-plane ARGB format</li>
 *    <li>Rotates on 90 degrees clock-wise</li>
 *    <li>Calls OpenCL native filter</li>
 *    <li>Draws result on the screen</li>
 * </ol>
 *
 * @author Mike, Kirill
 */
public class BlurFilteringSurface extends SurfaceView implements Camera.PreviewCallback {
    public static final int IMG_WIDTH = 640;
    public static final int IMG_HEIGHT = 480;

    static {
        System.loadLibrary("opencl-blur");
    }

    // Native C call that uses OpenCL
    native private int[] runFilter(int[] in, int width, int height);

    private static final String TAG = "FilteringSurface";

    private Camera mCamera;
    private Bitmap mFilteredImage;
    private boolean mFiltering = false;
    private boolean mFocus = false;


    public BlurFilteringSurface(Context context) {
        super(context);

        // Sets the Canvas size
        getHolder().setFixedSize(IMG_HEIGHT - (FILTER_SIZE - 1), IMG_WIDTH - (FILTER_SIZE - 1));

        // Necessary for {#link onDraw} method to be called
        setWillNotDraw(false);

        // Camera initializing
        try {
            mCamera = Camera.open(0);

            Camera.Parameters params = mCamera.getParameters();
            params.setPreviewFormat(ImageFormat.NV21);
            params.setPreviewSize(IMG_WIDTH, IMG_HEIGHT);
            mCamera.setParameters(params);

            // API necessity
            mCamera.setPreviewDisplay(null);

            // Sets this object to process camera data via
            // {#link onPreviewFrame} callback method
            mCamera.setPreviewCallback(this);

            // Starts camera processing
            mCamera.startPreview();
        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize camera", e);
        }
    }

    /**
     * Callback that is called by camera when image data is ready.
     *
     * @param data      NV21-format image data
     * @param camera    Camera object
     */
    public void onPreviewFrame(byte[] data, Camera camera) {
        int[] result = YUV_NV21_TO_RGB(data, IMG_WIDTH, IMG_HEIGHT);
        if (isFiltering()) {
            result = runFilter(result, IMG_HEIGHT, IMG_WIDTH);
        }
        mFilteredImage = Bitmap.createBitmap(result, IMG_HEIGHT, IMG_WIDTH, Bitmap.Config.ARGB_8888);

        // Important call. Comes from {#link View} superclass
        // and triggers object itself to be redrawn. That means
        // that {#link onDraw} method will be called somewhere
        // in the future.
        postInvalidate();
    }

    /**
     * Simply updates canvas image.
     *
     * @param pCanvas   canvas to be updated
     */
    @Override
    protected void onDraw(Canvas pCanvas) {
        if (mFilteredImage != null) {
            pCanvas.drawBitmap(mFilteredImage, 0, 0, null);
        }
    }

    /**
     * Converts native android NV21 format to plane ARGB for
     * OpenCL filter processing. One pixel takes 4 bytes or
     * one int. Also performs image rotation by 90 degrees
     * clock-wise. Method was stolen from StackOverflow question
     * (with rotation customization) :).
     * https://stackoverflow.com/questions/12469730/confusion-on-yuv-nv21-conversion-to-rgb
     *
     * @param yuv       YUV raw format data
     * @param width     image width
     * @param height    image height
     * @return          int array. Each int contains one ARGB pixel
     */
    public static int[] YUV_NV21_TO_RGB(byte[] yuv, int width, int height) {
        final int frameSize = width * height;
        int[] argbResult = new int[width * height];

        final int ii = 0;
        final int ij = 0;
        final int di = +1;
        final int dj = +1;

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

                // Rotation comes here
                argbResult[j * height + (height - i % height - 1)] = 0xff000000 | (r << 16) | (g << 8) | b;
            }
        }
        return argbResult;
    }

    public boolean isFiltering() {
        return mFiltering;
    }

    public void setFiltering(boolean filtering) {
        this.mFiltering = filtering;
    }

    public void setFocus(boolean focus) {
        this.mFocus = focus;
        Camera.Parameters params = mCamera.getParameters();

        if (focus && params.getSupportedFocusModes().contains(
                Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO)) {
            params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
        } else {
            params.setFocusMode(Camera.Parameters.FOCUS_MODE_MACRO);
        }
        mCamera.setParameters(params);
    }

    public boolean isFocus() {
        return mFocus;
    }
}