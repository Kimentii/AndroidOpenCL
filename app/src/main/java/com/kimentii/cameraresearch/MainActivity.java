package com.kimentii.cameraresearch;


import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
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

        //Log.d(TAG, "Compilation result: " + compileKernel());
        final GrayscaleFilteringSurface grayscaleFilteringSurface = new GrayscaleFilteringSurface(this);

        Button button = findViewById(R.id.button_action_start);
        button.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                grayscaleFilteringSurface.setFiltering(grayscaleFilteringSurface.isFiltering());
            }
        });

        FrameLayout preview = findViewById(R.id.camera_preview);
        preview.addView(grayscaleFilteringSurface);

    }
}
