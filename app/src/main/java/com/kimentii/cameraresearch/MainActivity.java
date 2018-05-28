package com.kimentii.cameraresearch;


import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;

public class MainActivity extends AppCompatActivity {
    private final static String TAG = MainActivity.class.getSimpleName();

    Camera mCamera;
    SurfaceView mSurfaceView;

    public native long getGPUWorkItems();

    public native byte[] filterImage(byte[] image, int width, int height);

    static {
        System.loadLibrary("opencl-blur");
    }

    @Override
    protected void onResume() {
        super.onResume();
        mCamera = Camera.open(0);

        mCamera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                Log.d(TAG, "onPreviewFrame");

                Camera.Size previewSize = mCamera.getParameters().getPreviewSize();
                YuvImage yuvImage = new YuvImage(data, ImageFormat.NV21, previewSize.width, previewSize.height, null);
                ByteArrayOutputStream baos = new ByteArrayOutputStream();

                data = filterImage(baos.toByteArray(), data.length, 1);

                yuvImage.compressToJpeg(new Rect(0, 0, previewSize.width, previewSize.height), 80, baos);
                byte[] jdata = baos.toByteArray();
                Bitmap bitmap = BitmapFactory.decodeByteArray(jdata, 0, jdata.length);
                ImageView imageView = findViewById(R.id.iv_picture);
                imageView.setImageBitmap(bitmap);
                Log.d(TAG, "onPreviewFrame_end");
            }
        });
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSurfaceView = findViewById(R.id.surface_view);

        Toast.makeText(
                getApplicationContext(),
                String.valueOf(getGPUWorkItems()),
                Toast.LENGTH_LONG
        ).show();

        SurfaceHolder holder = mSurfaceView.getHolder();
        holder.addCallback(new MySurfaceHolderCallback());
        if (mCamera == null) {
            Log.d(TAG, "Can't get camera.");
        }

    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mCamera != null)
            mCamera.release();
        mCamera = null;
    }

    class MySurfaceHolderCallback implements SurfaceHolder.Callback {

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            try {
                mCamera.setPreviewDisplay(holder);
                mCamera.startPreview();
                Log.d(TAG, "surfaceCreated");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format,
                                   int width, int height) {
            Log.d(TAG, "surfaceChanged");
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.d(TAG, "surfaceDestroyed");
        }
    }
}
