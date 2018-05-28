package com.kimentii.cameraresearch;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private BlurFilteringSurface blurFilteringSurface;
    // It's easy to adjust blur filter size, just change the constant here
    // Also we can make runtime-selectable filter size, but we are a little
    // bit lazy, sorry.
    public static final int FILTER_SIZE = 7;

    native private int compileKernel(int filter_size);

    static {
        System.loadLibrary("opencl-blur");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        int compilationResult = compileKernel(FILTER_SIZE);
        Log.d(TAG, "Compilation result: " + compilationResult);
        if (compilationResult == 0) {
            blurFilteringSurface = new BlurFilteringSurface(this);
            FrameLayout preview = findViewById(R.id.camera_preview);
            preview.addView(blurFilteringSurface);
        }
    }


    public void applyFilterBtnClicked(View button) {
        boolean filtering = !blurFilteringSurface.isFiltering();
        blurFilteringSurface.setFiltering(filtering);
        if (filtering) {
            ((TextView)button).setText("Stop filtering");
        } else {
            ((TextView)button).setText("Start filtering");
        }
    }

    public void focusBtnClicked(View button) {
        boolean focused = !blurFilteringSurface.isFocus();
        blurFilteringSurface.setFocus(focused);
        if (focused) {
            ((TextView)button).setText("No focus");
        } else {
            ((TextView)button).setText("Focus");
        }
    }
}
