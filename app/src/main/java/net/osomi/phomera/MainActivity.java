package net.osomi.phomera;

import androidx.annotation.IdRes;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Toast;
import android.widget.TextView;

import org.freedesktop.gstreamer.GStreamer;

import java.io.InvalidObjectException;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    private static boolean isInit = false, canInit = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        if(!isInit)
            try {

                if(!canInit)
                    throw new InvalidObjectException("initNative failed");

                GStreamer.init(this);
                isInit = true;
            }
            catch (Exception e) {
                Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
                finish();
                return;
            }

        getSupportActionBar().hide();
        setContentView(R.layout.activity_main);

        disableCamera();

        findViewById(R.id.div_bottom).setVisibility(View.VISIBLE);
        findViewById(R.id.div_extended).setVisibility(View.GONE);

        findViewById(R.id.more).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleMore();
            }
        });

        findViewById(R.id.camera_lib).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(v.getContext(), GalleryActivity.class));
            }
        });

        ((TextView)findViewById(R.id.version)).setText(getGStreamerVersion());

        ((SurfaceView) this.findViewById(R.id.preview)).getHolder().addCallback(this);
        initGS();
    }

    private void toggleMore() {

        if(findViewById(R.id.div_extended).getVisibility() == View.GONE)
            findViewById(R.id.div_extended).setVisibility(View.VISIBLE);

        else findViewById(R.id.div_extended).setVisibility(View.GONE);
    }

    @Override
    protected void onDestroy() {
        finalizeGS();
        super.onDestroy();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) { }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d("GStreamer", "Surface changed to " + format + " " + width + "x" + height);
        initSurface(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d("GStreamer", "Surface destroyed");
        finalizeSurface();
    }

    private void disableCamera() {
        System.out.println("Disable camera");
        disable(R.id.camera_button);
        disable(R.id.aperture_button);
        disable(R.id.colorization);
        disable(R.id.iso);
        disable(R.id.focus_button);
        disable(R.id.schedule);
        disable(R.id.burst);
        disable(R.id.hdr);
    }

    private void enableCamera() {
        enable(R.id.camera_button);
        enable(R.id.aperture_button);
        enable(R.id.colorization);
        enable(R.id.iso);
        enable(R.id.focus_button);
        enable(R.id.schedule);
        enable(R.id.burst);
        enable(R.id.hdr);
    }

    private void disable(@IdRes int id) {
        View v = findViewById(id);
        v.setAlpha(0.5f);
        v.setClickable(false);
    }

    private void enable(@IdRes int id) {
        View v = findViewById(id);
        v.setAlpha(1);
        v.setClickable(true);
    }

    private void onGSReady() {
        runOnUiThread (new Runnable() {
            public void run() {
                enableCamera();
            }
        });
    }

    private void onGSUnready() {
        runOnUiThread (new Runnable() {
            public void run() {
                disableCamera();
            }
        });
    }

    private native String getGStreamerVersion();
    private native void initGS();
    private native void finalizeGS();
    private native void initSurface(Object surface);
    private native void finalizeSurface();
    private static native boolean initNative();

    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("phomera");
        canInit = initNative();
    }
}
