package com.kimentii.cameraresearch;


import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.FrameLayout;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "FilteringSurface";
    native private int compileKernel();

    static {
        System.loadLibrary("opencl-blur");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.d(TAG, "Compilation result: " + compileKernel());

        FrameLayout preview = findViewById(R.id.camera_preview);
        preview.addView(new GrayscaleFilteringSurface(this));
    }
}
